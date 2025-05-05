#ifndef SYNTAX_H
#define SYNTAX_H

#include "arr.h"
#include "buf.h"
#include "list.h"
#include "strv.h"
#include "token.h"

typedef uint stx_rule_t;
typedef lnode_t stx_term_t;

typedef enum stx_term_type_e {
	STX_TERM_NONE,
	STX_TERM_RULE,
	STX_TERM_TOKEN,
	STX_TERM_LITERAL,
	STX_TERM_OR,
} stx_term_type_t;

typedef struct stx_term_data_s {
	stx_term_type_t type;
	union {
		stx_rule_t rule;
		token_type_t token;
		token_t literal;
		struct {
			stx_term_t l;
			stx_term_t r;
		} orv;
	} val;
} stx_term_data_t;

typedef struct stx_rule_data_s {
	stx_term_t terms;
	byte has_terms : 1;
} stx_rule_data_t;

// TODO: Use arr for terms with start and cnt
typedef struct stx_s {
	arr_t rules;
	list_t terms;
	buf_t strs;
} stx_t;

stx_t *stx_init(stx_t *stx, uint rules_cap, uint terms_cap, alloc_t alloc);
void stx_free(stx_t *stx);

int stx_add_rule(stx_t *stx, stx_rule_t *rule);
stx_rule_data_t *stx_get_rule_data(const stx_t *stx, stx_rule_t rule);

int stx_term_rule(stx_t *stx, stx_rule_t rule, stx_term_t *term);
int stx_term_tok(stx_t *stx, token_type_t token, stx_term_t *term);
int stx_term_lit(stx_t *stx, strv_t str, stx_term_t *term);
int stx_term_or(stx_t *stx, stx_term_t l, stx_term_t r, stx_term_t *term);

stx_term_data_t *stx_get_term_data(const stx_t *stx, stx_term_t term);

int stx_rule_add_term(stx_t *stx, stx_rule_t rule, stx_term_t term);
int stx_term_add_term(stx_t *stx, stx_term_t term, stx_term_t next);

int stx_rule_add_or(stx_t *stx, stx_rule_t rule, size_t n, ...);
int stx_rule_add_arr(stx_t *stx, stx_rule_t rule, stx_term_t term);
int stx_rule_add_arr_sep(stx_t *stx, stx_rule_t rule, stx_term_t term, stx_term_t sep);

size_t stx_print(const stx_t *stx, dst_t dst);
size_t stx_print_tree(const stx_t *stx, dst_t dst);

#define stx_rule_foreach arr_foreach
#define stx_term_foreach list_foreach

#endif
