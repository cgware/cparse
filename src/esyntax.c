#include "esyntax.h"

#include "log.h"

#include <stdarg.h>

estx_t *estx_init(estx_t *estx, uint rules_cap, uint terms_cap, alloc_t alloc)
{
	if (estx == NULL) {
		return NULL;
	}

	if (arr_init(&estx->rules, rules_cap, sizeof(estx_rule_data_t), alloc) == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to initialize rules array");
		return NULL;
	}

	if (tree_init(&estx->terms, terms_cap, sizeof(estx_term_data_t), alloc) == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to initialize terms tree");
		return NULL;
	}

	if (buf_init(&estx->strs, 16, alloc) == NULL) {
		log_error("cparse", "syntax", NULL, "failed to initialize strings buffer");
		return NULL;
	}

	return estx;
}

void estx_free(estx_t *estx)
{
	if (estx == NULL) {
		return;
	}

	arr_free(&estx->rules);
	tree_free(&estx->terms);
	buf_free(&estx->strs);
}

estx_rule_t estx_add_rule(estx_t *estx)
{
	if (estx == NULL) {
		return ESTX_RULE_END;
	}

	estx_rule_t rule       = estx->rules.cnt;
	estx_rule_data_t *data = arr_add(&estx->rules);
	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to add rule");
		return ESTX_RULE_END;
	}

	*data = (estx_rule_data_t){
		.terms = ESTX_TERM_END,
	};

	return rule;
}

estx_rule_data_t *estx_get_rule_data(const estx_t *estx, estx_rule_t rule)
{
	if (estx == NULL) {
		return NULL;
	}

	estx_rule_data_t *data = arr_get(&estx->rules, rule);

	if (data == NULL) {
		log_warn("cparse", "esyntax", NULL, "invalid rule: %d", rule);
		return NULL;
	}

	return data;
}

estx_term_t estx_create_term(estx_t *estx, estx_term_data_t term)
{
	if (estx == NULL) {
		return ESTX_TERM_END;
	}

	const estx_term_t child = tree_add(&estx->terms);
	estx_term_data_t *data	= tree_get_data(&estx->terms, child);

	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to create term");
		return ESTX_TERM_END;
	}

	*data = term;

	return child;
}

estx_term_data_t *estx_get_term_data(const estx_t *estx, estx_term_t term)
{
	if (estx == NULL) {
		return NULL;
	}

	estx_term_data_t *data = tree_get_data(&estx->terms, term);

	if (data == NULL) {
		log_warn("cparse", "esyntax", NULL, "invalid term: %d", term);
		return NULL;
	}

	return data;
}

estx_term_data_t estx_create_literal(estx_t *estx, strv_t str, estx_term_occ_t occ)
{
	if (estx == NULL) {
		return (estx_term_data_t){0};
	}

	size_t start = estx->strs.used;
	buf_add(&estx->strs, str.data, str.len, NULL);

	return (estx_term_data_t){.type = ESTX_TERM_LITERAL, .val.literal = {.start = start, .len = str.len}, .occ = occ};
}

estx_term_t estx_rule_set_term(estx_t *estx, estx_rule_t rule, estx_term_t term)
{
	estx_rule_data_t *data = estx_get_rule_data(estx, rule);

	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to get rule: %d", rule);
		return ESTX_TERM_END;
	}

	return data->terms = term;
}

estx_term_t estx_term_add_term(estx_t *estx, estx_term_t term, estx_term_t child)
{
	if (estx_get_term_data(estx, term) == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to get term: %d", term);
		return ESTX_TERM_END;
	}

	return tree_set_child_node(&estx->terms, term, child);
}

static int estx_term_occ_print(estx_term_occ_t occ, print_dst_t dst)
{
	if ((occ & ESTX_TERM_OCC_OPT) && (occ & ESTX_TERM_OCC_REP)) {
		return c_dprintf(dst, "*");
	}

	if (occ & ESTX_TERM_OCC_OPT) {
		return c_dprintf(dst, "?");
	}

	if (occ & ESTX_TERM_OCC_REP) {
		return c_dprintf(dst, "+");
	}

	return 0;
}

static int estx_term_print(const estx_t *estx, const estx_term_t term, print_dst_t dst)
{
	int off = dst.off;

	const estx_term_data_t *data = estx_get_term_data(estx, term);
	switch (data->type) {
	case ESTX_TERM_RULE: {
		dst.off += c_dprintf(dst, " %d", data->val.rule);
		dst.off += estx_term_occ_print(data->occ, dst);
		break;
	}
	case ESTX_TERM_TOKEN: {
		dst.off += c_dprintf(dst, " ");
		dst.off += token_type_print(1 << data->val.token, dst);
		dst.off += estx_term_occ_print(data->occ, dst);
		break;
	}
	case ESTX_TERM_LITERAL: {
		strv_t literal = STRVN((char *)&estx->strs.data[data->val.literal.start], data->val.literal.len);
		if (strv_eq(literal, STRV("'"))) {
			dst.off += c_dprintf(dst, " \"%.*s\"", literal.len, literal.data);
		} else {
			dst.off += c_dprintf(dst, " \'%.*s\'", literal.len, literal.data);
		}
		dst.off += estx_term_occ_print(data->occ, dst);
		break;
	}
	case ESTX_TERM_ALT: {
		estx_term_t child;
		int first = 1;
		tree_foreach_child(&estx->terms, term, child)
		{
			if (!first) {
				dst.off += c_dprintf(dst, " |");
			}

			dst.off += estx_term_print(estx, child, dst);
			first = 0;
		}
		break;
	}
	case ESTX_TERM_CON: {
		estx_term_t child;
		tree_foreach_child(&estx->terms, term, child)
		{
			dst.off += estx_term_print(estx, child, dst);
		}
		break;
	}
	case ESTX_TERM_GROUP: {
		estx_term_t child;
		dst.off += c_dprintf(dst, " (");
		tree_foreach_child(&estx->terms, term, child)
		{
			dst.off += estx_term_print(estx, child, dst);
		}
		dst.off += c_dprintf(dst, " )");
		dst.off += estx_term_occ_print(data->occ, dst);
		break;
	}
	default: log_warn("cparse", "esyntax", NULL, "unknown term type: %d", data->type); break;
	}

	return dst.off - off;
}

int estx_print(const estx_t *estx, print_dst_t dst)
{
	if (estx == NULL) {
		return 0;
	}

	int off = dst.off;

	const estx_rule_data_t *rule;
	arr_foreach(&estx->rules, rule)
	{
		dst.off += c_dprintf(dst, "%d =", _i);
		if (rule->terms >= estx->terms.cnt) {
			log_error("cparse", "esyntax", NULL, "failed to get rule %d terms", _i);
		} else {
			dst.off += estx_term_print(estx, rule->terms, dst);
		}
		dst.off += c_dprintf(dst, "\n");
	}

	return dst.off - off;
}

int term_print_cb(void *data, print_dst_t dst, const void *priv)
{
	const estx_term_data_t *term = data;

	const estx_t *estx = priv;

	int off = dst.off;

	switch (term->type) {
	case ESTX_TERM_RULE: {
		dst.off += c_dprintf(dst, "<%d>", term->val.rule);
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_TOKEN: {
		dst.off += token_type_print(1 << term->val.token, dst);
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_LITERAL: {
		strv_t literal = STRVN((char *)&estx->strs.data[term->val.literal.start], term->val.literal.len);
		if (strv_eq(literal, STRV("'"))) {
			dst.off += c_dprintf(dst, "\"%.*s\"", literal.len, literal.data);
		} else {
			dst.off += c_dprintf(dst, "\'%.*s\'", literal.len, literal.data);
		}
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	case ESTX_TERM_ALT: {
		dst.off += c_dprintf(dst, "alt");
		break;
	}
	case ESTX_TERM_CON: {
		dst.off += c_dprintf(dst, "con");
		break;
	}
	case ESTX_TERM_GROUP: {
		dst.off += c_dprintf(dst, "group");
		dst.off += estx_term_occ_print(term->occ, dst);
		break;
	}
	default: log_warn("cparse", "esyntax", NULL, "unknown term type: %d", term->type); break;
	}

	dst.off += c_dprintf(dst, "\n");

	return dst.off - off;
}

int estx_print_tree(const estx_t *estx, print_dst_t dst)
{
	if (estx == NULL) {
		return 0;
	}

	int off = dst.off;

	estx_rule_data_t *data;
	int first = 1;
	arr_foreach(&estx->rules, data)
	{
		if (!first) {
			dst.off += c_dprintf(dst, "\n");
		}
		dst.off += c_dprintf(dst, "<%d>\n", _i);
		dst.off += tree_print(&estx->terms, data->terms, term_print_cb, dst, estx);
		first = 0;
	}

	return dst.off - off;
}
