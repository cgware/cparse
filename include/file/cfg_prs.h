#ifndef CFG_PRS_H
#define CFG_PRS_H

#include "cfg.h"
#include "estx.h"

typedef struct cfg_prs_s {
	estx_t estx;
	estx_rule_t file;
	estx_rule_t cfg;
	estx_rule_t kv;
	estx_rule_t key;
	estx_rule_t val;
	estx_rule_t i;
	estx_rule_t str;
	estx_rule_t lit;
	estx_rule_t arr;
	estx_rule_t obj;
	estx_rule_t tbl;
	estx_rule_t ent;
} cfg_prs_t;

cfg_prs_t *cfg_prs_init(cfg_prs_t *cfg_prs, alloc_t alloc);
void cfg_prs_free(cfg_prs_t *cfg_prs);

cfg_var_t cfg_prs_parse(const cfg_prs_t *cfg_prs, strv_t str, cfg_t *cfg, alloc_t alloc, dst_t dst);

#endif
