#ifndef BNF_H
#define BNF_H

#include "prs.h"
#include "stx.h"

typedef struct bnf_s {
	stx_t stx;
	stx_node_t file;
	stx_node_t bnf;
	stx_node_t rules;
	stx_node_t rule;
	stx_node_t rname;
	stx_node_t expr;
	stx_node_t terms;
	stx_node_t term;
	stx_node_t literal;
	stx_node_t tok;
	stx_node_t tdouble;
	stx_node_t tsingle;
} bnf_t;

bnf_t *bnf_init(bnf_t *bnf, alloc_t alloc);
void bnf_free(bnf_t *bnf);

const stx_t *bnf_get_stx(bnf_t *bnf);

int stx_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t root, stx_t *stx, stx_node_t *rule);

#endif
