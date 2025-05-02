#include "eparser.h"

#include "log.h"

eprs_t *eprs_init(eprs_t *eprs, uint nodes_cap, alloc_t alloc)
{
	if (eprs == NULL) {
		return NULL;
	}

	if (tree_init(&eprs->nodes, nodes_cap, sizeof(eprs_node_data_t), alloc) == NULL) {
		log_error("cparse", "eparser", NULL, "failed to initialize nodes tree");
		return NULL;
	}

	return eprs;
}

void eprs_free(eprs_t *eprs)
{
	if (eprs == NULL) {
		return;
	}

	tree_free(&eprs->nodes);
}

eprs_node_t eprs_add(eprs_t *eprs, eprs_node_data_t node)
{
	if (eprs == NULL) {
		return EPRS_NODE_END;
	}

	eprs_node_t child;
	eprs_node_data_t *data = tree_add(&eprs->nodes, &child);
	if (data == NULL) {
		log_error("cparse", "eparser", NULL, "failed to add node");
		return EPRS_NODE_END;
	}

	*data = node;

	return child;
}

eprs_node_t eprs_add_node(eprs_t *eprs, eprs_node_t parent, eprs_node_t node)
{
	if (eprs == NULL) {
		return EPRS_NODE_END;
	}

	if (parent < eprs->nodes.cnt && tree_set_child(&eprs->nodes, parent, node)) {
		return (uint)-1;
	}

	return node;
}

int eprs_remove_node(eprs_t *eprs, eprs_node_t node)
{
	if (eprs == NULL) {
		return 1;
	}

	return tree_remove(&eprs->nodes, node);
}

eprs_node_t eprs_get_rule(const eprs_t *eprs, eprs_node_t parent, estx_rule_t rule)
{
	if (eprs == NULL) {
		return EPRS_NODE_END;
	}

	eprs_node_data_t *data = tree_get(&eprs->nodes, parent);
	if (data && data->type == EPRS_NODE_RULE && data->val.rule == rule) {
		return parent;
	}

	eprs_node_t child;
	tree_foreach_child(&eprs->nodes, parent, child)
	{
		data = tree_get(&eprs->nodes, child);
		switch (data->type) {
		case EPRS_NODE_RULE:
			if (data->val.rule == rule) {
				return child;
			}
			break;
		default: break;
		}
	}

	return EPRS_NODE_END;
}

int eprs_get_str(const eprs_t *eprs, eprs_node_t parent, token_t *out)
{
	if (eprs == NULL || out == NULL) {
		return 1;
	}

	eprs_node_t child;
	tree_foreach_child(&eprs->nodes, parent, child)
	{
		eprs_node_data_t *data = tree_get(&eprs->nodes, child);
		switch (data->type) {
		case EPRS_NODE_RULE: eprs_get_str(eprs, child, out); break;
		case EPRS_NODE_TOKEN: {
			out->start = (out->len == 0 ? data->val.token.start : out->start);
			out->len += data->val.token.len;
			break;
		}
		case EPRS_NODE_LITERAL: {
			out->start = (out->len == 0 ? data->val.literal.start : out->start);
			out->len += data->val.literal.len;
			break;
		}
		case EPRS_NODE_UNKNOWN:
		default: log_error("cparse", "eparser", NULL, "unexpected node: %d", data->type); break;
		}
	}

	return 0;
}

typedef struct eprs_parse_err_s {
	estx_rule_t rule;
	uint tok;
	estx_term_t exp;
} eprs_parse_err_t;

static int eprs_parse_rule(eprs_t *prs, const estx_rule_t rule_id, uint *off, eprs_node_t node, eprs_parse_err_t *err);
static int eprs_parse_terms(eprs_t *eprs, estx_rule_t rule, estx_term_t term_id, uint *off, eprs_node_t node, eprs_parse_err_t *err);

static int eprs_parse_term(eprs_t *eprs, estx_rule_t rule, estx_term_t term_id, uint *off, eprs_node_t node, eprs_parse_err_t *err)
{
	const estx_term_data_t *term = estx_get_term_data(eprs->estx, term_id);

	switch (term->type) {
	case ESTX_TERM_RULE: {
		uint nodes_cnt	  = eprs->nodes.cnt;
		eprs_node_t child = EPRS_NODE_RULE(eprs, term->val.rule);
		uint cur	  = *off;
		if (eprs_parse_rule(eprs, term->val.rule, off, child, err)) {
			tree_set_cnt(&eprs->nodes, nodes_cnt);
			*off = cur;
			return 1;
		}
		eprs_add_node(eprs, node, child);
		return 0;
	}
	case ESTX_TERM_TOKEN: {
		const token_type_t token_type = term->val.token;

		char buf[32] = {0};

		int len = token_type_print(1 << token_type, DST_BUF(buf));

		token_t token = lex_get_token(eprs->lex, *off);

		if (token.type & (1 << token_type)) {
			eprs_add_node(
				eprs, node, EPRS_NODE_TOKEN(eprs, ((token_t){.type = token_type, .start = token.start, .len = token.len})));
			log_trace("cparse", "eparser", NULL, "%.*s: success +%d", len, buf, token.len);
			*off += token.len;
			return 0;
		}

		if (err->tok == LEX_TOKEN_END || *off >= err->tok) {
			err->rule = rule;
			err->tok  = *off;
			err->exp  = term_id;
		}
		char act[32] = {0};
		int act_len  = lex_print_token(eprs->lex, token, DST_BUF(act));
		log_trace("cparse", "eparser", NULL, "failed: expected %.*s, but got %.*s", len, buf, act_len, act);
		return 1;
	}
	case ESTX_TERM_LITERAL: {
		strv_t literal = STRVN((char *)&eprs->estx->strs.data[term->val.literal.start], term->val.literal.len);

		for (uint i = 0; i < (uint)literal.len; i++) {
			token_t token = lex_get_token(eprs->lex, *off + i);

			if (token.type & (1 << TOKEN_EOF)) {
				err->rule = rule;
				err->tok  = *off + i;
				err->exp  = term_id;
				log_trace("cparse", "eparser", NULL, "\'%*s\': failed: end of tokens", literal.len, literal.data);
				return 1;
			}

			strv_t c	 = STRVN(&literal.data[i], 1);
			strv_t token_val = lex_get_token_val(eprs->lex, token);
			if (!strv_eq(token_val, c)) {
				if (err->tok == LEX_TOKEN_END || *off + i >= err->tok) {
					err->rule = rule;
					err->tok  = *off + i;
					err->exp  = term_id;
				}

				char buf[256] = {0};
				const int len = strv_print(token_val, DST_BUF(buf));

				log_trace("cparse",
					  "eparser",
					  NULL,
					  "failed: expected \'%*s\', but got \'%.*s\'",
					  literal.len,
					  literal.data,
					  len,
					  buf);
				return 1;
			}
		}

		eprs_add_node(eprs, node, EPRS_NODE_LITERAL(eprs, *off, (uint)literal.len));
		log_trace("cparse", "parser", NULL, "\'%*s\': success +%d", literal.len, literal.data, literal.len);
		*off += (uint)literal.len;
		return 0;
	}
	case ESTX_TERM_ALT: {
		estx_term_t child_id;
		estx_term_foreach(&eprs->estx->terms, term_id, child_id)
		{
			uint cur       = *off;
			uint nodes_cnt = eprs->nodes.cnt;
			if (eprs_parse_terms(eprs, rule, child_id, off, node, err)) {
				log_trace("cparse", "eparser", NULL, "alt: failed");
				tree_set_cnt(&eprs->nodes, nodes_cnt);
				*off = cur;
			} else {
				log_trace("cparse", "parser", NULL, "alt: success");
				return 0;
			}
		}
		break;
	}
	case ESTX_TERM_CON: {
		uint cur = *off;
		estx_term_t child_id;
		estx_term_foreach(&eprs->estx->terms, term_id, child_id)
		{
			uint nodes_cnt = eprs->nodes.cnt;
			if (eprs_parse_terms(eprs, rule, child_id, off, node, err)) {
				log_trace("cparse", "eparser", NULL, "con: failed");
				tree_set_cnt(&eprs->nodes, nodes_cnt);
				*off = cur;
				return 1;
			} else {
				log_trace("cparse", "parser", NULL, "con: success");
			}
		}
		return 0;
	}
	case ESTX_TERM_GROUP: {
		uint cur = *off;
		estx_term_t child_id;
		estx_term_foreach(&eprs->estx->terms, term_id, child_id)
		{
			uint nodes_cnt = eprs->nodes.cnt;
			if (eprs_parse_terms(eprs, rule, child_id, off, node, err)) {
				log_trace("cparse", "eparser", NULL, "group: failed");
				tree_set_cnt(&eprs->nodes, nodes_cnt);
				*off = cur;
				return 1;
			} else {
				log_trace("cparse", "parser", NULL, "group: success");
			}
		}
		return 0;
	}
	default: log_warn("cparse", "eparser", NULL, "unknown term type: %d", term->type); break;
	}

	return 1;
}

static int eprs_parse_terms(eprs_t *eprs, estx_rule_t rule, estx_term_t term_id, uint *off, eprs_node_t node, eprs_parse_err_t *err)
{
	const estx_term_data_t *term = estx_get_term_data(eprs->estx, term_id);

	uint cur = *off;

	int ret = eprs_parse_term(eprs, rule, term_id, off, node, err);
	int one = term->occ == ESTX_TERM_OCC_ONE;

	if (term->type == ESTX_TERM_ALT || term->type == ESTX_TERM_CON || one) {
		return ret;
	}

	int opt = (term->occ & ESTX_TERM_OCC_OPT) && !(term->occ & ESTX_TERM_OCC_REP);
	int rep = !(term->occ & ESTX_TERM_OCC_OPT) && (term->occ & ESTX_TERM_OCC_REP);

	if (ret && opt) {
		*off = cur;
		return 0;
	}

	if (ret && rep) {
		log_trace("cparse", "eparser", NULL, "rep: failed");
		*off = cur;
		return ret;
	}

	while (ret == 0) {
		if (cur == *off) {
			log_warn("cparse", "eparser", NULL, "loop detected");
			break;
		}
		cur = *off;
		ret = eprs_parse_term(eprs, rule, term_id, off, node, err);
	}

	*off = cur;
	return 0;
}

static int eprs_parse_rule(eprs_t *prs, const estx_rule_t rule_id, uint *off, eprs_node_t node, eprs_parse_err_t *err)
{
	const estx_rule_data_t *rule = estx_get_rule_data(prs->estx, rule_id);
	if (rule == NULL) {
		log_error("cparse", "eparser", NULL, "rule not found: %d", rule_id);
		return 1;
	}

	log_trace("cparse", "eparser", NULL, "<%d>", rule_id);

	uint cur = *off;
	if (eprs_parse_terms(prs, rule_id, rule->terms, off, node, err)) {
		log_trace("cparse", "eparser", NULL, "<%d>: failed", rule_id);
		*off = cur;
		return 1;
	}

	log_trace("cparse", "eparser", NULL, "<%d>: success +%d", rule_id, *off - cur);
	return 0;
}

int eprs_parse(eprs_t *eprs, const lex_t *lex, const estx_t *estx, estx_rule_t rule, eprs_node_t *root, dst_t dst)
{
	if (eprs == NULL || lex == NULL || estx == NULL) {
		return 1;
	}

	eprs->lex  = lex;
	eprs->estx = estx;

	eprs->nodes.cnt = 0;

	eprs_parse_err_t err = {
		.rule = ESTX_RULE_END,
		.tok  = LEX_TOKEN_END,
		.exp  = ESTX_TERM_END,
	};

	eprs_node_t tmp = eprs_add_node(eprs, EPRS_NODE_END, EPRS_NODE_RULE(eprs, rule));
	uint parsed	= 0;
	if (eprs_parse_rule(eprs, rule, &parsed, tmp, &err) || parsed != eprs->lex->tokens.cnt) {
		if (err.exp == ESTX_TERM_END) {
			log_error("cparse", "parser", NULL, "wrong syntax");
			return 1;
		}

		const estx_term_data_t *term = estx_get_term_data(eprs->estx, err.exp);

		token_loc_t loc = lex_get_token_loc(eprs->lex, err.tok);

		dst.off += lex_token_loc_print_loc(eprs->lex, loc, dst);

		if (term->type == ESTX_TERM_TOKEN) {
			char buf[32] = {0};
			int len	     = token_type_print(1 << term->val.token, DST_BUF(buf));
			dst.off += dputf(dst, "error: expected %.*s\n", len, buf);

		} else {
			strv_t exp_str = STRVN((char *)&eprs->estx->strs.data[term->val.literal.start], term->val.literal.len);
			dst.off += dputf(dst, "error: expected \'%.*s\'\n", exp_str.len, exp_str.data);
		}

		dst.off += lex_token_loc_print_src(eprs->lex, loc, dst);
		return 1;
	}

	if (root) {
		*root = tmp;
	}

	log_trace("cparse", "eparser", NULL, "success");
	return 0;
}

static size_t print_nodes(void *data, dst_t dst, const void *priv)
{
	const eprs_t *eprs = priv;

	size_t off = dst.off;

	const eprs_node_data_t *node = data;
	switch (node->type) {
	case EPRS_NODE_RULE: {
		dst.off += dputf(dst, "%d\n", node->val.rule);
		break;
	}
	case EPRS_NODE_TOKEN: {
		char type[32]	  = {0};
		int type_len	  = token_type_print(1 << node->val.token.type, DST_BUF(type));
		char val[32]	  = {0};
		const int val_len = strv_print(lex_get_token_val(eprs->lex, node->val.token), DST_BUF(val));
		dst.off += dputf(dst, "%.*s(%.*s)\n", type_len, type, val_len, val);
		break;
	}
	case EPRS_NODE_LITERAL: {
		char val[32]	  = {0};
		const int val_len = strv_print(lex_get_token_val(eprs->lex, node->val.literal), DST_BUF(val));
		dst.off += dputf(dst, "\'%.*s\'\n", val_len, val);
		break;
	}
	default: break;
	}

	return dst.off - off;
}

int eprs_print(const eprs_t *eprs, eprs_node_t node, dst_t dst)
{
	if (eprs == NULL) {
		return 0;
	}
	return tree_print(&eprs->nodes, node, print_nodes, dst, eprs);
}
