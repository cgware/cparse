#ifndef SYNTAX_H
#define SYNTAX_H

#include "arr.h"
#include "list.h"
#include "print.h"
#include "str.h"
#include "token.h"

#define STX_RULE_END ARR_END
#define STX_TERM_END LIST_END

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
		str_t literal;
		struct {
			stx_term_t l;
			stx_term_t r;
		} orv;
	} val;
} stx_term_data_t;

typedef struct stx_rule_data_s {
	stx_term_t terms;
} stx_rule_data_t;

// TODO: Use arr for terms with start and cnt
typedef struct stx_s {
	arr_t rules;
	list_t terms;
} stx_t;

stx_t *stx_init(stx_t *stx, uint rules_cap, uint terms_cap, alloc_t alloc);
void stx_free(stx_t *stx);

stx_rule_t stx_add_rule(stx_t *stx);
stx_rule_data_t *stx_get_rule_data(const stx_t *stx, stx_rule_t rule);

stx_term_t stx_create_term(stx_t *stx, stx_term_data_t term);
stx_term_data_t *stx_get_term_data(const stx_t *stx, stx_term_t term);

stx_term_t stx_rule_set_term(stx_t *stx, stx_rule_t rule, stx_term_t term);
stx_term_t stx_rule_add_term(stx_t *stx, stx_rule_t rule, stx_term_t term);
stx_term_t stx_term_add_term(stx_t *stx, stx_term_t term, stx_term_t next);

stx_term_t stx_rule_add_or(stx_t *stx, stx_rule_t rule, size_t n, ...);
stx_term_t stx_rule_add_arr(stx_t *stx, stx_rule_t rule, stx_term_t term, stx_term_t sep);

int stx_print(const stx_t *stx, print_dst_t dst);
int stx_print_tree(const stx_t *stx, print_dst_t dst);

#define STX_TERM_NONE(_stx)		 STX_TERM_END
#define STX_TERM_RULE(_stx, _rule)	 stx_create_term(_stx, (stx_term_data_t){.type = STX_TERM_RULE, .val.rule = _rule})
#define STX_TERM_TOKEN(_stx, _token)	 stx_create_term(_stx, (stx_term_data_t){.type = STX_TERM_TOKEN, .val.token = _token})
#define STX_TERM_LITERAL(_stx, _literal) stx_create_term(_stx, (stx_term_data_t){.type = STX_TERM_LITERAL, .val.literal = _literal})
#define STX_TERM_OR(_stx, _l, _r)	 stx_create_term(_stx, (stx_term_data_t){.type = STX_TERM_OR, .val.orv = {.l = _l, .r = _r}})

#define stx_rule_foreach arr_foreach
#define stx_term_foreach list_foreach

#endif
