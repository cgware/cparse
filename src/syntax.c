#include "syntax.h"

#include "log.h"

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

int stx_add_rule(stx_t *stx, stx_rule_t *rule)
{
	if (stx == NULL) {
		return 1;
	}

	stx_rule_t cnt;
	stx_rule_data_t *data = arr_add(&stx->rules, &cnt);
	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to add rule");
		return 1;
	}

	*data = (stx_rule_data_t){0};

	if (rule) {
		*rule = cnt;
	}

	return 0;
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

int stx_term_rule(stx_t *stx, stx_rule_t rule, stx_term_t *term)
{
	if (stx == NULL) {
		return 1;
	}

	stx_term_data_t *data = list_add(&stx->terms, term);
	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to create rule term");
		return 1;
	}

	*data = (stx_term_data_t){
		.type	  = STX_TERM_RULE,
		.val.rule = rule,
	};

	return 0;
}

int stx_term_tok(stx_t *stx, token_type_t token, stx_term_t *term)
{
	if (stx == NULL) {
		return 1;
	}

	stx_term_data_t *data = list_add(&stx->terms, term);
	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to create token term");
		return 1;
	}

	*data = (stx_term_data_t){
		.type	   = STX_TERM_TOKEN,
		.val.token = token,
	};

	return 0;
}

int stx_term_lit(stx_t *stx, strv_t str, stx_term_t *term)
{
	if (stx == NULL) {
		return 1;
	}

	stx_term_data_t *data = list_add(&stx->terms, term);
	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to create literal term");
		return 1;
	}

	size_t start;
	if (buf_add(&stx->strs, str.data, str.len, &start)) {
		return 1;
	}

	*data = (stx_term_data_t){
		.type	     = STX_TERM_LITERAL,
		.val.literal = {.start = start, .len = (uint)str.len},
	};

	return 0;
}

int stx_term_or(stx_t *stx, stx_term_t l, stx_term_t r, stx_term_t *term)
{
	if (stx == NULL) {
		return 1;
	}

	stx_term_data_t *data = list_add(&stx->terms, term);
	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to create or term");
		return 1;
	}

	*data = (stx_term_data_t){
		.type	 = STX_TERM_OR,
		.val.orv = {.l = l, .r = r},
	};

	return 0;
}

stx_term_data_t *stx_get_term_data(const stx_t *stx, stx_term_t term)
{
	if (stx == NULL) {
		return NULL;
	}

	stx_term_data_t *data = list_get(&stx->terms, term);
	if (data == NULL) {
		log_warn("cparse", "syntax", NULL, "invalid term: %d", term);
		return NULL;
	}

	return data;
}

int stx_rule_add_term(stx_t *stx, stx_rule_t rule, stx_term_t term)
{
	stx_rule_data_t *data = stx_get_rule_data(stx, rule);
	if (data == NULL) {
		log_error("cparse", "syntax", NULL, "failed to get rule: %d", rule);
		return 1;
	}

	if (data->has_terms) {
		list_set_next(&stx->terms, data->terms, term);
	} else {
		data->terms	= term;
		data->has_terms = 1;
	}

	return 0;
}

int stx_term_add_term(stx_t *stx, stx_term_t term, stx_term_t next)
{
	if (stx == NULL) {
		return 1;
	}

	return list_set_next(&stx->terms, term, next);
}

static int stx_rule_add_orv(stx_t *stx, stx_rule_t rule, size_t n, va_list args, stx_term_t *term)
{
	if (n < 2) {
		*term = va_arg(args, stx_term_t);
		return 0;
	}

	stx_term_t l = va_arg(args, stx_term_t);
	stx_term_t r;
	stx_rule_add_orv(stx, rule, n - 1, args, &r);
	return stx_term_or(stx, l, r, term);
}

int stx_rule_add_or(stx_t *stx, stx_rule_t rule, size_t n, ...)
{
	if (n < 1) {
		return 1;
	}

	va_list args;
	va_start(args, n);

	stx_term_t term;
	if (n < 2) {
		term = va_arg(args, stx_term_t);
	} else {
		stx_term_t l = va_arg(args, stx_term_t);
		stx_term_t r;
		stx_rule_add_orv(stx, rule, n - 1, args, &r);
		stx_term_or(stx, l, r, &term);
	}

	va_end(args);

	return stx_rule_add_term(stx, rule, term);
}

int stx_rule_add_arr(stx_t *stx, stx_rule_t rule, stx_term_t term)
{
	stx_term_data_t *data = stx_get_term_data(stx, term);
	if (data == NULL) {
		return 1;
	}

	stx_term_t l;
	stx_term_data_t *copy = list_add(&stx->terms, &l);
	if (copy == NULL) {
		log_error("cparse", "syntax", NULL, "failed to copy term");
		return 1;
	}

	*copy = *data;

	stx_term_t tmp;
	stx_term_rule(stx, rule, &tmp);
	stx_term_add_term(stx, l, tmp);
	stx_term_or(stx, l, term, &tmp);
	return stx_rule_add_term(stx, rule, tmp);
}

int stx_rule_add_arr_sep(stx_t *stx, stx_rule_t rule, stx_term_t term, stx_term_t sep)
{
	stx_term_data_t *data = stx_get_term_data(stx, term);
	if (data == NULL) {
		return 1;
	}

	stx_term_t l;
	stx_term_data_t *copy = list_add(&stx->terms, &l);
	if (copy == NULL) {
		log_error("cparse", "syntax", NULL, "failed to copy term");
		return 1;
	}

	*copy = *data;

	stx_term_add_term(stx, l, sep);

	stx_term_t tmp;
	stx_term_rule(stx, rule, &tmp);
	stx_term_add_term(stx, l, tmp);
	stx_term_or(stx, l, term, &tmp);
	return stx_rule_add_term(stx, rule, tmp);
}

static size_t stx_terms_print(const stx_t *stx, stx_term_t terms, dst_t dst)
{
	size_t off = dst.off;

	const stx_term_data_t *term;
	list_foreach(&stx->terms, terms, term)
	{
		switch (term->type) {
		case STX_TERM_RULE: {
			dst.off += dputf(dst, " <%d>", term->val.rule);
			break;
		}
		case STX_TERM_TOKEN: {
			dst.off += dputs(dst, STRV(" "));
			dst.off += token_type_print(1 << term->val.token, dst);
			break;
		}
		case STX_TERM_LITERAL: {
			strv_t literal = STRVN((char *)&stx->strs.data[term->val.literal.start], term->val.literal.len);
			if (strv_eq(literal, STRV("'"))) {
				dst.off += dputf(dst, " \"%.*s\"", literal.len, literal.data);
			} else {
				dst.off += dputf(dst, " \'%.*s\'", literal.len, literal.data);
			}
			break;
		}
		case STX_TERM_OR:
			dst.off += stx_terms_print(stx, term->val.orv.l, dst);
			dst.off += dputs(dst, STRV(" |"));
			dst.off += stx_terms_print(stx, term->val.orv.r, dst);
			break;
		default: log_warn("cparse", "syntax", NULL, "unknown term type: %d", term->type); break;
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

	const stx_rule_data_t *rule;
	uint i = 0;
	arr_foreach(&stx->rules, i, rule)
	{
		dst.off += dputf(dst, "<%d> ::=", i);
		dst.off += stx_terms_print(stx, rule->terms, dst);
		dst.off += dputs(dst, STRV("\n"));
	}

	return dst.off - off;
}

static size_t print_header(const stx_t *stx, stx_term_t *stack, int *state, int top, dst_t dst)
{
	size_t off = dst.off;
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
	if (top > 1 && (parent = stx_get_term_data(stx, stack[top - 2]))->type == STX_TERM_OR) {
		if (stack[top - 1] == parent->val.orv.l || stack[top - 1] == parent->val.orv.r) { // if left or right branch row
			// ── if last, ┬─ otherwise
			return dst.off + dputs(dst, list_get_next(&stx->terms, stack[top - 1], NULL) ? STRV("┬─") : STRV("──")) - off;
		}
	}

	// └─ if last, ├─ otherwise
	return dst.off + dputs(dst, list_get_next(&stx->terms, stack[top - 1], NULL) ? STRV("├─") : STRV("└─")) - off;
}

static size_t stx_rule_print_tree(const stx_t *stx, stx_rule_data_t *rule, uint rule_index, dst_t dst)
{
	size_t off = dst.off;

	dst.off += dputf(dst, "<%d>\n", rule_index);

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
			dst.off += dputf(dst, "<%d>\n", term->val.rule);
			list_get_next(&stx->terms, stack[top - 1], &stack[top - 1]);
			break;
		}
		case STX_TERM_TOKEN: {
			dst.off += print_header(stx, stack, state, top, dst);
			dst.off += token_type_print(1 << term->val.token, dst);
			dst.off += dputs(dst, STRV("\n"));
			list_get_next(&stx->terms, stack[top - 1], &stack[top - 1]);
			break;
		}
		case STX_TERM_LITERAL: {
			dst.off += print_header(stx, stack, state, top, dst);
			strv_t literal = STRVN((char *)&stx->strs.data[term->val.literal.start], term->val.literal.len);
			if (strv_eq(literal, STRV("'"))) {
				dst.off += dputf(dst, "\"%.*s\"", literal.len, literal.data);
			} else {
				dst.off += dputf(dst, "\'%.*s\'", literal.len, literal.data);
			}
			dst.off += dputs(dst, STRV("\n"));
			list_get_next(&stx->terms, stack[top - 1], &stack[top - 1]);
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

size_t stx_print_tree(const stx_t *stx, dst_t dst)
{
	if (stx == NULL) {
		return 0;
	}

	size_t off = dst.off;

	stx_rule_data_t *data;
	int first = 1;
	uint i	  = 0;
	arr_foreach(&stx->rules, i, data)
	{
		if (!first) {
			dst.off += dputs(dst, STRV("\n"));
		}
		dst.off += stx_rule_print_tree(stx, data, i, dst);
		first = 0;
	}

	return dst.off - off;
}
