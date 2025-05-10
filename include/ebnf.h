#ifndef EBNF_H
#define EBNF_H

#include "bnf.h"
#include "estx.h"

typedef struct ebnf_s {
	stx_t stx;
	stx_node_t file;
	stx_node_t ebnf;
	stx_node_t rules;
	stx_node_t rule;
	stx_node_t rname;
	stx_node_t alt;
	stx_node_t concat;
	stx_node_t factor;
	stx_node_t term;
	stx_node_t literal;
	stx_node_t tok;
	stx_node_t tdouble;
	stx_node_t tsingle;
	stx_node_t group;
	stx_node_t opt;
	stx_node_t rep;
	stx_node_t opt_rep;
} ebnf_t;

ebnf_t *ebnf_init(ebnf_t *ebnf, alloc_t alloc);
void ebnf_free(ebnf_t *ebnf);

const stx_t *ebnf_get_stx(ebnf_t *ebnf, alloc_t alloc, dst_t dst);

int estx_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t root, estx_t *estx, estx_node_t *rule);

#endif
