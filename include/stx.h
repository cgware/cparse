#ifndef STX_H
#define STX_H

#include "buf.h"
#include "list.h"
#include "loc.h"
#include "tok.h"

typedef lnode_t stx_node_t;

typedef enum stx_node_type_e {
	STX_UNKNOWN,
	STX_RULE,
	STX_TERM_RULE,
	STX_TERM_TOK,
	STX_TERM_LIT,
	STX_TERM_OR,
} stx_node_type_t;

typedef struct stx_node_data_s {
	stx_node_type_t type;
	union {
		loc_t name;
		stx_node_t rule;
		tok_type_t tok;
		loc_t lit;
		struct {
			stx_node_t l;
			stx_node_t r;
		} orv;
	} val;
} stx_node_data_t;

typedef struct stx_s {
	list_t nodes;
	buf_t strs;
} stx_t;

stx_t *stx_init(stx_t *stx, uint nodes_cap, alloc_t alloc);
void stx_free(stx_t *stx);

int stx_rule(stx_t *stx, strv_t name, stx_node_t *rule);
int stx_term_rule(stx_t *stx, stx_node_t rule, stx_node_t *term);
int stx_term_tok(stx_t *stx, tok_type_t tok, stx_node_t *term);
int stx_term_lit(stx_t *stx, strv_t str, stx_node_t *term);
int stx_term_or(stx_t *stx, stx_node_t l, stx_node_t r, stx_node_t *term);

int stx_find_rule(stx_t *stx, strv_t name, stx_node_t *rule);

stx_node_data_t *stx_get_node(const stx_t *stx, stx_node_t node);

strv_t stx_data_lit(const stx_t *stx, const stx_node_data_t *data);

int stx_add_term(stx_t *stx, stx_node_t node, stx_node_t term);

int stx_rule_add_or(stx_t *stx, stx_node_t rule, size_t n, ...);
int stx_rule_add_arr(stx_t *stx, stx_node_t rule, stx_node_t term);
int stx_rule_add_arr_sep(stx_t *stx, stx_node_t rule, stx_node_t term, stx_node_t sep);

size_t stx_print(const stx_t *stx, dst_t dst);
size_t stx_print_tree(const stx_t *stx, dst_t dst);

#define stx_node_foreach     list_foreach
#define stx_node_foreach_all list_foreach_all

#endif
