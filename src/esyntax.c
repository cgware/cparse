#include "esyntax.h"

#include "log.h"

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
		return (uint)-1;
	}

	estx_rule_t rule;
	estx_rule_data_t *data = arr_add(&estx->rules, &rule);
	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to add rule");
		return (uint)-1;
	}

	*data = (estx_rule_data_t){
		.terms = (uint)-1,
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

int estx_term_rule(estx_t *estx, estx_rule_t rule, estx_term_occ_t occ, estx_term_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_term_data_t *data = tree_add(&estx->terms, term);
	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to create rule term");
		return 1;
	}

	*data = (estx_term_data_t){
		.type	  = ESTX_TERM_RULE,
		.val.rule = rule,
		.occ	  = occ,
	};

	return 0;
}

int estx_term_tok(estx_t *estx, token_type_t token, estx_term_occ_t occ, estx_term_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_term_data_t *data = tree_add(&estx->terms, term);
	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to create token term");
		return 1;
	}

	*data = (estx_term_data_t){
		.type	   = ESTX_TERM_TOKEN,
		.val.token = token,
		.occ	   = occ,
	};

	return 0;
}

int estx_term_lit(estx_t *estx, strv_t str, estx_term_occ_t occ, estx_term_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_term_data_t *data = tree_add(&estx->terms, term);
	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to create literal term");
		return 1;
	}

	size_t start;
	if (buf_add(&estx->strs, str.data, str.len, &start)) {
		return 1;
	}

	*data = (estx_term_data_t){
		.type	     = ESTX_TERM_LITERAL,
		.val.literal = {.start = start, .len = (uint)str.len},
		.occ	     = occ,
	};

	return 0;
}

int estx_term_alt(estx_t *estx, estx_term_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_term_data_t *data = tree_add(&estx->terms, term);
	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to create alternative term");
		return 1;
	}

	*data = (estx_term_data_t){
		.type = ESTX_TERM_ALT,
	};

	return 0;
}

int estx_term_con(estx_t *estx, estx_term_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_term_data_t *data = tree_add(&estx->terms, term);
	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to create concat term");
		return 1;
	}

	*data = (estx_term_data_t){
		.type = ESTX_TERM_CON,
	};

	return 0;
}

int estx_term_group(estx_t *estx, estx_term_occ_t occ, estx_term_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_term_data_t *data = tree_add(&estx->terms, term);
	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to create group term");
		return 1;
	}

	*data = (estx_term_data_t){
		.type = ESTX_TERM_GROUP,
		.occ  = occ,
	};

	return 0;
}

estx_term_data_t *estx_get_term_data(const estx_t *estx, estx_term_t term)
{
	if (estx == NULL) {
		return NULL;
	}

	estx_term_data_t *data = tree_get(&estx->terms, term);

	if (data == NULL) {
		log_warn("cparse", "esyntax", NULL, "invalid term: %d", term);
		return NULL;
	}

	return data;
}

int estx_rule_set_term(estx_t *estx, estx_rule_t rule, estx_term_t term)
{
	estx_rule_data_t *data = estx_get_rule_data(estx, rule);

	if (data == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to get rule: %d", rule);
		return 1;
	}

	data->terms = term;
	return 0;
}

int estx_term_add_term(estx_t *estx, estx_term_t term, estx_term_t child)
{
	if (estx_get_term_data(estx, term) == NULL) {
		log_error("cparse", "esyntax", NULL, "failed to get term: %d", term);
		return 1;
	}

	return tree_set_child(&estx->terms, term, child);
}

static size_t estx_term_occ_print(estx_term_occ_t occ, dst_t dst)
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

static size_t estx_term_print(const estx_t *estx, const estx_term_t term, dst_t dst)
{
	size_t off = dst.off;

	const estx_term_data_t *data = estx_get_term_data(estx, term);
	if (data == NULL) {
		return 0;
	}

	switch (data->type) {
	case ESTX_TERM_RULE: {
		dst.off += dputf(dst, " %d", data->val.rule);
		dst.off += estx_term_occ_print(data->occ, dst);
		break;
	}
	case ESTX_TERM_TOKEN: {
		dst.off += dputs(dst, STRV(" "));
		dst.off += token_type_print(1 << data->val.token, dst);
		dst.off += estx_term_occ_print(data->occ, dst);
		break;
	}
	case ESTX_TERM_LITERAL: {
		strv_t literal = STRVN((char *)&estx->strs.data[data->val.literal.start], data->val.literal.len);
		if (strv_eq(literal, STRV("'"))) {
			dst.off += dputf(dst, " \"%.*s\"", literal.len, literal.data);
		} else {
			dst.off += dputf(dst, " \'%.*s\'", literal.len, literal.data);
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
				dst.off += dputs(dst, STRV(" |"));
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
		dst.off += dputs(dst, STRV(" ("));
		tree_foreach_child(&estx->terms, term, child)
		{
			dst.off += estx_term_print(estx, child, dst);
		}
		dst.off += dputs(dst, STRV(" )"));
		dst.off += estx_term_occ_print(data->occ, dst);
		break;
	}
	default: log_warn("cparse", "esyntax", NULL, "unknown term type: %d", data->type); break;
	}

	return dst.off - off;
}

size_t estx_print(const estx_t *estx, dst_t dst)
{
	if (estx == NULL) {
		return 0;
	}

	size_t off = dst.off;

	const estx_rule_data_t *rule;
	uint i = 0;
	arr_foreach(&estx->rules, i, rule)
	{
		dst.off += dputf(dst, "%d =", i);
		dst.off += estx_term_print(estx, rule->terms, dst);
		dst.off += dputs(dst, STRV("\n"));
	}

	return dst.off - off;
}

size_t term_print_cb(void *data, dst_t dst, const void *priv)
{
	const estx_term_data_t *term = data;

	const estx_t *estx = priv;

	size_t off = dst.off;

	switch (term->type) {
	case ESTX_TERM_RULE: {
		dst.off += dputf(dst, "<%d>", term->val.rule);
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
	default: log_warn("cparse", "esyntax", NULL, "unknown term type: %d", term->type); break;
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

	estx_rule_data_t *data;
	int first = 1;
	uint i	  = 0;
	arr_foreach(&estx->rules, i, data)
	{
		if (!first) {
			dst.off += dputs(dst, STRV("\n"));
		}
		dst.off += dputf(dst, "<%d>\n", i);
		dst.off += tree_print(&estx->terms, data->terms, term_print_cb, dst, estx);
		first = 0;
	}

	return dst.off - off;
}
