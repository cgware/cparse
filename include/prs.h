#ifndef PRS_H
#define PRS_H

#include "lex.h"
#include "stx.h"
#include "tree.h"

#define PRS_NODE_END ((uint)-1)

typedef tree_node_t prs_node_t;

typedef struct prs_s {
	const lex_t *lex;
	const stx_t *stx;
	tree_t nodes;
} prs_t;

prs_t *prs_init(prs_t *prs, uint nodes_cap, alloc_t alloc);
void prs_free(prs_t *prs);

int prs_node_rule(prs_t *prs, stx_node_t rule, prs_node_t *node);
int prs_node_tok(prs_t *prs, tok_t tok, prs_node_t *node);
int prs_node_lit(prs_t *prs, size_t start, uint len, prs_node_t *node);

int prs_add_node(prs_t *prs, prs_node_t parent, prs_node_t node);
int prs_remove_node(prs_t *prs, prs_node_t node);

int prs_get_rule(const prs_t *prs, prs_node_t parent, stx_node_t rule, prs_node_t *node);
int prs_get_str(const prs_t *prs, prs_node_t parent, tok_t *out);

int prs_parse(prs_t *prs, const lex_t *lex, const stx_t *stx, stx_node_t rule, prs_node_t *root, dst_t dst);

size_t prs_print(const prs_t *prs, prs_node_t node, dst_t dst);

#endif
