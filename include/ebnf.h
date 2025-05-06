#ifndef EBNF_H
#define EBNF_H

#include "bnf.h"
#include "esyntax.h"

typedef struct ebnf_s {
	stx_t stx;
	stx_rule_t file;
	stx_rule_t ebnf;
	stx_rule_t rules;
	stx_rule_t rule;
	stx_rule_t rname;
	stx_rule_t alt;
	stx_rule_t concat;
	stx_rule_t factor;
	stx_rule_t term;
	stx_rule_t literal;
	stx_rule_t token;
	stx_rule_t tdouble;
	stx_rule_t tsingle;
	stx_rule_t group;
	stx_rule_t opt;
	stx_rule_t rep;
	stx_rule_t opt_rep;
} ebnf_t;

ebnf_t *ebnf_init(ebnf_t *ebnf, alloc_t alloc);
void ebnf_free(ebnf_t *ebnf);

const stx_t *ebnf_get_stx(ebnf_t *ebnf, alloc_t alloc, dst_t dst);

int estx_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t root, estx_t *estx, strbuf_t *names, estx_rule_t *rule);

#endif
