#ifndef CFG_H
#define CFG_H

#include "list.h"
#include "strbuf.h"
#include "strv.h"

#define CFG_VAR_END ((uint)-1)

typedef list_node_t cfg_var_t;

typedef struct cfg_s {
	strbuf_t strs;
	list_t vars;
} cfg_t;

cfg_t *cfg_init(cfg_t *cfg, uint strs_cap, uint vars_cap, alloc_t alloc);
void cfg_free(cfg_t *cfg);

int cfg_root(cfg_t *cfg, cfg_var_t *var);
int cfg_lit(cfg_t *cfg, strv_t key, strv_t str, cfg_var_t *var);
int cfg_str(cfg_t *cfg, strv_t key, strv_t str, cfg_var_t *var);
int cfg_int(cfg_t *cfg, strv_t key, int val, cfg_var_t *var);
int cfg_arr(cfg_t *cfg, strv_t key, cfg_var_t *var);
int cfg_obj(cfg_t *cfg, strv_t key, cfg_var_t *var);
int cfg_tbl(cfg_t *cfg, strv_t key, cfg_var_t *var);

int cfg_add_var(cfg_t *cfg, cfg_var_t parent, cfg_var_t var);

int cfg_get_var(const cfg_t *cfg, cfg_var_t parent, strv_t key, cfg_var_t *val);
int cfg_get_lit(const cfg_t *cfg, cfg_var_t var, strv_t *val);
int cfg_get_str(const cfg_t *cfg, cfg_var_t var, strv_t *val);
int cfg_get_int(const cfg_t *cfg, cfg_var_t var, int *val);
int cfg_get_arr(const cfg_t *cfg, cfg_var_t arr, cfg_var_t *var);

size_t cfg_print(const cfg_t *cfg, cfg_var_t var, dst_t dst);

#endif
