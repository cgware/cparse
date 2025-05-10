#include "estx.h"

#include "log.h"

estx_t *estx_init(estx_t *estx, uint nodes_cap, alloc_t alloc)
{
	if (estx == NULL) {
		return NULL;
	}

	if (tree_init(&estx->nodes, nodes_cap, sizeof(estx_node_data_t), alloc) == NULL ||
	    buf_init(&estx->strs, nodes_cap * 8, alloc) == NULL) {
		log_error("cparse", "estx", NULL, "failed to initialize nodes");
		return NULL;
	}

	return estx;
}

void estx_free(estx_t *estx)
{
	if (estx == NULL) {
		return;
	}

	tree_free(&estx->nodes);
	buf_free(&estx->strs);
}

int estx_rule(estx_t *estx, strv_t name, estx_node_t *rule)
{
	if (estx == NULL) {
		return 1;
	}

	loc_t rule_name = {.len = name.len};

	size_t used = estx->strs.used;
	if (buf_add(&estx->strs, name.data, name.len, &rule_name.off)) {
		log_error("cparse", "estx", NULL, "failed to add rule name");
		return 1;
	}

	estx_node_data_t *data = tree_add(&estx->nodes, rule);
	if (data == NULL) {
		buf_reset(&estx->strs, used);
		log_error("cparse", "stx", NULL, "failed to add rule");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	  = ESTX_RULE,
		.val.name = rule_name,
	};

	return 0;
}

int estx_term_rule(estx_t *estx, estx_node_t rule, estx_node_occ_t occ, estx_node_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_node_data_t *data = estx_get_node(estx, rule);
	if (data == NULL || data->type != ESTX_RULE) {
		log_error("cparse", "estx", NULL, "invalid rule: %d", rule);
		return 1;
	}

	data = tree_add(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create rule term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	  = ESTX_TERM_RULE,
		.val.rule = rule,
		.occ	  = occ,
	};

	return 0;
}

int estx_term_tok(estx_t *estx, tok_type_t tok, estx_node_occ_t occ, estx_node_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_node_data_t *data = tree_add(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create tok term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	 = ESTX_TERM_TOK,
		.val.tok = tok,
		.occ	 = occ,
	};

	return 0;
}

int estx_term_lit(estx_t *estx, strv_t str, estx_node_occ_t occ, estx_node_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	loc_t lit = {.len = str.len};

	size_t used = estx->strs.used;
	if (buf_add(&estx->strs, str.data, str.len, &lit.off)) {
		log_error("cparse", "estx", NULL, "failed to add literal string");
		return 1;
	}

	estx_node_data_t *data = tree_add(&estx->nodes, term);
	if (data == NULL) {
		buf_reset(&estx->strs, used);
		log_error("cparse", "estx", NULL, "failed to create literal term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	 = ESTX_TERM_LIT,
		.val.lit = lit,
		.occ	 = occ,
	};

	return 0;
}

int estx_term_alt(estx_t *estx, estx_node_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_node_data_t *data = tree_add(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create alternative term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type = ESTX_TERM_ALT,
	};

	return 0;
}

int estx_term_con(estx_t *estx, estx_node_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_node_data_t *data = tree_add(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create concat term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type = ESTX_TERM_CON,
	};

	return 0;
}

int estx_term_group(estx_t *estx, estx_node_occ_t occ, estx_node_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_node_data_t *data = tree_add(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create group term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type = ESTX_TERM_GROUP,
		.occ  = occ,
	};

	return 0;
}

int estx_find_rule(estx_t *estx, strv_t name, estx_node_t *rule)
{
	if (estx == NULL) {
		return 1;
	}

	uint i = 0;
	estx_node_foreach_all(&estx->nodes, i)
	{
		estx_node_data_t *node = tree_get(&estx->nodes, i);
		if (node->type != ESTX_RULE) {
			continue;
		}

		if (strv_eq(STRVN(buf_get(&estx->strs, node->val.name.off), node->val.name.len), name)) {
			*rule = i;
			return 0;
		}
	}

	return 1;
}

estx_node_data_t *estx_get_node(const estx_t *estx, estx_node_t node)
{
	if (estx == NULL) {
		return NULL;
	}

	estx_node_data_t *data = tree_get(&estx->nodes, node);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "invalid node: %d", node);
		return NULL;
	}

	return data;
}

strv_t estx_data_lit(const estx_t *estx, const estx_node_data_t *data)
{
	if (estx == NULL || data == NULL || data->type != ESTX_TERM_LIT) {
		return STRV_NULL;
	}

	return STRVN(buf_get(&estx->strs, data->val.lit.off), data->val.lit.len);
}

int estx_add_term(estx_t *estx, estx_node_t term, estx_node_t child)
{
	if (estx_get_node(estx, term) == NULL) {
		log_error("cparse", "estx", NULL, "failed to get term: %d", term);
		return 1;
	}

	return tree_set_child(&estx->nodes, term, child);
}

static size_t estx_term_occ_print(estx_node_occ_t occ, dst_t dst)
{
	if ((occ & ESTX_TERM_OCC_OPT) && (occ & ESTX_TERM_OCC_REP)) {
		return dputs(dst, STRV("*"));
	}

	if (occ & ESTX_TERM_OCC_OPT) {
		return dputs(dst, STRV("?"));
	}

	if (occ & ESTX_TERM_OCC_REP) {
		return dputs(dst, STRV("+"));
	}

	return 0;
}

static size_t estx_term_print(const estx_t *estx, const estx_node_t term_id, const estx_node_data_t *term, dst_t dst)
{
	size_t off = dst.off;

	if (term == NULL) {
		return 0;
	}

	switch (term->type) {
	case ESTX_RULE: {
		estx_node_t terms;
		const estx_node_data_t *data = tree_get_child(&estx->nodes, term_id, &terms);
		dst.off += estx_term_print(estx, terms, data, dst);
		break;
	}
	case ESTX_TERM_RULE: {
		const estx_node_data_t *rule = tree_get(&estx->nodes, term->val.rule);
		if (rule == NULL) {
			break;
		}
		strv_t name = STRVN(buf_get(&estx->strs, rule->val.name.off), rule->val.name.len);
		dst.off += dputf(dst, " %.*s", name.len, name.data);
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_TOK: {
		dst.off += dputs(dst, STRV(" "));
		dst.off += tok_type_print(1 << term->val.tok, dst);
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_LIT: {
		strv_t literal = estx_data_lit(estx, term);
		if (strv_eq(literal, STRV("'"))) {
			dst.off += dputf(dst, " \"%.*s\"", literal.len, literal.data);
		} else {
			dst.off += dputf(dst, " \'%.*s\'", literal.len, literal.data);
		}
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_ALT: {
		estx_node_t child;
		int first = 1;
		const estx_node_data_t *data;
		tree_foreach_child(&estx->nodes, term_id, child, data)
		{
			if (!first) {
				dst.off += dputs(dst, STRV(" |"));
			}

			dst.off += estx_term_print(estx, child, data, dst);
			first = 0;
		}
		break;
	}
	case ESTX_TERM_CON: {
		estx_node_t child;
		const estx_node_data_t *data;
		tree_foreach_child(&estx->nodes, term_id, child, data)
		{
			dst.off += estx_term_print(estx, child, data, dst);
		}
		break;
	}
	case ESTX_TERM_GROUP: {
		estx_node_t child;
		const estx_node_data_t *data;
		dst.off += dputs(dst, STRV(" ("));
		tree_foreach_child(&estx->nodes, term_id, child, data)
		{
			dst.off += estx_term_print(estx, child, data, dst);
		}
		dst.off += dputs(dst, STRV(" )"));
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	default: log_warn("cparse", "estx", NULL, "unknown term type: %d", term->type); break;
	}

	return dst.off - off;
}

size_t estx_print(const estx_t *estx, dst_t dst)
{
	if (estx == NULL) {
		return 0;
	}

	size_t off = dst.off;

	uint i = 0;
	estx_node_foreach_all(&estx->nodes, i)
	{
		const estx_node_data_t *node = tree_get(&estx->nodes, i);
		if (node->type != ESTX_RULE) {
			continue;
		}

		strv_t name = STRVN(buf_get(&estx->strs, node->val.name.off), node->val.name.len);
		dst.off += dputf(dst, "%.*s =", name.len, name.data);
		dst.off += estx_term_print(estx, i, node, dst);
		dst.off += dputs(dst, STRV("\n"));
	}

	return dst.off - off;
}

size_t term_print_cb(void *data, dst_t dst, const void *priv)
{
	const estx_node_data_t *term = data;

	const estx_t *estx = priv;

	size_t off = dst.off;

	switch (term->type) {
	case ESTX_RULE: {
		dst.off += dputs(dst, STRV("rule"));
		break;
	}
	case ESTX_TERM_RULE: {
		const estx_node_data_t *rule = tree_get(&estx->nodes, term->val.rule);
		if (rule == NULL) {
			break;
		}
		strv_t name = STRVN(buf_get(&estx->strs, rule->val.name.off), rule->val.name.len);
		dst.off += dputf(dst, "<%.*s>", name.len, name.data);
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_TOK: {
		dst.off += tok_type_print(1 << term->val.tok, dst);
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_LIT: {
		strv_t literal = estx_data_lit(estx, term);
		if (strv_eq(literal, STRV("'"))) {
			dst.off += dputf(dst, "\"%.*s\"", literal.len, literal.data);
		} else {
			dst.off += dputf(dst, "\'%.*s\'", literal.len, literal.data);
		}
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_ALT: {
		dst.off += dputs(dst, STRV("alt"));
		break;
	}
	case ESTX_TERM_CON: {
		dst.off += dputs(dst, STRV("con"));
		break;
	}
	case ESTX_TERM_GROUP: {
		dst.off += dputs(dst, STRV("group"));
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	default: log_warn("cparse", "estx", NULL, "unknown term type: %d", term->type); break;
	}

	dst.off += dputs(dst, STRV("\n"));

	return dst.off - off;
}

size_t estx_print_tree(const estx_t *estx, dst_t dst)
{
	if (estx == NULL) {
		return 0;
	}

	size_t off = dst.off;

	int first = 1;
	uint i	  = 0;
	estx_node_foreach_all(&estx->nodes, i)
	{
		const estx_node_data_t *data = tree_get(&estx->nodes, i);
		if (data->type != ESTX_RULE) {
			continue;
		}

		if (!first) {
			dst.off += dputs(dst, STRV("\n"));
		}
		strv_t name = STRVN(buf_get(&estx->strs, data->val.name.off), data->val.name.len);
		dst.off += dputf(dst, "<%.*s>\n", name.len, name.data);
		dst.off += tree_print(&estx->nodes, i, term_print_cb, dst, estx);
		first = 0;
	}

	return dst.off - off;
}
