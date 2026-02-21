#ifndef CFG_H
#define CFG_H

#include "list.h"
#include "strvbuf.h"

typedef list_node_t cfg_var_t;

typedef enum cfg_var_type_e {
	CFG_VAR_UNKNOWN,
	CFG_VAR_ROOT,
	CFG_VAR_LIT,
	CFG_VAR_STR,
	CFG_VAR_INT,
	CFG_VAR_ARR,
	CFG_VAR_OBJ,
	CFG_VAR_TBL,
} cfg_var_type_t;

typedef enum cfg_mode_s {
	CFG_MODE_UNKNOWN,
	CFG_MODE_SET,
	CFG_MODE_ADD,
	CFG_MODE_SUB,
	CFG_MODE_ENS,
	__CFG_MODE_CNT,
} cfg_mode_t;

typedef struct cfg_var_data_s {
	size_t key;
	cfg_var_type_t type;
	cfg_mode_t mode;
	union {
		cfg_var_t child;
		size_t str;
		int i;
	} val;
	byte has_val : 1;
	byte multiline : 1;
} cfg_var_data_t;

typedef struct cfg_s {
	list_t vars;
	strvbuf_t strs;
} cfg_t;

cfg_t *cfg_init(cfg_t *cfg, uint strs_cap, uint vars_cap, alloc_t alloc);
void cfg_free(cfg_t *cfg);

int cfg_root(cfg_t *cfg, cfg_var_t *var);
int cfg_lit(cfg_t *cfg, strv_t key, cfg_mode_t mode, strv_t str, cfg_var_t *var);
int cfg_str(cfg_t *cfg, strv_t key, cfg_mode_t mode, strv_t str, cfg_var_t *var);
int cfg_int(cfg_t *cfg, strv_t key, cfg_mode_t mode, int val, cfg_var_t *var);
int cfg_arr(cfg_t *cfg, strv_t key, cfg_mode_t mode, int multiline, cfg_var_t *var);
int cfg_obj(cfg_t *cfg, strv_t key, cfg_var_t *var);
int cfg_tbl(cfg_t *cfg, strv_t key, cfg_var_t *var);

int cfg_add_var(cfg_t *cfg, cfg_var_t parent, cfg_var_t var);

int cfg_has_var(const cfg_t *cfg, cfg_var_t parent, strv_t key, cfg_var_t *val);

strv_t cfg_get_key(const cfg_t *cfg, cfg_var_t var);

int cfg_get_lit(const cfg_t *cfg, cfg_var_t var, strv_t *val);
int cfg_get_str(const cfg_t *cfg, cfg_var_t var, strv_t *val);
int cfg_get_int(const cfg_t *cfg, cfg_var_t var, int *val);

cfg_var_data_t *cfg_it_begin(const cfg_t *cfg, cfg_var_t var, cfg_var_t *it);
cfg_var_data_t *cfg_it_next(const cfg_t *cfg, cfg_var_t *it);

size_t cfg_print(const cfg_t *cfg, cfg_var_t var, dst_t dst);

#define cfg_foreach(_cfg, _var, _val, _it) for (_val = cfg_it_begin(_cfg, _var, _it); _val; _val = cfg_it_next(_cfg, _it))

#endif
