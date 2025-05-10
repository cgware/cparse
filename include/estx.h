#ifndef ESYNTAX_H
#define ESYNTAX_H

#include "buf.h"
#include "loc.h"
#include "tok.h"
#include "tree.h"

typedef uint estx_node_t;

typedef enum estx_node_type_e {
	ESTX_UNKNOWN,
	ESTX_RULE,
	ESTX_TERM_RULE,
	ESTX_TERM_TOK,
	ESTX_TERM_LIT,
	ESTX_TERM_ALT,
	ESTX_TERM_CON,
	ESTX_TERM_GROUP,
} estx_node_type_t;

typedef enum estx_node_occ_e {
	ESTX_TERM_OCC_ONE = 0,
	ESTX_TERM_OCC_OPT = 1 << 0,
	ESTX_TERM_OCC_REP = 1 << 1,
} estx_node_occ_t;

typedef struct estx_term_data_s {
	estx_node_type_t type;
	estx_node_occ_t occ;
	union {
		loc_t name;
		estx_node_t rule;
		tok_type_t tok;
		loc_t lit;
	} val;
} estx_node_data_t;

typedef struct estx_s {
	tree_t nodes;
	buf_t strs;
} estx_t;

estx_t *estx_init(estx_t *estx, uint nodes_cap, alloc_t alloc);
void estx_free(estx_t *estx);

int estx_rule(estx_t *estx, strv_t name, estx_node_t *rule);
int estx_term_rule(estx_t *estx, estx_node_t rule, estx_node_occ_t occ, estx_node_t *term);
int estx_term_tok(estx_t *estx, tok_type_t tok, estx_node_occ_t occ, estx_node_t *term);
int estx_term_lit(estx_t *estx, strv_t str, estx_node_occ_t occ, estx_node_t *term);
int estx_term_alt(estx_t *estx, estx_node_t *term);
int estx_term_con(estx_t *estx, estx_node_t *term);
int estx_term_group(estx_t *estx, estx_node_occ_t occ, estx_node_t *term);

int estx_find_rule(estx_t *estx, strv_t name, estx_node_t *rule);

estx_node_data_t *estx_get_node(const estx_t *estx, estx_node_t node);

strv_t estx_data_lit(const estx_t *estx, const estx_node_data_t *data);

int estx_add_term(estx_t *estx, estx_node_t term, estx_node_t child);

size_t estx_print(const estx_t *estx, dst_t dst);
size_t estx_print_tree(const estx_t *estx, dst_t dst);

#define estx_node_foreach     tree_foreach_child
#define estx_node_foreach_all tree_foreach_all

#endif
