#include "syntax.h"

#include "log.h"

#include <stdarg.h>

stx_t *stx_init(stx_t *stx, uint rules_cap, uint terms_cap, alloc_t alloc)
{
	if (stx == NULL) {
		return NULL;
	}

	if (arr_init(&stx->rules, rules_cap, sizeof(stx_rule_data_t), alloc) == NULL) {
		log_error("cparse", "syntax", NULL, "failed to initialize rules array");
		return NULL;
	}

	if (list_init(&stx->terms, terms_cap, sizeof(stx_term_data_t), alloc) == NULL) {
		log_error("cparse", "syntax", NULL, "failed to initialize terms list");
		return NULL;
	}

	if (buf_init(&stx->strs, 16, alloc) == NULL) {
		log_error("cparse", "syntax", NULL, "failed to initialize strings buffer");
		return NULL;
	}

	return stx;
}

void stx_free(stx_t *stx)
{
	if (stx == NULL) {
		return;
	}

	arr_free(&stx->rules);
	list_free(&stx->terms);
	buf_free(&stx->strs);
}

stx_rule_t stx_add_rule(stx_t *stx)
{
	if (stx == NULL) {
		return STX_RULE_END;
	}

	stx_rule_t rule	      = stx->rules.cnt;
	stx_rule_data_t *data = arr_add(&stx->rules);
	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to add rule");
		return STX_RULE_END;
	}

	*data = (stx_rule_data_t){
		.terms = STX_TERM_END,
	};

	return rule;
}

stx_rule_data_t *stx_get_rule_data(const stx_t *stx, stx_rule_t rule)
{
	if (stx == NULL) {
		return NULL;
	}

	stx_rule_data_t *data = arr_get(&stx->rules, rule);

	if (data == NULL) {
		log_warn("cparse", "syntax", NULL, "invalid rule: %d", rule);
		return NULL;
	}

	return data;
}

stx_term_t stx_create_term(stx_t *stx, stx_term_data_t term)
{
	if (stx == NULL) {
		return STX_TERM_END;
	}

	const stx_term_t child = list_add(&stx->terms);
	stx_term_data_t *data  = list_get_data(&stx->terms, child);

	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to create term");
		return STX_TERM_END;
	}

	*data = term;

	return child;
}

stx_term_data_t *stx_get_term_data(const stx_t *stx, stx_term_t term)
{
	if (stx == NULL) {
		return NULL;
	}

	stx_term_data_t *data = list_get_data(&stx->terms, term);

	if (data == NULL) {
		log_warn("cparse", "syntax", NULL, "invalid term: %d", term);
		return NULL;
	}

	return data;
}

stx_term_data_t stx_create_literal(stx_t *stx, strv_t str)
{
	if (stx == NULL) {
		return (stx_term_data_t){0};
	}

	size_t start = stx->strs.used;
	buf_add(&stx->strs, str.data, str.len, NULL);

	return (stx_term_data_t){.type = STX_TERM_LITERAL, .val.literal = {.start = start, .len = (uint)str.len}};
}

stx_term_t stx_rule_set_term(stx_t *stx, stx_rule_t rule, stx_term_t term)
{
	stx_rule_data_t *data = stx_get_rule_data(stx, rule);

	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to get rule: %d", rule);
		return STX_TERM_END;
	}

	return data->terms = term;
}

stx_term_t stx_rule_add_term(stx_t *stx, stx_rule_t rule, stx_term_t term)
{
	stx_rule_data_t *data = stx_get_rule_data(stx, rule);

	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to get rule: %d", rule);
		return STX_TERM_END;
	}

	return list_set_next_node(&stx->terms, data->terms, term);
}

stx_term_t stx_term_add_term(stx_t *stx, stx_term_t term, stx_term_t next)
{
	if (stx_get_term_data(stx, term) == NULL) {
		log_error("cparse", "syntax", NULL, "failed to get term: %d", term);
		return STX_TERM_END;
	}

	return list_set_next_node(&stx->terms, term, next);
}

static stx_term_t stx_rule_add_orv(stx_t *stx, stx_rule_t rule, size_t n, va_list args)
{
	if (n < 2) {
		return va_arg(args, stx_term_t);
	}

	return STX_TERM_OR(stx, va_arg(args, stx_term_t), stx_rule_add_orv(stx, rule, n - 1, args));
}

stx_term_t stx_rule_add_or(stx_t *stx, stx_rule_t rule, size_t n, ...)
{
	if (n < 1) {
		return STX_TERM_END;
	}

	va_list args;
	va_start(args, n);

	const stx_term_t term =
		n < 2 ? va_arg(args, stx_term_t) : STX_TERM_OR(stx, va_arg(args, stx_term_t), stx_rule_add_orv(stx, rule, n - 1, args));

	va_end(args);

	return stx_rule_add_term(stx, rule, term);
}

stx_term_t stx_rule_add_arr(stx_t *stx, stx_rule_t rule, stx_term_t term, stx_term_t sep)
{
	stx_term_data_t *data = stx_get_term_data(stx, term);
	if (data == NULL) {
		return STX_TERM_END;
	}

	stx_term_t l = stx_create_term(stx, *data);
	if (sep < STX_TERM_END) {
		stx_term_add_term(stx, l, sep);
	}
	stx_term_add_term(stx, l, STX_TERM_RULE(stx, rule));
	return stx_rule_add_term(stx, rule, STX_TERM_OR(stx, l, term));
}

static int stx_terms_print(const stx_t *stx, const stx_term_t terms, print_dst_t dst)
{
	int off = dst.off;

	const stx_term_data_t *term;
	list_foreach(&stx->terms, terms, term)
	{
		switch (term->type) {
		case STX_TERM_RULE: {
			dst.off += c_dprintf(dst, " <%d>", term->val.rule);
			break;
		}
		case STX_TERM_TOKEN: {
			dst.off += c_dprintf(dst, " ");
			dst.off += token_type_print(1 << term->val.token, dst);
			break;
		}
		case STX_TERM_LITERAL: {
			strv_t literal = STRVN((char *)&stx->strs.data[term->val.literal.start], term->val.literal.len);
			if (strv_eq(literal, STRV("'"))) {
				dst.off += c_dprintf(dst, " \"%.*s\"", literal.len, literal.data);
			} else {
				dst.off += c_dprintf(dst, " \'%.*s\'", literal.len, literal.data);
			}
			break;
		}
		case STX_TERM_OR:
			dst.off += stx_terms_print(stx, term->val.orv.l, dst);
			dst.off += c_dprintf(dst, " |");
			dst.off += stx_terms_print(stx, term->val.orv.r, dst);
			break;
		default: log_warn("cparse", "syntax", NULL, "unknown term type: %d", term->type); break;
		}
	}

	return dst.off - off;
}

int stx_print(const stx_t *stx, print_dst_t dst)
{
	if (stx == NULL) {
		return 0;
	}

	int off = dst.off;

	const stx_rule_data_t *rule;
	arr_foreach(&stx->rules, rule)
	{
		dst.off += c_dprintf(dst, "<%d> ::=", _i);
		dst.off += stx_terms_print(stx, rule->terms, dst);
		dst.off += c_dprintf(dst, "\n");
	}

	return dst.off - off;
}

static int print_header(const stx_t *stx, stx_term_t *stack, int *state, int top, print_dst_t dst)
{
	int off = dst.off;
	stx_term_data_t *term;
	stx_term_data_t *parent;

	int last_or = -1;
	if (top > 1 && state[top - 2] == 1) {
		for (int i = 0; i < top - 2; i++) {
			// if 'or' column
			if ((term = stx_get_term_data(stx, stack[i]))->type == STX_TERM_OR) {
				last_or = i;
			}
		}
	}

	for (int i = 0; i < top - 1; i++) {
		// if 'or' column
		if ((term = stx_get_term_data(stx, stack[i]))->type == STX_TERM_OR) {
			const char *str = "  ";
			if (stack[top - 1] == term->val.orv.l) { // if left branch start row
				str = "or";
			} else if (stack[top - 1] == term->val.orv.r) { // if right branch start row
				str = "└─";
			} else if (state[i] == 1) { // if left branch row
				str = "│ ";
			} else if (last_or == i) {
				str = "└─";
			}

			dst.off += c_dprintf(dst, str);
		}
	}

	// if 'or' row
	if (top > 1 && (parent = stx_get_term_data(stx, stack[top - 2]))->type == STX_TERM_OR) {
		if (stack[top - 1] == parent->val.orv.l || stack[top - 1] == parent->val.orv.r) { // if left or right branch row
			// ── if last, ┬─ otherwise
			return dst.off + c_dprintf(dst, list_get_next(&stx->terms, stack[top - 1]) < stx->terms.cnt ? "┬─" : "──") - off;
		}
	}

	// └─ if last, ├─ otherwise
	return dst.off + c_dprintf(dst, list_get_next(&stx->terms, stack[top - 1]) < stx->terms.cnt ? "├─" : "└─") - off;
}

static int stx_rule_print_tree(const stx_t *stx, stx_rule_data_t *rule, uint rule_index, print_dst_t dst)
{
	int off = dst.off;

	dst.off += c_dprintf(dst, "<%d>\n", rule_index);

	stx_term_t stack[64] = {0};
	int state[64]	     = {0};
	stack[0]	     = rule->terms;
	int top		     = 1;

	while (top > 0) {
		if (stack[top - 1] >= stx->terms.cnt) {
			top--;
			if (top <= 0) {
				break;
			}
		}

		stx_term_data_t *term = stx_get_term_data(stx, stack[top - 1]);

		switch (term->type) {
		case STX_TERM_RULE: {
			dst.off += print_header(stx, stack, state, top, dst);
			dst.off += c_dprintf(dst, "<%d>\n", term->val.rule);
			stack[top - 1] = list_get_next(&stx->terms, stack[top - 1]);
			break;
		}
		case STX_TERM_TOKEN: {
			dst.off += print_header(stx, stack, state, top, dst);
			dst.off += token_type_print(1 << term->val.token, dst);
			dst.off += c_dprintf(dst, "\n");
			stack[top - 1] = list_get_next(&stx->terms, stack[top - 1]);
			break;
		}
		case STX_TERM_LITERAL: {
			dst.off += print_header(stx, stack, state, top, dst);
			strv_t literal = STRVN((char *)&stx->strs.data[term->val.literal.start], term->val.literal.len);
			if (strv_eq(literal, STRV("'"))) {
				dst.off += c_dprintf(dst, "\"%.*s\"", literal.len, literal.data);
			} else {
				dst.off += c_dprintf(dst, "\'%.*s\'", literal.len, literal.data);
			}
			dst.off += c_dprintf(dst, "\n");
			stack[top - 1] = list_get_next(&stx->terms, stack[top - 1]);
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
			log_warn("cparse", "syntax", NULL, "unknown term type: %d", term->type);
			top--;
			break;
		}
	}

	return dst.off - off;
}

int stx_print_tree(const stx_t *stx, print_dst_t dst)
{
	if (stx == NULL) {
		return 0;
	}

	int off = dst.off;

	stx_rule_data_t *data;
	int first = 1;
	arr_foreach(&stx->rules, data)
	{
		if (!first) {
			dst.off += c_dprintf(dst, "\n");
		}
		dst.off += stx_rule_print_tree(stx, data, _i, dst);
		first = 0;
	}

	return dst.off - off;
}
