#ifndef MAKE_PRIV_H
#define MAKE_PRIV_H

#define MAX_VAR_VALUE_LEN 256

typedef struct make_var_data_s {
	size_t name;
	make_var_type_t type;
	list_node_t values;
	size_t expanded;
	size_t resolved;
	uint8_t scope;
	struct {
		uint8_t ext : 1;
		uint8_t ref : 1;
	} flags;
	byte ext : 1;
	byte def : 1;
} make_var_data_t;

typedef struct make_str_data_s {
	make_str_type_t type;
	union {
		size_t val;
		make_act_t var;
	} val;
} make_str_data_t;

typedef struct make_rule_target_data_s {
	make_str_data_t target;
	size_t action;
	byte has_action : 1;
} make_rule_target_data_t;

typedef struct make_rule_data_s {
	make_rule_target_data_t target;
	list_node_t depends;
	make_act_t acts;
	byte file : 1;
} make_rule_data_t;

typedef struct make_cmd_data_s {
	make_cmd_type_t type;
	uint args;
	size_t arg[2];
} make_cmd_data_t;

typedef struct make_if_data_s {
	make_str_data_t l;
	size_t lexpanded;
	size_t lresolved;
	make_str_data_t r;
	size_t rexpanded;
	size_t rresolved;
	make_act_t true_acts;
	make_act_t false_acts;
} make_if_data_t;

typedef struct make_def_data_s {
	size_t name;
	make_act_t acts;
} make_def_data_t;

typedef struct make_eval_def_data_s {
	make_act_t def;
	list_node_t args;
} make_eval_def_data_t;

typedef struct make_inc_data_s {
	size_t path;
	make_act_t acts;
} make_inc_data_t;

typedef enum make_act_type_e {
	MAKE_ACT_EMPTY,
	MAKE_ACT_VAR,
	MAKE_ACT_RULE,
	MAKE_ACT_CMD,
	MAKE_ACT_IF,
	MAKE_ACT_DEF,
	MAKE_ACT_EVAL_DEF,
	MAKE_ACT_INCLUDE,
} make_act_type_t;

typedef struct make_act_data_s {
	make_act_type_t type;
	union {
		make_var_data_t var;
		make_rule_data_t rule;
		make_cmd_data_t cmd;
		make_if_data_t mif;
		make_def_data_t def;
		make_eval_def_data_t eval_def;
		make_inc_data_t inc;
	} val;
} make_act_data_t;

#endif
