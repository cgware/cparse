#ifndef TOML_PARSE_H
#define TOML_PARSE_H

#include "esyntax.h"
#include "toml.h"

typedef struct toml_prs_s {
	estx_t estx;
	estx_rule_t file;
	estx_rule_t toml;
	estx_rule_t kv;
	estx_rule_t key;
	estx_rule_t val;
	estx_rule_t i;
	estx_rule_t strl;
	estx_rule_t arr;
	estx_rule_t inl;
	estx_rule_t tbl;
	estx_rule_t tbla;
	estx_rule_t ent;
} toml_prs_t;

toml_prs_t *toml_prs_init(toml_prs_t *toml_prs, alloc_t alloc);
void toml_prs_free(toml_prs_t *toml_prs);

toml_val_t toml_prs_parse(const toml_prs_t *toml_prs, strv_t str, toml_t *toml, alloc_t alloc, print_dst_t dst);

#endif
