#ifndef MAKE_H
#define MAKE_H

#include "list.h"
#include "str.h"
#include "strbuf.h"

#define MAKE_END ((uint)-1)

typedef list_node_t make_act_t;

typedef enum make_var_type_e {
	MAKE_VAR_INST, //:=
	MAKE_VAR_REF,  //=
	MAKE_VAR_COND, //?=
	MAKE_VAR_APP,  //+=
} make_var_type_t;

typedef enum make_str_type_e {
	MAKE_STR_STR,
	MAKE_STR_VAR,
} make_str_type_t;

typedef enum make_cmd_type_e {
	MAKE_CMD_NORMAL,
	MAKE_CMD_CHILD,
	MAKE_CMD_ERR,
} make_cmd_type_t;

typedef struct make_create_str_s {
	make_str_type_t type;
	union {
		strv_t str;
		make_act_t var;
	} val;
} make_create_str_t;

typedef struct make_create_rule_s {
	make_create_str_t target;
	strv_t action;
} make_create_rule_t;

typedef struct make_create_cmd_s {
	make_cmd_type_t type;
	strv_t arg1;
	strv_t arg2;
} make_create_cmd_t;

typedef struct make_s {
	list_t arrs;
	list_t acts;
	list_t targets;
	strbuf_t strs;
	make_act_t root;
} make_t;

typedef struct make_vars_s {
	strbuf_t names;
	arr_t flags;
	strbuf_t expanded;
	strbuf_t resolved;
} make_vars_t;

make_t *make_init(make_t *make, uint arrs_cap, uint acts_cap, uint targets_cap, uint strs_cap, alloc_t alloc);
void make_free(make_t *make);

int make_empty(make_t *make, make_act_t *act);
int make_var(make_t *make, strv_t name, make_var_type_t type, uint *id, make_act_t *act);
int make_var_ext(make_t *make, strv_t name, uint *id, make_act_t *act);
int make_rule(make_t *make, make_create_rule_t target, int file, make_act_t *act);
int make_phony(make_t *make, make_act_t *act);
int make_cmd(make_t *make, make_create_cmd_t cmd, make_act_t *act);
int make_if(make_t *make, make_create_str_t l, make_create_str_t r, make_act_t *act);
int make_def(make_t *make, strv_t name, make_act_t *act);
int make_eval_def(make_t *make, make_act_t def, make_act_t *act);
int make_inc(make_t *make, strv_t path, make_act_t *act);

int make_add_act(make_t *make, make_act_t act);
int make_var_add_val(make_t *make, make_act_t var, make_create_str_t val);
int make_rule_add_depend(make_t *make, make_act_t rule, make_create_rule_t depend);
int make_rule_add_act(make_t *make, make_act_t rule, make_act_t act);
int make_if_add_true_act(make_t *make, make_act_t mif, make_act_t act);
int make_if_add_false_act(make_t *make, make_act_t mif, make_act_t act);
int make_def_add_act(make_t *make, make_act_t def, make_act_t act);
int make_eval_def_add_arg(make_t *make, make_act_t def, make_create_str_t arg);
int make_inc_add_act(make_t *make, make_act_t inc, make_act_t act);

int make_ext_set_val(make_t *make, uint id, make_create_str_t val);

int make_rule_get_target(const make_t *make, make_create_rule_t target, make_act_t *act);

make_vars_t *make_vars_init(const make_t *make, make_vars_t *vars, alloc_t alloc);
void make_vars_free(make_vars_t *vars);

int make_vars_eval(const make_t *make, make_vars_t *vars);

strv_t make_vars_get_expanded(const make_vars_t *make, uint id);
strv_t make_vars_get_resolved(const make_vars_t *vars, uint id);

size_t make_vars_print(const make_vars_t *vars, dst_t dst);

size_t make_inc_print(const make_t *make, make_act_t inc, dst_t dst);
size_t make_print(const make_t *make, dst_t dst);

size_t make_dbg(const make_t *make, dst_t dst);

#define MSTR(_str)                                                                                                                         \
	(make_create_str_t)                                                                                                                \
	{                                                                                                                                  \
		.type = MAKE_STR_STR, .val.str = _str,                                                                                     \
	}

#define MVAR(_var)                                                                                                                         \
	(make_create_str_t)                                                                                                                \
	{                                                                                                                                  \
		.type = MAKE_STR_VAR, .val.var = _var,                                                                                     \
	}

#define MRULE(_target)                                                                                                                     \
	(make_create_rule_t)                                                                                                               \
	{                                                                                                                                  \
		.target = _target, .action = {0},                                                                                          \
	}

#define MRULEACT(_target, _action)                                                                                                         \
	(make_create_rule_t)                                                                                                               \
	{                                                                                                                                  \
		.target = _target, .action = _action,                                                                                      \
	}

#define MCMD(_cmd)                                                                                                                         \
	(make_create_cmd_t)                                                                                                                \
	{                                                                                                                                  \
		.type = MAKE_CMD_NORMAL, .arg1 = _cmd,                                                                                     \
	}

#define MCMDCHILD(_dir, _target)                                                                                                           \
	(make_create_cmd_t)                                                                                                                \
	{                                                                                                                                  \
		.type = MAKE_CMD_CHILD, .arg1 = _dir, .arg2 = _target,                                                                     \
	}

#define MCMDERR(_msg)                                                                                                                      \
	(make_create_cmd_t)                                                                                                                \
	{                                                                                                                                  \
		.type = MAKE_CMD_ERR, .arg1 = _msg,                                                                                        \
	}

#endif
