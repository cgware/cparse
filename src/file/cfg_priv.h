#ifndef CFG_PRIV_H
#define CFG_PRIV_H

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

typedef struct var_data_s {
	uint key;
	cfg_var_type_t type;
	union {
		cfg_var_t child;
		uint str;
		int i;
	} val;
} cfg_var_data_t;

#endif
