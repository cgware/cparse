#include "eprs.h"

#include "log.h"

typedef enum eprs_node_type_e {
	EPRS_NODE_UNKNOWN,
	EPRS_NODE_RULE,
	EPRS_NODE_TOKEN,
	EPRS_NODE_LITERAL,
} eprs_node_type_t;

typedef struct eprs_node_data_s {
	eprs_node_type_t type;
	union {
		estx_node_t rule;
		tok_t literal;
		tok_t tok;
		int alt;
	} val;
} eprs_node_data_t;

eprs_t *eprs_init(eprs_t *eprs, uint nodes_cap, alloc_t alloc)
{
	if (eprs == NULL) {
		return NULL;
	}

	if (tree_init(&eprs->nodes, nodes_cap, sizeof(eprs_node_data_t), alloc) == NULL) {
		log_error("cparse", "eprs", NULL, "failed to initialize nodes tree");
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

void eprs_reset(eprs_t *eprs, uint cnt)
{
	if (eprs == NULL) {
		return;
	}

	tree_reset(&eprs->nodes, cnt);
}

int eprs_node_rule(eprs_t *eprs, estx_node_t rule, eprs_node_t *node)
{
	if (eprs == NULL) {
		return 1;
	}

	eprs_node_data_t *data = tree_node(&eprs->nodes, node);
	if (data == NULL) {
		log_error("cparse", "eprs", NULL, "failed to add rule node");
		return 1;
	}

	*data = (eprs_node_data_t){
		.type	  = EPRS_NODE_RULE,
		.val.rule = rule,
	};

	return 0;
}

int eprs_node_tok(eprs_t *eprs, tok_t tok, eprs_node_t *node)
{
	if (eprs == NULL) {
		return 1;
	}

	eprs_node_data_t *data = tree_node(&eprs->nodes, node);
	if (data == NULL) {
		log_error("cparse", "eprs", NULL, "failed to add tok node");
		return 1;
	}

	*data = (eprs_node_data_t){
		.type	 = EPRS_NODE_TOKEN,
		.val.tok = tok,
	};

	return 0;
}

int eprs_node_lit(eprs_t *eprs, size_t start, uint len, eprs_node_t *node)
{
	if (eprs == NULL) {
		return 1;
	}

	eprs_node_data_t *data = tree_node(&eprs->nodes, node);
	if (data == NULL) {
		log_error("cparse", "eprs", NULL, "failed to add literal node");
		return 1;
	}

	*data = (eprs_node_data_t){
		.type	 = EPRS_NODE_LITERAL,
		.val.tok = {.start = start, .len = len},
	};

	return 0;
}

int eprs_add_node(eprs_t *eprs, eprs_node_t parent, eprs_node_t node)
{
	if (eprs == NULL) {
		return 1;
	}

	return tree_add(&eprs->nodes, parent, node);
}

int eprs_remove_node(eprs_t *eprs, eprs_node_t node)
{
	if (eprs == NULL) {
		return 1;
	}

	return tree_remove(&eprs->nodes, node);
}

int eprs_get_rule(const eprs_t *eprs, eprs_node_t parent, estx_node_t rule, eprs_node_t *node)
{
	if (eprs == NULL) {
		return 1;
	}

	const eprs_node_data_t *data = tree_get(&eprs->nodes, parent);
	if (data && data->type == EPRS_NODE_RULE && data->val.rule == rule) {
		if (node) {
			*node = parent;
		}
		return 0;
	}

	eprs_node_t child;
	tree_foreach_child(&eprs->nodes, parent, child, data)
	{
		switch (data->type) {
		case EPRS_NODE_RULE:
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

int eprs_get_str(const eprs_t *eprs, eprs_node_t parent, tok_t *out)
{
	if (eprs == NULL || out == NULL) {
		return 1;
	}

	eprs_node_t child;
	const eprs_node_data_t *data;
	tree_foreach_child(&eprs->nodes, parent, child, data)
	{
		switch (data->type) {
		case EPRS_NODE_RULE: eprs_get_str(eprs, child, out); break;
		case EPRS_NODE_TOKEN: {
			out->start = (out->len == 0 ? data->val.tok.start : out->start);
			out->len += data->val.tok.len;
			break;
		}
		case EPRS_NODE_LITERAL: {
			out->start = (out->len == 0 ? data->val.literal.start : out->start);
			out->len += data->val.literal.len;
			break;
		}
		case EPRS_NODE_UNKNOWN:
		default: log_error("cparse", "eprs", NULL, "unexpected node: %d", data->type); break;
		}
	}

	return 0;
}

typedef struct eprs_parse_err_s {
	estx_node_t rule;
	uint tok;
	estx_node_t exp;
	byte failed : 1;
} eprs_parse_err_t;

static int eprs_parse_rule(eprs_t *prs, const estx_node_t rule_id, uint *off, eprs_node_t node, eprs_parse_err_t *err);
static int eprs_parse_terms(eprs_t *eprs, estx_node_t rule, estx_node_t term_id, uint *off, eprs_node_t node, eprs_parse_err_t *err,
			    const estx_node_data_t *term);

static int eprs_parse_term(eprs_t *eprs, estx_node_t rule, estx_node_t term_id, uint *off, eprs_node_t node, eprs_parse_err_t *err,
			   const estx_node_data_t *term)
{
	switch (term->type) {
	case ESTX_RULE: {
		estx_node_t terms;
		estx_node_data_t *data = list_get_next(&eprs->estx->nodes, term_id, &terms);
		return eprs_parse_terms(eprs, rule, terms, off, node, err, data);
	}
	case ESTX_TERM_RULE: {
		uint nodes_cnt = eprs->nodes.cnt;
		eprs_node_t child;
		uint cur = *off;
		if (eprs_node_rule(eprs, term->val.rule, &child) || eprs_parse_rule(eprs, term->val.rule, off, child, err)) {
			eprs_reset(eprs, nodes_cnt);
			*off = cur;
			return 1;
		}
		eprs_add_node(eprs, node, child);
		return 0;
	}
	case ESTX_TERM_TOK: {
		const tok_type_t tok_type = term->val.tok;

		char buf[32] = {0};

		size_t len = tok_type_print(1 << tok_type, DST_BUF(buf));

		tok_t tok = lex_get_tok(eprs->lex, *off);

		if (tok.type & (1 << tok_type)) {
			eprs_node_t token;
			eprs_node_tok(eprs, (tok_t){.type = tok_type, .start = tok.start, .len = tok.len}, &token);
			eprs_add_node(eprs, node, token);
			log_trace("cparse", "eprs", NULL, "%.*s: success +%d", len, buf, tok.len);
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
		size_t act_len = lex_print_tok(eprs->lex, tok, DST_BUF(act));
		log_trace("cparse", "eprs", NULL, "failed: expected %.*s, but got %.*s", len, buf, (int)act_len, act);
		return 1;
	}
	case ESTX_TERM_LIT: {
		strv_t literal = estx_data_lit(eprs->estx, term);

		for (uint i = 0; i < (uint)literal.len; i++) {
			tok_t tok = lex_get_tok(eprs->lex, *off + i);

			if (tok.type & (1 << TOK_EOF)) {
				err->rule   = rule;
				err->tok    = *off + i;
				err->exp    = term_id;
				err->failed = 1;
				log_trace("cparse", "eprs", NULL, "\'%*s\': failed: end of toks", literal.len, literal.data);
				return 1;
			}

			strv_t c       = STRVN(&literal.data[i], 1);
			strv_t tok_val = lex_get_tok_val(eprs->lex, tok);
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
					  "eprs",
					  NULL,
					  "failed: expected \'%.*s\', but got \'%.*s\'",
					  literal.len,
					  literal.data,
					  (int)len,
					  buf);
				return 1;
			}
		}

		eprs_node_t lit;
		eprs_node_lit(eprs, *off, (uint)literal.len, &lit);
		eprs_add_node(eprs, node, lit);
		log_trace("cparse", "eprs", NULL, "\'%*s\': success +%d", literal.len, literal.data, literal.len);
		*off += (uint)literal.len;
		return 0;
	}
	case ESTX_TERM_ALT: {
		estx_node_t terms = term->val.terms;
		estx_node_foreach(&eprs->estx->nodes, terms, term)
		{
			uint cur       = *off;
			uint nodes_cnt = eprs->nodes.cnt;
			if (eprs_parse_terms(eprs, rule, terms, off, node, err, term)) {
				log_trace("cparse", "eprs", NULL, "alt: failed");
				eprs_reset(eprs, nodes_cnt);
				*off = cur;
			} else {
				log_trace("cparse", "eprs", NULL, "alt: success");
				return 0;
			}
		}
		break;
	}
	case ESTX_TERM_CON: {
		uint cur	  = *off;
		estx_node_t terms = term->val.terms;
		estx_node_foreach(&eprs->estx->nodes, terms, term)
		{
			uint nodes_cnt = eprs->nodes.cnt;
			if (eprs_parse_terms(eprs, rule, terms, off, node, err, term)) {
				log_trace("cparse", "eprs", NULL, "con: failed");
				eprs_reset(eprs, nodes_cnt);
				*off = cur;
				return 1;
			} else {
				log_trace("cparse", "eprs", NULL, "con: success");
			}
		}
		return 0;
	}
	case ESTX_TERM_GROUP: {
		uint cur	  = *off;
		estx_node_t terms = term->val.terms;
		estx_node_foreach(&eprs->estx->nodes, terms, term)
		{
			uint nodes_cnt = eprs->nodes.cnt;
			if (eprs_parse_terms(eprs, rule, terms, off, node, err, term)) {
				log_trace("cparse", "eprs", NULL, "group: failed");
				eprs_reset(eprs, nodes_cnt);
				*off = cur;
				return 1;
			} else {
				log_trace("cparse", "eprs", NULL, "group: success");
			}
		}
		return 0;
	}
	default: log_warn("cparse", "eprs", NULL, "unknown term type: %d", term->type); break;
	}

	return 1;
}

static int eprs_parse_terms(eprs_t *eprs, estx_node_t rule, estx_node_t term_id, uint *off, eprs_node_t node, eprs_parse_err_t *err,
			    const estx_node_data_t *term)
{
	if (term == NULL) {
		return 1;
	}
	uint cur = *off;

	int ret = eprs_parse_term(eprs, rule, term_id, off, node, err, term);
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
		log_trace("cparse", "eprs", NULL, "rep: failed");
		*off = cur;
		return ret;
	}

	while (ret == 0) {
		if (cur == *off) {
			log_warn("cparse", "eprs", NULL, "loop detected: %d", cur);
			break;
		}
		cur = *off;
		ret = eprs_parse_term(eprs, rule, term_id, off, node, err, term);
	}

	*off = cur;
	return 0;
}

static int eprs_parse_rule(eprs_t *prs, const estx_node_t rule, uint *off, eprs_node_t node, eprs_parse_err_t *err)
{
	log_trace("cparse", "eprs", NULL, "<%d>", rule);

	uint cur		     = *off;
	const estx_node_data_t *term = estx_get_node(prs->estx, rule);
	if (eprs_parse_terms(prs, rule, rule, off, node, err, term)) {
		log_trace("cparse", "eprs", NULL, "<%d>: failed", rule);
		*off = cur;
		return 1;
	}

	log_trace("cparse", "eprs", NULL, "<%d>: success +%d", rule, *off - cur);
	return 0;
}

int eprs_parse(eprs_t *eprs, const lex_t *lex, const estx_t *estx, estx_node_t rule, eprs_node_t *root, dst_t dst)
{
	if (eprs == NULL || lex == NULL || estx == NULL) {
		return 1;
	}

	eprs->lex  = lex;
	eprs->estx = estx;

	eprs_reset(eprs, 0);

	eprs_parse_err_t err = {0};

	eprs_node_t tmp;
	eprs_node_rule(eprs, rule, &tmp);
	uint parsed = 0;
	if (eprs_parse_rule(eprs, rule, &parsed, tmp, &err) || parsed != eprs->lex->toks.cnt) {
		if (!err.failed) {
			log_error("cparse", "eprs", NULL, "wrong syntax");
			return 1;
		}

		const estx_node_data_t *term = estx_get_node(eprs->estx, err.exp);

		tok_loc_t loc = lex_get_tok_loc(eprs->lex, err.tok);

		dst.off += lex_tok_loc_print_loc(eprs->lex, loc, dst);

		if (term->type == ESTX_TERM_TOK) {
			char buf[32] = {0};
			size_t len   = tok_type_print(1 << term->val.tok, DST_BUF(buf));
			dst.off += dputf(dst, "error: expected %.*s\n", (int)len, buf);

		} else {
			strv_t exp_str = estx_data_lit(eprs->estx, term);
			dst.off += dputf(dst, "error: expected \'%.*s\'\n", exp_str.len, exp_str.data);
		}

		dst.off += lex_tok_loc_print_src(eprs->lex, loc, dst);
		return 1;
	}

	if (root) {
		*root = tmp;
	}

	log_trace("cparse", "eprs", NULL, "success");
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
		char type[32]	= {0};
		size_t type_len = tok_type_print(1 << node->val.tok.type, DST_BUF(type));
		char val[32]	= {0};
		size_t val_len	= strv_print(lex_get_tok_val(eprs->lex, node->val.tok), DST_BUF(val));
		dst.off += dputf(dst, "%.*s(%.*s)\n", type_len, type, val_len, val);
		break;
	}
	case EPRS_NODE_LITERAL: {
		char val[32]   = {0};
		size_t val_len = strv_print(lex_get_tok_val(eprs->lex, node->val.literal), DST_BUF(val));
		dst.off += dputf(dst, "\'%.*s\'\n", val_len, val);
		break;
	}
	default: break;
	}

	return dst.off - off;
}

size_t eprs_print(const eprs_t *eprs, eprs_node_t node, dst_t dst)
{
	if (eprs == NULL) {
		return 0;
	}
	return tree_print(&eprs->nodes, node, print_nodes, dst, eprs);
}
