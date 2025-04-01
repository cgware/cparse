#ifndef TOML_H
#define TOML_H

#include "list.h"
#include "strbuf.h"
#include "strv.h"

#define TOML_VAL_END LIST_END

typedef lnode_t toml_var_t;

typedef enum toml_val_type_e {
	TOML_VAL_UNKNOWN,
	TOML_VAL_ROOT,
	TOML_VAL_STRL,
	TOML_VAL_INT,
	TOML_VAL_ARR,
	TOML_VAL_INL,
	TOML_VAL_TBL,
	TOML_VAL_TBL_ARR,
} toml_val_type_t;

typedef struct toml_s {
	strbuf_t strs;
	list_t vars;
} toml_t;

typedef struct toml_val_s {
	toml_val_type_t type;
	union {
		strv_t str;
		int i;
	} val;
} toml_val_t;

toml_t *toml_init(toml_t *toml, uint strs_cap, uint vars_cap, alloc_t alloc);
void toml_free(toml_t *toml);

toml_var_t toml_var_init(toml_t *toml, strv_t key, toml_val_t val);
toml_var_t toml_add_var(toml_t *toml, toml_var_t parent, toml_var_t var);

int toml_get_var(const toml_t *toml, toml_var_t parent, strv_t key, toml_var_t *val);
int toml_get_str(const toml_t *toml, toml_var_t var, strv_t *val);
int toml_get_int(const toml_t *toml, toml_var_t var, int *val);
int toml_get_arr(const toml_t *toml, toml_var_t arr, toml_var_t *var);

int toml_print(const toml_t *toml, toml_var_t var, print_dst_t dst);

#define TOML_NULL		     ((toml_val_t){0})
#define TOML_NONE(_toml, _key)	     TOML_VAL_END
#define TOML_ROOT(_toml)	     toml_var_init(_toml, STRV_NULL, (toml_val_t){.type = TOML_VAL_ROOT})
#define TOML_STRL(_toml, _key, _str) toml_var_init(_toml, _key, (toml_val_t){.type = TOML_VAL_STRL, .val.str = _str})
#define TOML_INT(_toml, _key, _int)  toml_var_init(_toml, _key, (toml_val_t){.type = TOML_VAL_INT, .val.i = _int})
#define TOML_ARR(_toml, _key)	     toml_var_init(_toml, _key, (toml_val_t){.type = TOML_VAL_ARR})
#define TOML_INL(_toml, _key)	     toml_var_init(_toml, _key, (toml_val_t){.type = TOML_VAL_INL})
#define TOML_TBL(_toml, _key)	     toml_var_init(_toml, _key, (toml_val_t){.type = TOML_VAL_TBL})
#define TOML_TBL_ARR(_toml, _key)    toml_var_init(_toml, _key, (toml_val_t){.type = TOML_VAL_TBL_ARR})

#endif
