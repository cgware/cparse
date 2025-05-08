#ifndef EPARSER_H
#define EPARSER_H

#include "estx.h"
#include "lex.h"
#include "tree.h"

typedef tnode_t eprs_node_t;

typedef struct eprs_s {
	const estx_t *estx;
	const lex_t *lex;
	tree_t nodes;
} eprs_t;

eprs_t *eprs_init(eprs_t *eprs, uint nodes_cap, alloc_t alloc);
void eprs_free(eprs_t *eprs);

int eprs_node_rule(eprs_t *eprs, estx_rule_t rule, eprs_node_t *node);
int eprs_node_tok(eprs_t *eprs, tok_t tok, eprs_node_t *node);
int eprs_node_lit(eprs_t *eprs, size_t start, uint len, eprs_node_t *node);

int eprs_add_node(eprs_t *eprs, eprs_node_t parent, eprs_node_t node);
int eprs_remove_node(eprs_t *eprs, eprs_node_t node);

int eprs_get_rule(const eprs_t *eprs, eprs_node_t parent, estx_rule_t rule, eprs_node_t *node);
int eprs_get_str(const eprs_t *eprs, eprs_node_t parent, tok_t *out);

int eprs_parse(eprs_t *eprs, const lex_t *lex, const estx_t *estx, estx_rule_t rule, eprs_node_t *root, dst_t dst);

size_t eprs_print(const eprs_t *eprs, eprs_node_t node, dst_t dst);

#define eprs_node_foreach tree_foreach_child

#endif
