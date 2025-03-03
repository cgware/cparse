#ifndef TOML_H
#define TOML_H

#include "strbuf.h"
#include "strv.h"
#include "tree.h"

#define TOML_VAL_END TREE_END

typedef tnode_t toml_val_t;

typedef enum toml_val_type_e {
	TOML_VAL_UNKNOWN,
	TOML_VAL_STRL,
	TOML_VAL_INT,
	TOML_VAL_ARR,
	TOML_VAL_INL,
	TOML_VAL_TBL,
	TOML_VAL_TBL_ARR,
} toml_val_type_t;

typedef struct toml_s {
	strbuf_t strs;
	tree_t vals;
} toml_t;

typedef struct toml_add_val_s {
	toml_val_type_t type;
	union {
		strv_t str;
		int i;
	} val;
} toml_add_val_t;

toml_t *toml_init(toml_t *toml, uint strs_cap, uint vals_cap, alloc_t alloc);
void toml_free(toml_t *toml);

toml_val_t toml_val_init(toml_t *toml, strv_t key, toml_add_val_t val);
toml_val_t toml_add_val(toml_t *toml, toml_val_t parent, toml_val_t val);

int toml_print(const toml_t *toml, toml_val_t val, print_dst_t dst);

#define TOML_NULL ((toml_add_val_t){0})

#define TOML_NONE(_toml, _key)	     TOML_VAL_END
#define TOML_STRL(_toml, _key, _str) toml_val_init(_toml, _key, (toml_add_val_t){.type = TOML_VAL_STRL, .val.str = _str})
#define TOML_INT(_toml, _key, _int)  toml_val_init(_toml, _key, (toml_add_val_t){.type = TOML_VAL_INT, .val.i = _int})
#define TOML_ARR(_toml, _key)	     toml_val_init(_toml, _key, (toml_add_val_t){.type = TOML_VAL_ARR})
#define TOML_INL(_toml, _key)	     toml_val_init(_toml, _key, (toml_add_val_t){.type = TOML_VAL_INL})
#define TOML_TBL(_toml, _key)	     toml_val_init(_toml, _key, (toml_add_val_t){.type = TOML_VAL_TBL})
#define TOML_TBL_ARR(_toml, _key)    toml_val_init(_toml, _key, (toml_add_val_t){.type = TOML_VAL_TBL_ARR})

#endif
