#ifndef MAKE_H
#define MAKE_H

#include "list.h"
#include "str.h"
#include "strbuf.h"

#define MAKE_END LIST_END

typedef lnode_t make_str_t;

typedef lnode_t make_act_t;
typedef make_act_t make_empty_t;
typedef make_act_t make_var_t;
typedef make_act_t make_rule_t;
typedef make_act_t make_cmd_t;
typedef make_act_t make_if_t;
typedef make_act_t make_def_t;
typedef make_act_t make_eval_def_t;
typedef make_act_t make_inc_t;

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
		make_var_t var;
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

make_empty_t make_create_empty(make_t *make);
make_var_t make_create_var(make_t *make, strv_t name, make_var_type_t type, uint *id);
make_var_t make_create_var_ext(make_t *make, strv_t name, uint *id);
make_rule_t make_create_rule(make_t *make, make_create_rule_t target, int file);
make_rule_t make_create_phony(make_t *make);
make_cmd_t make_create_cmd(make_t *make, make_create_cmd_t cmd);
make_if_t make_create_if(make_t *make, make_create_str_t l, make_create_str_t r);
make_def_t make_create_def(make_t *make, strv_t name);
make_eval_def_t make_create_eval_def(make_t *make, make_def_t def);
make_inc_t make_create_inc(make_t *make, strv_t path);

make_act_t make_add_act(make_t *make, make_act_t act);
make_var_t make_var_add_val(make_t *make, make_var_t var, make_create_str_t val);
make_str_t make_rule_add_depend(make_t *make, make_rule_t rule, make_create_rule_t depend);
make_act_t make_rule_add_act(make_t *make, make_rule_t rule, make_act_t act);
make_act_t make_if_add_true_act(make_t *make, make_if_t mif, make_act_t act);
make_act_t make_if_add_false_act(make_t *make, make_if_t mif, make_act_t act);
make_act_t make_def_add_act(make_t *make, make_def_t def, make_act_t act);
make_var_t make_eval_def_add_arg(make_t *make, make_eval_def_t def, make_create_str_t arg);
make_act_t make_inc_add_act(make_t *make, make_inc_t inc, make_act_t act);

make_str_t make_ext_set_val(make_t *make, uint id, make_create_str_t val);

make_rule_t make_rule_get_target(const make_t *make, make_create_rule_t target);

make_vars_t *make_vars_init(const make_t *make, make_vars_t *vars, alloc_t alloc);
void make_vars_free(make_vars_t *vars);

int make_vars_eval(const make_t *make, make_vars_t *vars);

strv_t make_vars_get_expanded(const make_vars_t *make, uint id);
strv_t make_vars_get_resolved(const make_vars_t *vars, uint id);

int make_vars_print(const make_vars_t *vars, print_dst_t dst);

int make_inc_print(const make_t *make, make_inc_t inc, print_dst_t dst);
int make_print(const make_t *make, print_dst_t dst);

int make_dbg(const make_t *make, print_dst_t dst);

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
