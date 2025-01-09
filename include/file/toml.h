#ifndef TOML_H
#define TOML_H

#include "strbuf.h"
#include "strv.h"
#include "tree.h"

typedef tnode_t toml_val_t;

typedef enum toml_val_type_e {
	TOML_VAL_NONE,
	TOML_VAL_STR,
	TOML_VAL_INT,
	TOML_VAL_ARR,
	TOML_VAL_INL,
	TOML_VAL_TBL,
	TOML_VAL_TBL_ARR,
} toml_val_type_t;

typedef struct toml_s {
	strbuf_t strs;
	tree_t vals;
	toml_val_t root;
} toml_t;

typedef struct toml_add_val_s {
	toml_val_type_t type;
	union {
		strv_t str;
		int i;
	} val;
} toml_add_val_t;

toml_t *toml_init(toml_t *toml, size_t strs_size, size_t vals_cap, alloc_t alloc);
void toml_free(toml_t *toml);

int toml_add_val(toml_t *toml, strv_t key, toml_add_val_t val, toml_val_t *id);
int toml_arr_add_val(toml_t *toml, toml_val_t arr, toml_add_val_t val, toml_val_t *id);
int toml_tbl_add_val(toml_t *toml, toml_val_t tbl, strv_t key, toml_add_val_t val, toml_val_t *id);

int toml_print(const toml_t *toml, print_dst_t dst);

#define TOML_NONE()    ((toml_add_val_t){.type = TOML_VAL_NONE})
#define TOML_STR(_str) ((toml_add_val_t){.type = TOML_VAL_STR, .val.str = _str})
#define TOML_INT(_int) ((toml_add_val_t){.type = TOML_VAL_INT, .val.i = _int})
#define TOML_ARR()     ((toml_add_val_t){.type = TOML_VAL_ARR})
#define TOML_INL()     ((toml_add_val_t){.type = TOML_VAL_INL})
#define TOML_TBL()     ((toml_add_val_t){.type = TOML_VAL_TBL})
#define TOML_TBL_ARR() ((toml_add_val_t){.type = TOML_VAL_TBL_ARR})

#endif
