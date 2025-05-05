#include "parser.h"

#include "log.h"

prs_t *prs_init(prs_t *prs, uint nodes_cap, alloc_t alloc)
{
	if (prs == NULL) {
		return NULL;
	}

	if (tree_init(&prs->nodes, nodes_cap, sizeof(prs_node_data_t), alloc) == NULL) {
		log_error("cparse", "parser", NULL, "failed to initialize nodes tree");
		return NULL;
	}

	return prs;
}

void prs_free(prs_t *prs)
{
	if (prs == NULL) {
		return;
	}

	tree_free(&prs->nodes);
}

prs_node_t prs_add(prs_t *prs, prs_node_data_t node)
{
	if (prs == NULL) {
		return PRS_NODE_END;
	}

	prs_node_t child;
	prs_node_data_t *data = tree_add(&prs->nodes, &child);
	if (data == NULL) {
		log_error("cparse", "parser", NULL, "failed to add node");
		return PRS_NODE_END;
	}

	*data = node;

	return child;
}

prs_node_t prs_add_node(prs_t *prs, prs_node_t parent, prs_node_t node)
{
	if (prs == NULL) {
		return PRS_NODE_END;
	}

	if (parent < prs->nodes.cnt) {
		tree_set_child(&prs->nodes, parent, node);
	}

	return node;
}

int prs_remove_node(prs_t *prs, prs_node_t node)
{
	if (prs == NULL) {
		return 1;
	}

	return tree_remove(&prs->nodes, node);
}

prs_node_t prs_get_rule(const prs_t *prs, prs_node_t parent, stx_rule_t rule)
{
	if (prs == NULL) {
		return PRS_NODE_END;
	}

	prs_node_t child;
	tree_foreach_child(&prs->nodes, parent, child)
	{
		prs_node_data_t *cdata = tree_get(&prs->nodes, child);
		switch (cdata->type) {
		case PRS_NODE_RULE:
			if (cdata->val.rule == rule) {
				return child;
			}
			break;
		default: break;
		}
	}

	return PRS_NODE_END;
}

int prs_get_str(const prs_t *prs, prs_node_t parent, token_t *out)
{
	if (prs == NULL || out == NULL) {
		return 1;
	}

	prs_node_t child;
	tree_foreach_child(&prs->nodes, parent, child)
	{
		prs_node_data_t *data = tree_get(&prs->nodes, child);
		switch (data->type) {
		case PRS_NODE_RULE: prs_get_str(prs, child, out); break;
		case PRS_NODE_TOKEN: {
			out->start = (out->len == 0 ? data->val.token.start : out->start);
			out->len += data->val.token.len;
			break;
		}
		case PRS_NODE_LITERAL: {
			out->start = (out->len == 0 ? data->val.literal.start : out->start);
			out->len += data->val.literal.len;
			break;
		}
		case PRS_NODE_UNKNOWN:
		default: log_error("cparse", "parser", NULL, "unexpected node: %d", data->type); break;
		}
	}

	return 0;
}

typedef struct prs_parse_err_s {
	stx_rule_t rule;
	uint tok;
	stx_term_t exp;
} prs_parse_err_t;

static int prs_parse_rule(prs_t *prs, stx_rule_t rule_id, uint *off, prs_node_t node, prs_parse_err_t *err);
static int prs_parse_terms(prs_t *prs, stx_rule_t rule, stx_term_t terms, uint *off, prs_node_t node, prs_parse_err_t *err);

static int prs_parse_term(prs_t *prs, stx_rule_t rule, stx_term_t term_id, uint *off, prs_node_t node, prs_parse_err_t *err)
{
	const stx_term_data_t *term = stx_get_term_data(prs->stx, term_id);

	switch (term->type) {
	case STX_TERM_RULE: {
		uint nodes_cnt	 = prs->nodes.cnt;
		uint cur	 = *off;
		prs_node_t child = PRS_NODE_RULE(prs, term->val.rule);
		if (prs_parse_rule(prs, term->val.rule, off, child, err)) {
			prs->nodes.cnt = nodes_cnt;
			*off	       = cur;
			return 1;
		}

		prs_add_node(prs, node, child);
		return 0;
	}
	case STX_TERM_TOKEN: {
		const token_type_t token_type = term->val.token;

		char buf[32] = {0};

		size_t len = token_type_print(1 << token_type, DST_BUF(buf));

		token_t token = lex_get_token(prs->lex, *off);

		if (token.type & (1 << token_type)) {
			prs_add_node(
				prs, node, PRS_NODE_TOKEN(prs, ((token_t){.type = token_type, .start = token.start, .len = token.len})));
			log_trace("cparse", "parser", NULL, "%.*s: success +%d", (int)len, buf, token.len);
			*off += token.len;
			return 0;
		}

		if (err->tok == LEX_TOKEN_END || *off >= err->tok) {
			err->rule = rule;
			err->tok  = *off;
			err->exp  = term_id;
		}
		char act[32]   = {0};
		size_t act_len = lex_print_token(prs->lex, token, DST_BUF(act));
		log_trace("cparse", "parser", NULL, "failed: expected %.*s, but got %.*s", (int)len, buf, act_len, act);
		return 1;
	}
	case STX_TERM_LITERAL: {
		strv_t literal = STRVN((char *)&prs->stx->strs.data[term->val.literal.start], term->val.literal.len);

		for (uint i = 0; i < (uint)literal.len; i++) {
			token_t token = lex_get_token(prs->lex, *off + i);

			if (token.type & (1 << TOKEN_EOF)) {
				err->rule = rule;
				err->tok  = *off + i;
				err->exp  = term_id;
				log_trace("cparse", "parser", NULL, "\'%*s\': failed: end of tokens", literal.len, literal.data);
				return 1;
			}

			strv_t c	 = STRVN(&literal.data[i], 1);
			strv_t token_val = lex_get_token_val(prs->lex, token);
			if (!strv_eq(token_val, c)) {
				if (err->tok == LEX_TOKEN_END || *off + i >= err->tok) {
					err->rule = rule;
					err->tok  = *off + i;
					err->exp  = term_id;
				}

				char buf[256] = {0};
				size_t len    = strv_print(token_val, DST_BUF(buf));

				log_trace("cparse",
					  "parser",
					  NULL,
					  "failed: expected \'%*s\', but got \'%.*s\'",
					  literal.len,
					  literal.data,
					  (int)len,
					  buf);
				return 1;
			}
		}

		prs_add_node(prs, node, PRS_NODE_LITERAL(prs, *off, (uint)literal.len));
		log_trace("cparse", "parser", NULL, "\'%*s\': success +%d", literal.len, literal.data, literal.len);
		*off += (uint)literal.len;
		return 0;
	}
	case STX_TERM_OR: {
		uint nodes_cnt = prs->nodes.cnt;
		uint cur       = *off;
		if (!prs_parse_terms(prs, rule, term->val.orv.l, off, node, err)) {
			log_trace("cparse", "parser", NULL, "left: success");
			return 0;
		}

		log_trace("cparse", "parser", NULL, "left: failed");
		tree_set_cnt(&prs->nodes, nodes_cnt);

		if (!prs_parse_terms(prs, rule, term->val.orv.r, off, node, err)) {
			log_trace("cparse", "parser", NULL, "right: success");
			return 0;
		}

		log_trace("cparse", "parser", NULL, "right: failed");
		tree_set_cnt(&prs->nodes, nodes_cnt);
		*off = cur;
		return 1;
	}
	default: log_warn("cparse", "parser", NULL, "unknown term type: %d", term->type); break;
	}

	return 1;
}

static int prs_parse_terms(prs_t *prs, stx_rule_t rule, stx_term_t terms, uint *off, prs_node_t node, prs_parse_err_t *err)
{
	uint cur = *off;
	const stx_term_data_t *term;
	stx_term_foreach(&prs->stx->terms, terms, term)
	{
		if (prs_parse_term(prs, rule, terms, off, node, err)) {
			*off = cur;
			return 1;
		}
	}

	return 0;
}

static int prs_parse_rule(prs_t *prs, const stx_rule_t rule_id, uint *off, prs_node_t node, prs_parse_err_t *err)
{
	const stx_rule_data_t *rule = stx_get_rule_data(prs->stx, rule_id);
	if (rule == NULL) {
		log_error("cparse", "parser", NULL, "rule not found: %d", rule_id);
		return 1;
	}

	log_trace("cparse", "parser", NULL, "<%d>", rule_id);

	uint cur = *off;
	if (prs_parse_terms(prs, rule_id, rule->terms, off, node, err)) {
		log_trace("cparse", "parser", NULL, "<%d>: failed", rule_id);
		*off = cur;
		return 1;
	}

	log_trace("cparse", "parser", NULL, "<%d>: success +%d", rule_id, *off - cur);
	return 0;
}

int prs_parse(prs_t *prs, const lex_t *lex, const stx_t *stx, stx_rule_t rule, prs_node_t *root, dst_t dst)
{
	if (prs == NULL || lex == NULL || stx == NULL) {
		return 1;
	}

	prs->lex = lex;
	prs->stx = stx;

	prs->nodes.cnt = 0;

	prs_parse_err_t err = {
		.rule = (uint)-1,
		.tok  = LEX_TOKEN_END,
		.exp  = (uint)-1,
	};

	prs_node_t tmp = prs_add_node(prs, PRS_NODE_END, PRS_NODE_RULE(prs, rule));
	uint parsed    = 0;
	if (prs_parse_rule(prs, rule, &parsed, tmp, &err) || parsed != prs->lex->src.len) {
		if (err.exp == (uint)-1) {
			log_error("cparse", "parser", NULL, "wrong syntax");
			return 1;
		}

		const stx_term_data_t *term = stx_get_term_data(prs->stx, err.exp);

		token_loc_t loc = lex_get_token_loc(prs->lex, err.tok);

		dst.off += lex_token_loc_print_loc(prs->lex, loc, dst);

		if (term->type == STX_TERM_TOKEN) {
			char buf[32] = {0};
			size_t len   = token_type_print(1 << term->val.token, DST_BUF(buf));
			dst.off += dputf(dst, "error: expected %.*s\n", (int)len, buf);

		} else {
			strv_t exp_str = STRVN((char *)&prs->stx->strs.data[term->val.literal.start], term->val.literal.len);
			dst.off += dputf(dst, "error: expected \'%.*s\'\n", exp_str.len, exp_str.data);
		}

		dst.off += lex_token_loc_print_src(prs->lex, loc, dst);
		return 1;
	}

	if (root) {
		*root = tmp;
	}

	log_trace("cparse", "parser", NULL, "success");
	return 0;
}

static size_t print_nodes(void *data, dst_t dst, const void *priv)
{
	const prs_t *prs = priv;

	size_t off = dst.off;

	const prs_node_data_t *node = data;
	switch (node->type) {
	case PRS_NODE_RULE: {
		dst.off += dputf(dst, "%d\n", node->val.rule);
		break;
	}
	case PRS_NODE_TOKEN: {
		char type[32]	= {0};
		size_t type_len = token_type_print(1 << node->val.token.type, DST_BUF(type));
		char val[32]	= {0};
		size_t val_len	= strv_print(lex_get_token_val(prs->lex, node->val.token), DST_BUF(val));
		dst.off += dputf(dst, "%.*s(%.*s)\n", (int)type_len, type, (int)val_len, val);
		break;
	}
	case PRS_NODE_LITERAL: {
		char val[32]   = {0};
		size_t val_len = strv_print(lex_get_token_val(prs->lex, node->val.literal), DST_BUF(val));
		dst.off += dputf(dst, "\'%.*s\'\n", (int)val_len, val);
		break;
	}
	default: break;
	}

	return dst.off - off;
}

size_t prs_print(const prs_t *prs, prs_node_t node, dst_t dst)
{
	if (prs == NULL) {
		return 0;
	}
	return tree_print(&prs->nodes, node, print_nodes, dst, prs);
}
