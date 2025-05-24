#include "estx.h"

#include "log.h"

estx_t *estx_init(estx_t *estx, uint nodes_cap, alloc_t alloc)
{
	if (estx == NULL) {
		return NULL;
	}

	if (list_init(&estx->nodes, nodes_cap, sizeof(estx_node_data_t), alloc) == NULL ||
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

	list_free(&estx->nodes);
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

	estx_node_data_t *data = list_node(&estx->nodes, rule);
	if (data == NULL) {
		buf_reset(&estx->strs, used);
		log_error("cparse", "stx", NULL, "failed to add rule");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	  = ESTX_RULE,
		.val.name = rule_name,
	};

	if (rule) {
		log_trace("cparse", "estx", NULL, "created rule('%.*s'): %d", name.len, name.data, *rule);
	}

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

	data = list_node(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create rule term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	  = ESTX_TERM_RULE,
		.val.rule = rule,
		.occ	  = occ,
	};

	if (term) {
		log_trace("cparse", "estx", NULL, "created rule(%d) term: %d", rule, *term);
	}

	return 0;
}

int estx_term_tok(estx_t *estx, tok_type_t tok, estx_node_occ_t occ, estx_node_t *term)
{
	if (estx == NULL) {
		return 1;
	}

	estx_node_data_t *data = list_node(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create tok term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	 = ESTX_TERM_TOK,
		.val.tok = tok,
		.occ	 = occ,
	};

	if (term) {
		log_trace("cparse", "estx", NULL, "created tok(%d) term: %d", tok, *term);
	}

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

	estx_node_data_t *data = list_node(&estx->nodes, term);
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

	if (term) {
		log_trace("cparse", "estx", NULL, "created lit('%.*s') term: %d", str.len, str.data, *term);
	}

	return 0;
}

int estx_term_alt(estx_t *estx, estx_node_t terms, estx_node_t *term)
{
	if (estx_get_node(estx, terms) == NULL) {
		return 1;
	}

	estx_node_data_t *data = list_node(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create alt term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	   = ESTX_TERM_ALT,
		.val.terms = terms,
	};

	if (term) {
		log_trace("cparse", "estx", NULL, "created alt(%d) literal term: %d", terms, *term);
	}

	return 0;
}

int estx_term_con(estx_t *estx, estx_node_t terms, estx_node_t *term)
{
	if (estx_get_node(estx, terms) == NULL) {
		return 1;
	}

	estx_node_data_t *data = list_node(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create con term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	   = ESTX_TERM_CON,
		.val.terms = terms,
	};

	if (term) {
		log_trace("cparse", "estx", NULL, "created con(%d) term: %d", terms, *term);
	}

	return 0;
}

int estx_term_group(estx_t *estx, estx_node_t terms, estx_node_occ_t occ, estx_node_t *term)
{
	if (estx_get_node(estx, terms) == NULL) {
		return 1;
	}

	estx_node_data_t *data = list_node(&estx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "estx", NULL, "failed to create group term");
		return 1;
	}

	*data = (estx_node_data_t){
		.type	   = ESTX_TERM_GROUP,
		.val.terms = terms,
		.occ	   = occ,
	};

	if (term) {
		log_trace("cparse", "estx", NULL, "created group(%d) term: %d", terms, *term);
	}

	return 0;
}

int estx_find_rule(estx_t *estx, strv_t name, estx_node_t *rule)
{
	if (estx == NULL) {
		return 1;
	}

	uint i = 0;
	const estx_node_data_t *node;
	estx_node_foreach_all(&estx->nodes, i, node)
	{
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

	estx_node_data_t *data = list_get(&estx->nodes, node);
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

int estx_add_term(estx_t *estx, estx_node_t node, estx_node_t term)
{
	if (estx == NULL) {
		return 1;
	}

	if (list_app(&estx->nodes, node, term)) {
		log_error("cparse", "estx", NULL, "failed to add term %d to node %d", term, node);
		return 1;
	}

	log_trace("cparse", "estx", NULL, "added %d to %d", term, node);

	return 0;
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

static size_t estx_term_print(const estx_t *estx, const estx_node_data_t *term, dst_t dst)
{
	size_t off = dst.off;

	switch (term->type) {
	case ESTX_RULE: break;
	case ESTX_TERM_RULE: {
		const estx_node_data_t *rule = list_get(&estx->nodes, term->val.rule);
		if (rule == NULL) {
			break;
		}
		dst.off += dputs(dst, STRVN(buf_get(&estx->strs, rule->val.name.off), rule->val.name.len));
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
		int first = 1;
		const estx_node_data_t *data;
		estx_node_t terms = term->val.terms;
		list_foreach(&estx->nodes, terms, data)
		{
			if (!first) {
				dst.off += dputs(dst, STRV(" | "));
			}

			dst.off += estx_term_print(estx, data, dst);
			first = 0;
		}
		break;
	}
	case ESTX_TERM_CON: {
		int first = 1;
		const estx_node_data_t *data;
		estx_node_t terms = term->val.terms;
		list_foreach(&estx->nodes, terms, data)
		{
			if (!first) {
				dst.off += dputs(dst, STRV(" "));
			}

			dst.off += estx_term_print(estx, data, dst);
			first = 0;
		}
		break;
	}
	case ESTX_TERM_GROUP: {
		const estx_node_data_t *data;
		estx_node_t terms = term->val.terms;
		dst.off += dputs(dst, STRV("("));
		list_foreach(&estx->nodes, terms, data)
		{
			dst.off += estx_term_print(estx, data, dst);
		}
		dst.off += dputs(dst, STRV(")"));
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
	const estx_node_data_t *node;
	estx_node_foreach_all(&estx->nodes, i, node)
	{
		if (node->type != ESTX_RULE) {
			continue;
		}

		strv_t name = STRVN(buf_get(&estx->strs, node->val.name.off), node->val.name.len);
		dst.off += dputf(dst, "%.*s = ", name.len, name.data);

		uint term = i;
		list_foreach(&estx->nodes, term, node)
		{
			dst.off += estx_term_print(estx, node, dst);
		}
		dst.off += dputs(dst, STRV("\n"));
	}

	return dst.off - off;
}

static size_t print_header(const estx_t *estx, estx_node_t *stack, int *state, int top, dst_t dst)
{
	(void)state;
	size_t off = dst.off;

	for (int i = 0; i < top - 1; i++) {
		dst.off += dputs(dst, list_get_next(&estx->nodes, stack[i], NULL) ? STRV("│ ") : STRV("  "));
	}

	if (top > 0) {
		dst.off += dputs(dst, list_get_next(&estx->nodes, stack[top - 1], NULL) ? STRV("├─") : STRV("└─"));
	}

	return dst.off - off;
}

static size_t estx_node_print_tree(const estx_t *estx, estx_node_t rule, dst_t dst)
{
	size_t off = dst.off;

	estx_node_t stack[64] = {0};
	int state[64]	      = {0};
	stack[0]	      = rule;
	int top		      = 1;

	while (top > 0) {
		estx_node_data_t *term = estx_get_node(estx, stack[top - 1]);
		if (term == NULL) {
			top--;
			continue;
		}

		strv_t title = STRV_NULL;
		int occ	     = 0;

		switch (term->type) {
		case ESTX_RULE: {
			strv_t name = STRVN(buf_get(&estx->strs, term->val.name.off), term->val.name.len);
			dst.off += dputf(dst, "<%.*s>\n", name.len, name.data);
			if (list_get_next(&estx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
				top--;
			}
			break;
		}
		case ESTX_TERM_RULE: {
			estx_node_data_t *node = estx_get_node(estx, term->val.rule);
			if (node == NULL) {
				top--;
				break;
			}
			strv_t name = STRVN(buf_get(&estx->strs, node->val.name.off), node->val.name.len);
			dst.off += print_header(estx, stack, state, top, dst);
			dst.off += dputf(dst, "<%.*s>", name.len, name.data);
			dst.off += estx_term_occ_print(term->occ, dst);
			dst.off += dputs(dst, STRV("\n"));
			if (list_get_next(&estx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
				top--;
			}
			break;
		}
		case ESTX_TERM_TOK: {
			dst.off += print_header(estx, stack, state, top, dst);
			dst.off += tok_type_print(1 << term->val.tok, dst);
			dst.off += estx_term_occ_print(term->occ, dst);
			dst.off += dputs(dst, STRV("\n"));
			if (list_get_next(&estx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
				top--;
			}
			break;
		}
		case ESTX_TERM_LIT: {
			dst.off += print_header(estx, stack, state, top, dst);
			strv_t lit = estx_data_lit(estx, term);
			if (strv_eq(lit, STRV("'"))) {
				dst.off += dputf(dst, "\"%.*s\"", lit.len, lit.data);
			} else {
				dst.off += dputf(dst, "\'%.*s\'", lit.len, lit.data);
			}
			dst.off += estx_term_occ_print(term->occ, dst);
			dst.off += dputs(dst, STRV("\n"));
			if (list_get_next(&estx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
				top--;
			}
			break;
		}
		case ESTX_TERM_GROUP: title = title.data ? title : STRV("group"); occ = 1; // fall-through
		case ESTX_TERM_ALT: title = title.data ? title : STRV("alt");		   // fall-through
		case ESTX_TERM_CON:
			title = title.data ? title : STRV("con");

			if (state[top - 1] == 0) {
				dst.off += print_header(estx, stack, state, top, dst);
				dst.off += dputs(dst, title);
				if (occ) {
					dst.off += estx_term_occ_print(term->occ, dst);
				}
				dst.off += dputs(dst, STRV("\n"));
				state[top - 1] = 1;
				stack[top++]   = term->val.terms;
			} else {
				state[top - 1] = 0;
				if (list_get_next(&estx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
					top--;
				}
			}
			break;
		default:
			log_warn("cparse", "estx", NULL, "unknown term type: %d", term->type);
			top--;
			break;
		}
	}

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
	const estx_node_data_t *node;
	estx_node_foreach_all(&estx->nodes, i, node)
	{
		if (node->type != ESTX_RULE) {
			continue;
		}

		if (!first) {
			dst.off += dputs(dst, STRV("\n"));
		}
		dst.off += estx_node_print_tree(estx, i, dst);
		first = 0;
	}

	return dst.off - off;
}
