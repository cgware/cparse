#ifndef ESYNTAX_H
#define ESYNTAX_H

#include "arr.h"
#include "buf.h"
#include "str.h"
#include "tok.h"
#include "tree.h"

typedef uint estx_rule_t;
typedef tnode_t estx_term_t;

typedef enum estx_term_type_e {
	ESTX_TERM_NONE,
	ESTX_TERM_RULE,
	ESTX_TERM_TOKEN,
	ESTX_TERM_LITERAL,
	ESTX_TERM_ALT,
	ESTX_TERM_CON,
	ESTX_TERM_GROUP,
} estx_term_type_t;

typedef enum estx_term_occ_e {
	ESTX_TERM_OCC_ONE = 0,
	ESTX_TERM_OCC_OPT = 1 << 0,
	ESTX_TERM_OCC_REP = 1 << 1,
} estx_term_occ_t;

typedef struct estx_term_data_s {
	estx_term_type_t type;
	estx_term_occ_t occ;
	union {
		estx_rule_t rule;
		tok_type_t tok;
		tok_t literal;
	} val;
} estx_term_data_t;

typedef struct estx_rule_data_s {
	estx_term_t terms;
} estx_rule_data_t;

typedef struct estx_s {
	arr_t rules;
	tree_t terms;
	buf_t strs;
} estx_t;

estx_t *estx_init(estx_t *estx, uint rules_cap, uint terms_cap, alloc_t alloc);
void estx_free(estx_t *estx);

int estx_add_rule(estx_t *estx, estx_rule_t *rule);
estx_rule_data_t *estx_get_rule_data(const estx_t *estx, estx_rule_t rule);

int estx_term_rule(estx_t *estx, estx_rule_t rule, estx_term_occ_t occ, estx_term_t *term);
int estx_term_tok(estx_t *estx, tok_type_t tok, estx_term_occ_t occ, estx_term_t *term);
int estx_term_lit(estx_t *estx, strv_t str, estx_term_occ_t occ, estx_term_t *term);
int estx_term_alt(estx_t *estx, estx_term_t *term);
int estx_term_con(estx_t *estx, estx_term_t *term);
int estx_term_group(estx_t *estx, estx_term_occ_t occ, estx_term_t *term);

estx_term_data_t *estx_get_term_data(const estx_t *estx, estx_term_t term);

int estx_rule_set_term(estx_t *estx, estx_rule_t rule, estx_term_t term);
int estx_term_add_term(estx_t *estx, estx_term_t term, estx_term_t child);

size_t estx_print(const estx_t *estx, dst_t dst);
size_t estx_print_tree(const estx_t *estx, dst_t dst);

#define estx_rule_foreach arr_foreach
#define estx_term_foreach tree_foreach_child

#endif
