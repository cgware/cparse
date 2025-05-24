#include "stx.h"

#include "log.h"

stx_t *stx_init(stx_t *stx, uint nodes_cap, alloc_t alloc)
{
	if (stx == NULL) {
		return NULL;
	}

	if (list_init(&stx->nodes, nodes_cap, sizeof(stx_node_data_t), alloc) == NULL ||
	    strvbuf_init(&stx->strs, nodes_cap, 8, alloc) == NULL) {
		log_error("cparse", "stx", NULL, "failed to initialize nodes");
		return NULL;
	}

	return stx;
}

void stx_free(stx_t *stx)
{
	if (stx == NULL) {
		return;
	}

	list_free(&stx->nodes);
	strvbuf_free(&stx->strs);
}

int stx_rule(stx_t *stx, strv_t name, stx_node_t *rule)
{
	if (stx == NULL) {
		return 1;
	}

	size_t rule_name;
	size_t used = stx->strs.used;
	if (strvbuf_add(&stx->strs, name, &rule_name)) {
		log_error("cparse", "stx", NULL, "failed to add rule name");
		return 1;
	}

	stx_node_data_t *data = list_node(&stx->nodes, rule);
	if (data == NULL) {
		buf_reset(&stx->strs, used);
		log_error("cparse", "stx", NULL, "failed to add rule");
		return 1;
	}

	*data = (stx_node_data_t){
		.type	  = STX_RULE,
		.val.name = rule_name,
	};

	return 0;
}

int stx_term_rule(stx_t *stx, stx_node_t rule, stx_node_t *term)
{
	if (stx == NULL) {
		return 1;
	}

	stx_node_data_t *data = stx_get_node(stx, rule);
	if (data == NULL || data->type != STX_RULE) {
		log_error("cparse", "stx", NULL, "invalid rule: %d", rule);
		return 1;
	}

	data = list_node(&stx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "stx", NULL, "failed to create rule term");
		return 1;
	}

	*data = (stx_node_data_t){
		.type	  = STX_TERM_RULE,
		.val.rule = rule,
	};

	return 0;
}

int stx_term_tok(stx_t *stx, tok_type_t tok, stx_node_t *term)
{
	if (stx == NULL) {
		return 1;
	}

	stx_node_data_t *data = list_node(&stx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "stx", NULL, "failed to create tok term");
		return 1;
	}

	*data = (stx_node_data_t){
		.type	 = STX_TERM_TOK,
		.val.tok = tok,
	};

	return 0;
}

int stx_term_lit(stx_t *stx, strv_t str, stx_node_t *term)
{
	if (stx == NULL) {
		return 1;
	}

	size_t lit;
	size_t used = stx->strs.used;
	if (strvbuf_add(&stx->strs, str, &lit)) {
		log_error("cparse", "stx", NULL, "failed to add literal string");
		return 1;
	}

	stx_node_data_t *data = list_node(&stx->nodes, term);
	if (data == NULL) {
		buf_reset(&stx->strs, used);
		log_error("cparse", "stx", NULL, "failed to create literal term");
		return 1;
	}

	*data = (stx_node_data_t){
		.type	 = STX_TERM_LIT,
		.val.lit = lit,
	};

	return 0;
}

int stx_term_or(stx_t *stx, stx_node_t l, stx_node_t r, stx_node_t *term)
{
	if (stx == NULL) {
		return 1;
	}

	if (stx_get_node(stx, l) == NULL) {
		log_error("cparse", "stx", NULL, "invalid left node: %d", l);
		return 1;
	}

	if (stx_get_node(stx, r) == NULL) {
		log_error("cparse", "stx", NULL, "invalid right node: %d", r);
		return 1;
	}

	stx_node_data_t *data = list_node(&stx->nodes, term);
	if (data == NULL) {
		log_error("cparse", "stx", NULL, "failed to create or term");
		return 1;
	}

	*data = (stx_node_data_t){
		.type	 = STX_TERM_OR,
		.val.orv = {.l = l, .r = r},
	};

	return 0;
}

int stx_find_rule(stx_t *stx, strv_t name, stx_node_t *rule)
{
	if (stx == NULL) {
		return 1;
	}

	uint i = 0;
	stx_node_data_t *node;
	stx_node_foreach_all(&stx->nodes, i, node)
	{
		if (node->type != STX_RULE) {
			continue;
		}

		if (strv_eq(strvbuf_get(&stx->strs, node->val.name), name)) {
			*rule = i;
			return 0;
		}
	}

	return 1;
}

stx_node_data_t *stx_get_node(const stx_t *stx, stx_node_t node)
{
	if (stx == NULL) {
		return NULL;
	}

	stx_node_data_t *data = list_get(&stx->nodes, node);
	if (data == NULL) {
		log_error("cparse", "stx", NULL, "invalid node: %d", node);
		return NULL;
	}

	return data;
}

strv_t stx_data_lit(const stx_t *stx, const stx_node_data_t *data)
{
	if (stx == NULL || data == NULL || data->type != STX_TERM_LIT) {
		return STRV_NULL;
	}

	return strvbuf_get(&stx->strs, data->val.lit);
}

int stx_add_term(stx_t *stx, stx_node_t node, stx_node_t term)
{
	if (stx == NULL) {
		return 1;
	}

	if (list_app(&stx->nodes, node, term)) {
		log_error("cparse", "stx", NULL, "failed to add term %d to node %d", term, node);
		return 1;
	}

	return 0;
}

static int stx_rule_add_orv(stx_t *stx, stx_node_t rule, size_t n, va_list args, stx_node_t *term)
{
	if (n < 2) {
		*term = va_arg(args, stx_node_t);
		return 0;
	}

	stx_node_t l = va_arg(args, stx_node_t);
	stx_node_t r;
	stx_rule_add_orv(stx, rule, n - 1, args, &r);
	return stx_term_or(stx, l, r, term);
}

int stx_rule_add_or(stx_t *stx, stx_node_t rule, size_t n, ...)
{
	if (n < 1) {
		return 1;
	}

	va_list args;
	va_start(args, n);

	stx_node_t term;
	if (n < 2) {
		term = va_arg(args, stx_node_t);
	} else {
		stx_node_t l = va_arg(args, stx_node_t);
		stx_node_t r;
		stx_rule_add_orv(stx, rule, n - 1, args, &r);
		stx_term_or(stx, l, r, &term);
	}

	va_end(args);

	return stx_add_term(stx, rule, term);
}

int stx_rule_add_arr(stx_t *stx, stx_node_t rule, stx_node_t term)
{
	stx_node_data_t *data = stx_get_node(stx, term);
	if (data == NULL) {
		return 1;
	}

	stx_node_t tmp;
	if (stx_term_rule(stx, rule, &tmp)) {
		return 1;
	}

	stx_node_t l;
	stx_node_data_t *copy = list_node(&stx->nodes, &l);
	if (copy == NULL) {
		log_error("cparse", "stx", NULL, "failed to copy term");
		return 1;
	}

	*copy = *data;

	list_app(&stx->nodes, l, tmp);
	stx_term_or(stx, l, term, &tmp);
	return stx_add_term(stx, rule, tmp);
}

int stx_rule_add_arr_sep(stx_t *stx, stx_node_t rule, stx_node_t term, stx_node_t sep)
{
	stx_node_data_t *data = stx_get_node(stx, term);
	if (data == NULL) {
		return 1;
	}

	stx_node_t tmp;
	if (stx_term_rule(stx, rule, &tmp)) {
		return 1;
	}

	stx_node_t l;
	stx_node_data_t *copy = list_node(&stx->nodes, &l);
	if (copy == NULL) {
		log_error("cparse", "stx", NULL, "failed to copy term");
		return 1;
	}

	*copy = *data;

	list_app(&stx->nodes, l, sep);
	list_app(&stx->nodes, l, tmp);
	stx_term_or(stx, l, term, &tmp);
	return stx_add_term(stx, rule, tmp);
}

static size_t stx_terms_print(const stx_t *stx, stx_node_t terms, dst_t dst)
{
	size_t off = dst.off;

	const stx_node_data_t *term;
	list_foreach(&stx->nodes, terms, term)
	{
		switch (term->type) {
		case STX_RULE: break;
		case STX_TERM_RULE: {
			stx_node_data_t *rule = stx_get_node(stx, term->val.rule);
			if (rule == NULL) {
				break;
			}
			strv_t name = strvbuf_get(&stx->strs, rule->val.name);
			dst.off += dputf(dst, " <%.*s>", name.len, name.data);
			break;
		}
		case STX_TERM_TOK: {
			dst.off += dputs(dst, STRV(" "));
			dst.off += tok_type_print(1 << term->val.tok, dst);
			break;
		}
		case STX_TERM_LIT: {
			strv_t lit = stx_data_lit(stx, term);
			if (strv_eq(lit, STRV("'"))) {
				dst.off += dputf(dst, " \"%.*s\"", lit.len, lit.data);
			} else {
				dst.off += dputf(dst, " \'%.*s\'", lit.len, lit.data);
			}
			break;
		}
		case STX_TERM_OR:
			dst.off += stx_terms_print(stx, term->val.orv.l, dst);
			dst.off += dputs(dst, STRV(" |"));
			dst.off += stx_terms_print(stx, term->val.orv.r, dst);
			break;
		default: log_warn("cparse", "stx", NULL, "unknown term type: %d", term->type); break;
		}
	}

	return dst.off - off;
}

size_t stx_print(const stx_t *stx, dst_t dst)
{
	if (stx == NULL) {
		return 0;
	}

	size_t off = dst.off;

	uint i = 0;
	const stx_node_data_t *node;
	stx_node_foreach_all(&stx->nodes, i, node)
	{
		if (node->type != STX_RULE) {
			continue;
		}

		strv_t name = strvbuf_get(&stx->strs, node->val.name);
		dst.off += dputf(dst, "<%.*s> ::=", name.len, name.data);
		dst.off += stx_terms_print(stx, i, dst);
		dst.off += dputs(dst, STRV("\n"));
	}

	return dst.off - off;
}

static size_t print_header(const stx_t *stx, stx_node_t *stack, int *state, int top, dst_t dst)
{
	size_t off = dst.off;
	stx_node_data_t *term;
	stx_node_data_t *parent;

	int last_or = -1;
	if (top > 1 && state[top - 2] == 1) {
		for (int i = 0; i < top - 2; i++) {
			// if 'or' column
			if ((term = stx_get_node(stx, stack[i]))->type == STX_TERM_OR) {
				last_or = i;
			}
		}
	}

	for (int i = 0; i < top - 1; i++) {
		// if 'or' column
		if ((term = stx_get_node(stx, stack[i]))->type == STX_TERM_OR) {
			strv_t str = STRV("  ");
			if (stack[top - 1] == term->val.orv.l) { // if left branch start row
				str = STRV("or");
			} else if (stack[top - 1] == term->val.orv.r) { // if right branch start row
				str = STRV("└─");
			} else if (state[i] == 1) { // if left branch row
				str = STRV("│ ");
			} else if (last_or == i) {
				str = STRV("└─");
			}

			dst.off += dputs(dst, str);
		}
	}

	// if 'or' row
	if (top > 1 && (parent = stx_get_node(stx, stack[top - 2]))->type == STX_TERM_OR) {
		if (stack[top - 1] == parent->val.orv.l || stack[top - 1] == parent->val.orv.r) { // if left or right branch row
			// ── if last, ┬─ otherwise
			return dst.off + dputs(dst, list_get_next(&stx->nodes, stack[top - 1], NULL) ? STRV("┬─") : STRV("──")) - off;
		}
	}

	// └─ if last, ├─ otherwise
	return dst.off + dputs(dst, list_get_next(&stx->nodes, stack[top - 1], NULL) ? STRV("├─") : STRV("└─")) - off;
}

static size_t stx_node_print_tree(const stx_t *stx, stx_node_t rule, dst_t dst)
{
	size_t off = dst.off;

	stx_node_t stack[64] = {0};
	int state[64]	     = {0};
	stack[0]	     = rule;
	int top		     = 1;

	while (top > 0) {
		stx_node_data_t *term = stx_get_node(stx, stack[top - 1]);

		switch (term->type) {
		case STX_RULE: {
			strv_t name = strvbuf_get(&stx->strs, term->val.name);
			dst.off += dputf(dst, "<%.*s>\n", name.len, name.data);
			if (list_get_next(&stx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
				top--;
			}
			break;
		}
		case STX_TERM_RULE: {
			stx_node_data_t *node = stx_get_node(stx, term->val.rule);
			if (node == NULL) {
				top--;
				break;
			}
			strv_t name = strvbuf_get(&stx->strs, node->val.name);
			dst.off += print_header(stx, stack, state, top, dst);
			dst.off += dputf(dst, "<%.*s>\n", name.len, name.data);
			if (list_get_next(&stx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
				top--;
			}
			break;
		}
		case STX_TERM_TOK: {
			dst.off += print_header(stx, stack, state, top, dst);
			dst.off += tok_type_print(1 << term->val.tok, dst);
			dst.off += dputs(dst, STRV("\n"));
			if (list_get_next(&stx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
				top--;
			}
			break;
		}
		case STX_TERM_LIT: {
			dst.off += print_header(stx, stack, state, top, dst);
			strv_t lit = stx_data_lit(stx, term);
			if (strv_eq(lit, STRV("'"))) {
				dst.off += dputf(dst, "\"%.*s\"", lit.len, lit.data);
			} else {
				dst.off += dputf(dst, "\'%.*s\'", lit.len, lit.data);
			}
			dst.off += dputs(dst, STRV("\n"));
			if (list_get_next(&stx->nodes, stack[top - 1], &stack[top - 1]) == NULL) {
				top--;
			}
			break;
		}
		case STX_TERM_OR:
			if (state[top - 1] == 0) {
				state[top - 1] = 1;
				stack[top++]   = term->val.orv.l;
			} else if (state[top - 1] == 1) {
				state[top - 1] = 2;
				stack[top++]   = term->val.orv.r;
			} else {
				state[top - 1] = 0;
				top--;
			}
			break;
		default:
			log_warn("cparse", "stx", NULL, "unknown term type: %d", term->type);
			top--;
			break;
		}
	}

	return dst.off - off;
}

size_t stx_print_tree(const stx_t *stx, dst_t dst)
{
	if (stx == NULL) {
		return 0;
	}

	size_t off = dst.off;

	int first = 1;
	uint i	  = 0;
	const stx_node_data_t *node;
	stx_node_foreach_all(&stx->nodes, i, node)
	{
		if (node->type != STX_RULE) {
			continue;
		}

		if (!first) {
			dst.off += dputs(dst, STRV("\n"));
		}
		dst.off += stx_node_print_tree(stx, i, dst);
		first = 0;
	}

	return dst.off - off;
}
