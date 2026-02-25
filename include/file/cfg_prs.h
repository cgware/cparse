#ifndef CFG_PRS_H
#define CFG_PRS_H

#include "cfg.h"
#include "eprs.h"

typedef struct cfg_prs_s {
	estx_t estx;
	estx_node_t file;
	estx_node_t cfg;
	estx_node_t tv;
	estx_node_t vals;
	estx_node_t key;
	estx_node_t val;
	estx_node_t i;
	estx_node_t str;
	estx_node_t lit;
	estx_node_t arr;
	estx_node_t obj;
	estx_node_t kv;
	estx_node_t mode;
	estx_node_t tbl;
	estx_node_t name;
	estx_node_t ent;
	lex_t lex;
	eprs_t eprs;
} cfg_prs_t;

cfg_prs_t *cfg_prs_init(cfg_prs_t *cfg_prs, alloc_t alloc);
void cfg_prs_free(cfg_prs_t *cfg_prs);

int cfg_prs_parse(cfg_prs_t *cfg_prs, strv_t str, cfg_t *cfg, cfg_var_t *root, dst_t dst);

#endif
