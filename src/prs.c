#include "prs.h"

#include "log.h"

typedef enum prs_node_type_e {
	PRS_NODE_UNKNOWN,
	PRS_NODE_RULE,
	PRS_NODE_TOKEN,
	PRS_NODE_LITERAL,
} prs_node_type_t;

typedef struct prs_node_data_s {
	prs_node_type_t type;
	union {
		stx_node_t rule;
		tok_t literal;
		tok_t tok;
	} val;
} prs_node_data_t;

prs_t *prs_init(prs_t *prs, uint nodes_cap, alloc_t alloc)
{
	if (prs == NULL) {
		return NULL;
	}

	if (tree_init(&prs->nodes, nodes_cap, sizeof(prs_node_data_t), alloc) == NULL) {
		log_error("cparse", "prs", NULL, "failed to initialize nodes tree");
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

void prs_reset(prs_t *prs, uint cnt)
{
	if (prs == NULL) {
		return;
	}

	tree_reset(&prs->nodes, cnt);
}

int prs_node_rule(prs_t *prs, stx_node_t rule, prs_node_t *node)
{
	if (prs == NULL) {
		return 1;
	}

	if (stx_get_node(prs->stx, rule) == NULL) {
		log_error("cparse", "prs", NULL, "invalid rule: %d", rule);
		return 1;
	}

	prs_node_data_t *data = tree_node(&prs->nodes, node);
	if (data == NULL) {
		log_error("cparse", "prs", NULL, "failed to add rule node");
		return 1;
	}

	*data = (prs_node_data_t){
		.type	  = PRS_NODE_RULE,
		.val.rule = rule,
	};

	return 0;
}

int prs_node_tok(prs_t *prs, tok_t tok, prs_node_t *node)
{
	if (prs == NULL) {
		return 1;
	}

	prs_node_data_t *data = tree_node(&prs->nodes, node);
	if (data == NULL) {
		log_error("cparse", "prs", NULL, "failed to add tok node");
		return 1;
	}

	*data = (prs_node_data_t){
		.type	 = PRS_NODE_TOKEN,
		.val.tok = tok,
	};

	return 0;
}

int prs_node_lit(prs_t *prs, size_t start, uint len, prs_node_t *node)
{
	if (prs == NULL) {
		return 1;
	}

	prs_node_data_t *data = tree_node(&prs->nodes, node);
	if (data == NULL) {
		log_error("cparse", "prs", NULL, "failed to add tok node");
		return 1;
	}

	*data = (prs_node_data_t){
		.type	     = PRS_NODE_LITERAL,
		.val.literal = {.start = start, .len = len},
	};

	return 0;
}

int prs_add_node(prs_t *prs, prs_node_t parent, prs_node_t node)
{
	if (prs == NULL) {
		return 1;
	}

	return tree_add(&prs->nodes, parent, node);
}

int prs_remove_node(prs_t *prs, prs_node_t node)
{
	if (prs == NULL) {
		return 1;
	}

	return tree_remove(&prs->nodes, node);
}

int prs_get_rule(const prs_t *prs, prs_node_t parent, stx_node_t rule, prs_node_t *node)
{
	if (prs == NULL) {
		return 1;
	}

	prs_node_t child;
	const prs_node_data_t *data;
	tree_foreach_child(&prs->nodes, parent, child, data)
	{
		switch (data->type) {
		case PRS_NODE_RULE:
			if (data->val.rule == rule) {
				if (node) {
					*node = child;
				}
				return 0;
			}
			break;
		default: break;
		}
	}

	return 1;
}

int prs_get_str(const prs_t *prs, prs_node_t parent, tok_t *out)
{
	if (prs == NULL || out == NULL) {
		return 1;
	}

	prs_node_t child;
	const prs_node_data_t *data;
	tree_foreach_child(&prs->nodes, parent, child, data)
	{
		switch (data->type) {
		case PRS_NODE_RULE: prs_get_str(prs, child, out); break;
		case PRS_NODE_TOKEN: {
			out->start = (out->len == 0 ? data->val.tok.start : out->start);
			out->len += data->val.tok.len;
			break;
		}
		case PRS_NODE_LITERAL: {
			out->start = (out->len == 0 ? data->val.literal.start : out->start);
			out->len += data->val.literal.len;
			break;
		}
		default: log_error("cparse", "prs", NULL, "unexpected node: %d", data->type); break;
		}
	}

	return 0;
}

typedef struct prs_parse_err_s {
	stx_node_t rule;
	uint tok;
	stx_node_t exp;
	byte failed : 1;
} prs_parse_err_t;

static int prs_parse_rule(prs_t *prs, stx_node_t rule_id, uint *off, prs_node_t node, prs_parse_err_t *err);
static int prs_parse_terms(prs_t *prs, stx_node_t rule, stx_node_t terms, uint *off, prs_node_t node, prs_parse_err_t *err);

static int prs_parse_term(prs_t *prs, stx_node_t rule, stx_node_t term_id, uint *off, prs_node_t node, prs_parse_err_t *err)
{
	const stx_node_data_t *term = stx_get_node(prs->stx, term_id);

	switch (term->type) {
	case STX_RULE: return 0;
	case STX_TERM_RULE: {
		uint nodes_cnt = prs->nodes.cnt;
		uint cur       = *off;
		prs_node_t child;
		if (prs_node_rule(prs, term->val.rule, &child) || prs_parse_rule(prs, term->val.rule, off, child, err)) {
			prs_reset(prs, nodes_cnt);
			*off = cur;
			return 1;
		}

		prs_add_node(prs, node, child);
		return 0;
	}
	case STX_TERM_TOK: {
		const tok_type_t tok_type = term->val.tok;

		char buf[32] = {0};

		size_t len = tok_type_print(1 << tok_type, DST_BUF(buf));

		tok_t tok = lex_get_tok(prs->lex, *off);

		if (tok.type & (1 << tok_type)) {
			prs_node_t token;
			prs_node_tok(prs, (tok_t){.type = tok_type, .start = tok.start, .len = tok.len}, &token);
			prs_add_node(prs, node, token);
			log_trace("cparse", "prs", NULL, "%.*s: success +%d", (int)len, buf, tok.len);
			*off += tok.len;
			return 0;
		}

		if (!err->failed || *off >= err->tok) {
			err->rule   = rule;
			err->tok    = *off;
			err->exp    = term_id;
			err->failed = 1;
		}
		char act[32]   = {0};
		size_t act_len = lex_print_tok(prs->lex, tok, DST_BUF(act));
		log_trace("cparse", "prs", NULL, "failed: expected %.*s, but got %.*s", (int)len, buf, act_len, act);
		return 1;
	}
	case STX_TERM_LIT: {
		strv_t literal = stx_data_lit(prs->stx, term);

		for (uint i = 0; i < (uint)literal.len; i++) {
			tok_t tok = lex_get_tok(prs->lex, *off + i);

			if (tok.type & (1 << TOK_EOF)) {
				err->rule   = rule;
				err->tok    = *off + i;
				err->exp    = term_id;
				err->failed = 1;
				log_trace("cparse", "prs", NULL, "\'%*s\': failed: end of toks", literal.len, literal.data);
				return 1;
			}

			strv_t c       = STRVN(&literal.data[i], 1);
			strv_t tok_val = lex_get_tok_val(prs->lex, tok);
			if (!strv_eq(tok_val, c)) {
				if (!err->failed || *off + i >= err->tok) {
					err->rule   = rule;
					err->tok    = *off + i;
					err->exp    = term_id;
					err->failed = 1;
				}

				char buf[256] = {0};
				size_t len    = strv_print(tok_val, DST_BUF(buf));

				log_trace("cparse",
					  "prs",
					  NULL,
					  "failed: expected \'%*s\', but got \'%.*s\'",
					  literal.len,
					  literal.data,
					  (int)len,
					  buf);
				return 1;
			}
		}

		prs_node_t lit;
		prs_node_lit(prs, *off, (uint)literal.len, &lit);
		prs_add_node(prs, node, lit);
		log_trace("cparse", "prs", NULL, "\'%*s\': success +%d", literal.len, literal.data, literal.len);
		*off += (uint)literal.len;
		return 0;
	}
	case STX_TERM_OR: {
		uint nodes_cnt = prs->nodes.cnt;
		uint cur       = *off;
		if (!prs_parse_terms(prs, rule, term->val.orv.l, off, node, err)) {
			log_trace("cparse", "prs", NULL, "left: success");
			return 0;
		}

		log_trace("cparse", "prs", NULL, "left: failed");
		prs_reset(prs, nodes_cnt);

		if (!prs_parse_terms(prs, rule, term->val.orv.r, off, node, err)) {
			log_trace("cparse", "prs", NULL, "right: success");
			return 0;
		}

		log_trace("cparse", "prs", NULL, "right: failed");
		prs_reset(prs, nodes_cnt);
		*off = cur;
		return 1;
	}
	default: log_warn("cparse", "prs", NULL, "unknown term type: %d", term->type); break;
	}

	return 1;
}

static int prs_parse_terms(prs_t *prs, stx_node_t rule, stx_node_t terms, uint *off, prs_node_t node, prs_parse_err_t *err)
{
	uint cur = *off;
	const stx_node_data_t *data;
	stx_node_foreach(&prs->stx->nodes, terms, data)
	{
		if (prs_parse_term(prs, rule, terms, off, node, err)) {
			*off = cur;
			return 1;
		}
	}

	return 0;
}

static int prs_parse_rule(prs_t *prs, stx_node_t rule, uint *off, prs_node_t node, prs_parse_err_t *err)
{
	log_trace("cparse", "prs", NULL, "<%d>", rule);

	uint cur = *off;
	if (prs_parse_terms(prs, rule, rule, off, node, err)) {
		log_trace("cparse", "prs", NULL, "<%d>: failed", rule);
		*off = cur;
		return 1;
	}

	log_trace("cparse", "prs", NULL, "<%d>: success +%d", rule, *off - cur);
	return 0;
}

int prs_parse(prs_t *prs, const lex_t *lex, const stx_t *stx, stx_node_t rule, prs_node_t *root, dst_t dst)
{
	if (prs == NULL || lex == NULL || stx == NULL) {
		return 1;
	}

	prs->lex = lex;
	prs->stx = stx;

	prs_reset(prs, 0);

	prs_parse_err_t err = {0};

	prs_node_t tmp;
	prs_node_rule(prs, rule, &tmp);
	uint parsed = 0;
	if (prs_parse_rule(prs, rule, &parsed, tmp, &err) || parsed != prs->lex->src.len) {
		if (!err.failed) {
			log_error("cparse", "prs", NULL, "wrong syntax");
			return 1;
		}

		const stx_node_data_t *term = stx_get_node(prs->stx, err.exp);

		tok_loc_t loc = lex_get_tok_loc(prs->lex, err.tok);

		dst.off += lex_tok_loc_print_loc(prs->lex, loc, dst);

		if (term->type == STX_TERM_TOK) {
			char buf[32] = {0};
			size_t len   = tok_type_print(1 << term->val.tok, DST_BUF(buf));
			dst.off += dputf(dst, "error: expected %.*s\n", (int)len, buf);

		} else {
			strv_t exp_str = stx_data_lit(prs->stx, term);
			dst.off += dputf(dst, "error: expected \'%.*s\'\n", exp_str.len, exp_str.data);
		}

		dst.off += lex_tok_loc_print_src(prs->lex, loc, dst);
		return 1;
	}

	if (root) {
		*root = tmp;
	}

	log_trace("cparse", "prs", NULL, "success");
	return 0;
}

static size_t print_nodes(void *data, dst_t dst, const void *priv)
{
	const prs_t *prs = priv;

	size_t off = dst.off;

	const prs_node_data_t *node = data;
	switch (node->type) {
	case PRS_NODE_RULE: {
		stx_node_data_t *rule = stx_get_node(prs->stx, node->val.rule);
		strv_t name	      = STRVN(buf_get(&prs->stx->strs, rule->val.name.off), rule->val.name.len);
		dst.off += dputf(dst, "%.*s\n", name.len, name.data);
		break;
	}
	case PRS_NODE_TOKEN: {
		char type[32]	= {0};
		size_t type_len = tok_type_print(1 << node->val.tok.type, DST_BUF(type));
		char val[32]	= {0};
		size_t val_len	= strv_print(lex_get_tok_val(prs->lex, node->val.tok), DST_BUF(val));
		dst.off += dputf(dst, "%.*s(%.*s)\n", (int)type_len, type, (int)val_len, val);
		break;
	}
	case PRS_NODE_LITERAL: {
		char val[32]   = {0};
		size_t val_len = strv_print(lex_get_tok_val(prs->lex, node->val.literal), DST_BUF(val));
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
