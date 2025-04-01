#ifndef CFG_H
#define CFG_H

#include "list.h"
#include "strbuf.h"
#include "strv.h"

#define CFG_VAL_END LIST_END

typedef lnode_t cfg_var_t;

typedef enum cfg_val_type_e {
	CFG_VAL_UNKNOWN,
	CFG_VAL_ROOT,
	CFG_VAL_LIT,
	CFG_VAL_STR,
	CFG_VAL_INT,
	CFG_VAL_ARR,
	CFG_VAL_OBJ,
	CFG_VAL_TBL,
} cfg_val_type_t;

typedef struct cfg_s {
	strbuf_t strs;
	list_t vars;
} cfg_t;

typedef struct cfg_val_s {
	cfg_val_type_t type;
	union {
		strv_t str;
		int i;
	} val;
} cfg_val_t;

cfg_t *cfg_init(cfg_t *cfg, uint strs_cap, uint vars_cap, alloc_t alloc);
void cfg_free(cfg_t *cfg);

cfg_var_t cfg_var_init(cfg_t *cfg, strv_t key, cfg_val_t val);
cfg_var_t cfg_add_var(cfg_t *cfg, cfg_var_t parent, cfg_var_t var);

int cfg_get_var(const cfg_t *cfg, cfg_var_t parent, strv_t key, cfg_var_t *val);
int cfg_get_lit(const cfg_t *cfg, cfg_var_t var, strv_t *val);
int cfg_get_str(const cfg_t *cfg, cfg_var_t var, strv_t *val);
int cfg_get_int(const cfg_t *cfg, cfg_var_t var, int *val);
int cfg_get_arr(const cfg_t *cfg, cfg_var_t arr, cfg_var_t *var);

int cfg_print(const cfg_t *cfg, cfg_var_t var, print_dst_t dst);

#define CFG_NULL		  ((cfg_val_t){0})
#define CFG_NONE(_cfg, _key)	  CFG_VAL_END
#define CFG_ROOT(_cfg)		  cfg_var_init(_cfg, STRV_NULL, (cfg_val_t){.type = CFG_VAL_ROOT})
#define CFG_LIT(_cfg, _key, _str) cfg_var_init(_cfg, _key, (cfg_val_t){.type = CFG_VAL_LIT, .val.str = _str})
#define CFG_STR(_cfg, _key, _str) cfg_var_init(_cfg, _key, (cfg_val_t){.type = CFG_VAL_STR, .val.str = _str})
#define CFG_INT(_cfg, _key, _int) cfg_var_init(_cfg, _key, (cfg_val_t){.type = CFG_VAL_INT, .val.i = _int})
#define CFG_ARR(_cfg, _key)	  cfg_var_init(_cfg, _key, (cfg_val_t){.type = CFG_VAL_ARR})
#define CFG_OBJ(_cfg, _key)	  cfg_var_init(_cfg, _key, (cfg_val_t){.type = CFG_VAL_OBJ})
#define CFG_TBL(_cfg, _key)	  cfg_var_init(_cfg, _key, (cfg_val_t){.type = CFG_VAL_TBL})

#endif
