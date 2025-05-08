#ifndef BNF_H
#define BNF_H

#include "prs.h"
#include "stx.h"

typedef struct bnf_s {
	stx_t stx;
	stx_rule_t file;
	stx_rule_t bnf;
	stx_rule_t rules;
	stx_rule_t rule;
	stx_rule_t rname;
	stx_rule_t expr;
	stx_rule_t terms;
	stx_rule_t term;
	stx_rule_t literal;
	stx_rule_t tok;
	stx_rule_t tdouble;
	stx_rule_t tsingle;
} bnf_t;

bnf_t *bnf_init(bnf_t *bnf, alloc_t alloc);
void bnf_free(bnf_t *bnf);

const stx_t *bnf_get_stx(bnf_t *bnf);

int stx_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t root, stx_t *stx, strbuf_t *names, stx_rule_t *rule);

#endif
