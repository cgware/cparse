#include "file/make.h"

#include "log.h"
#include "platform.h"

#define MAX_VAR_VALUE_LEN 256

typedef struct make_var_data_s {
	uint id;
	make_var_type_t type;
	lnode_t values;
	int ext : 1;
	int def : 1;
} make_var_data_t;

typedef struct make_str_data_s {
	make_str_type_t type;
	union {
		uint id;
		make_var_t var;
	} val;
} make_str_data_t;

typedef struct make_rule_target_data_s {
	make_str_data_t target;
	uint action;
} make_rule_target_data_t;

typedef struct make_rule_data_s {
	make_rule_target_data_t target;
	lnode_t depends;
	make_act_t acts;
	int file : 1;
} make_rule_data_t;

typedef struct make_cmd_data_s {
	make_cmd_type_t type;
	uint arg1;
	uint arg2;
} make_cmd_data_t;

typedef struct make_if_data_s {
	make_str_data_t l;
	make_str_data_t r;
	make_act_t true_acts;
	make_act_t false_acts;
} make_if_data_t;

typedef struct make_def_data_s {
	uint name;
	make_act_t acts;
} make_def_data_t;

typedef struct make_eval_def_data_s {
	make_def_t def;
	lnode_t args;
} make_eval_def_data_t;

typedef struct make_inc_data_s {
	uint path;
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

typedef struct make_var_flags_s {
	uint8_t ext : 1;
	uint8_t ref : 1;
	uint8_t local : 1;
} make_var_flags_t;

static inline make_act_data_t *make_act_get(const make_t *make, make_act_t act)
{
	if (make == NULL) {
		return NULL;
	}

	return list_get(&make->acts, act);
}

static inline void *make_act_get_type(const make_t *make, make_act_t act, make_act_type_t type)
{
	make_act_data_t *data = make_act_get(make, act);
	if (data == NULL || data->type != type) {
		return NULL;
	}

	return &data->val;
}

static make_var_t make_var_get_name(const make_t *make, uint id)
{
	const make_act_data_t *act;
	make_var_t i = 0;
	list_foreach_all(&make->acts, i, act)
	{
		if (act->type == MAKE_ACT_VAR && act->val.var.id == id) {
			return i;
		}
	}

	return MAKE_END;
}

make_t *make_init(make_t *make, uint arrs_cap, uint acts_cap, uint targets_cap, uint strs_cap, alloc_t alloc)
{
	if (make == NULL) {
		return NULL;
	}

	if (list_init(&make->arrs, arrs_cap, sizeof(make_str_data_t), alloc) == NULL) {
		return NULL;
	}

	if (list_init(&make->acts, acts_cap, sizeof(make_act_data_t), alloc) == NULL) {
		return NULL;
	}

	if (list_init(&make->targets, targets_cap, sizeof(make_rule_target_data_t), alloc) == NULL) {
		return NULL;
	}

	if (strbuf_init(&make->strs, strs_cap, 8, alloc) == NULL) {
		return NULL;
	}

	make->root = MAKE_END;

	return make;
}

void make_free(make_t *make)
{
	if (make == NULL) {
		return;
	}

	list_free(&make->arrs);
	list_free(&make->acts);
	list_free(&make->targets);
	strbuf_free(&make->strs);

	make->root = MAKE_END;
}

make_empty_t make_create_empty(make_t *make)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_act_t act;
	make_act_data_t *data = list_add(&make->acts, &act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_EMPTY,
	};

	return act;
}

static make_var_t make_create_var_ex(make_t *make, strv_t name, make_var_type_t type, uint *id, int ext)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_act_t act;
	make_act_data_t *data = list_add(&make->acts, &act);

	if (data == NULL) {
		return MAKE_END;
	}

	uint index;
	if (id && *id < make->strs.off.cnt) {
		index = *id;
	} else if (strbuf_add(&make->strs, name, &index)) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_VAR,
		.val.var =
			{
				.id	= index,
				.type	= type,
				.values = MAKE_END,
				.ext	= ext,
				.def	= 0,
			},
	};

	if (id) {
		*id = index;
	}

	return act;
}

make_var_t make_create_var(make_t *make, strv_t name, make_var_type_t type, uint *id)
{
	return make_create_var_ex(make, name, type, id, 0);
}

make_var_t make_create_var_ext(make_t *make, strv_t name, uint *id)
{
	return make_create_var_ex(make, name, MAKE_VAR_REF, id, 1);
}

static int create_str(make_t *make, make_create_str_t str, make_str_data_t *data)
{
	if (str.type != MAKE_STR_STR) {
		*data = (make_str_data_t){.type = str.type, .val.var = str.val.var};
		return 0;
	}

	uint index;
	if (strbuf_add(&make->strs, str.val.str, &index)) {
		return 1;
	}

	*data = (make_str_data_t){.type = str.type, .val.id = index};
	return 0;
}

make_rule_t make_create_rule(make_t *make, make_create_rule_t target, int file)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_act_t act;
	make_act_data_t *data = list_add(&make->acts, &act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_RULE,
		.val.rule =
			{
				.target.action = (uint)-1,
				.depends       = MAKE_END,
				.acts	       = MAKE_END,
				.file	       = file,
			},
	};

	if (create_str(make, target.target, &data->val.rule.target.target)) {
		return MAKE_END;
	}

	if (target.action.data != NULL && strbuf_add(&make->strs, target.action, &data->val.rule.target.action)) {
		return MAKE_END;
	}

	return act;
}

make_rule_t make_create_phony(make_t *make)
{
	return make_create_rule(make, MRULE(MSTR(STRV(".PHONY"))), 1);
}

make_cmd_t make_create_cmd(make_t *make, make_create_cmd_t cmd)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_act_t act;
	make_act_data_t *data = list_add(&make->acts, &act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_CMD,
		.val.cmd =
			{
				.type = cmd.type,
				.arg1 = (uint)-1,
				.arg2 = (uint)-1,
			},
	};

	if (cmd.arg1.data != NULL && strbuf_add(&make->strs, cmd.arg1, &data->val.cmd.arg1)) {
		return MAKE_END;
	}

	if (cmd.arg2.data != NULL && strbuf_add(&make->strs, cmd.arg2, &data->val.cmd.arg2)) {
		return MAKE_END;
	}

	return act;
}

make_if_t make_create_if(make_t *make, make_create_str_t l, make_create_str_t r)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_act_t act;
	make_act_data_t *data = list_add(&make->acts, &act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_IF,
		.val.mif =
			{
				.true_acts  = MAKE_END,
				.false_acts = MAKE_END,
			},
	};

	if (create_str(make, l, &data->val.mif.l)) {
		return MAKE_END;
	}

	if (create_str(make, r, &data->val.mif.r)) {
		return MAKE_END;
	}

	return act;
}

make_def_t make_create_def(make_t *make, strv_t name)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_act_t act;
	make_act_data_t *data = list_add(&make->acts, &act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_DEF,
		.val.def =
			{
				.acts = MAKE_END,
			},
	};

	if (strbuf_add(&make->strs, name, &data->val.def.name)) {
		return MAKE_END;
	}

	return act;
}

static make_var_t eval_def_add_arg(make_t *make, make_eval_def_t def, make_str_data_t arg)
{
	make_eval_def_data_t *data = make_act_get_type(make, def, MAKE_ACT_EVAL_DEF);

	if (data == NULL) {
		return MAKE_END;
	}

	make_str_data_t *target;
	if (data->args < make->arrs.cnt) {
		target = list_add_next(&make->arrs, data->args, NULL);
	} else {
		target = list_add(&make->arrs, &data->args);
	}

	if (target == NULL) {
		return MAKE_END;
	}

	*target = arg;

	return def;
}

make_var_t make_create_eval_def(make_t *make, make_def_t def)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_act_t act;
	make_act_data_t *data = list_add(&make->acts, &act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_EVAL_DEF,
		.val.eval_def =
			{
				.def  = def,
				.args = MAKE_END,
			},
	};

	make_def_data_t *def_data = make_act_get_type(make, def, MAKE_ACT_DEF);
	if (def_data == NULL) {
		return MAKE_END;
	}

	return eval_def_add_arg(make, act, (make_str_data_t){.type = MAKE_STR_STR, .val.id = def_data->name});
}

make_inc_t make_create_inc(make_t *make, strv_t path)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_act_t act;
	make_act_data_t *data = list_add(&make->acts, &act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_INCLUDE,
		.val.inc =
			{
				.acts = MAKE_END,
			},
	};

	if (strbuf_add(&make->strs, path, &data->val.inc.path)) {
		return MAKE_END;
	}

	return act;
}

static make_str_t rule_add_depend(make_t *make, make_rule_t rule, make_rule_target_data_t depend)
{
	make_rule_data_t *data = make_act_get_type(make, rule, MAKE_ACT_RULE);

	if (data == NULL) {
		return MAKE_END;
	}

	make_str_t str;
	make_rule_target_data_t *target;
	if (data->depends < make->targets.cnt) {
		target = list_add_next(&make->targets, data->depends, &str);
	} else {
		target	      = list_add(&make->targets, &str);
		data->depends = str;
	}

	if (target == NULL) {
		return MAKE_END;
	}

	*target = depend;

	return str;
}

make_act_t make_add_act(make_t *make, make_act_t act)
{
	make_act_data_t *data = make_act_get(make, act);
	if (data == NULL) {
		return MAKE_END;
	}

	if (data->type == MAKE_ACT_RULE && !data->val.rule.file) {
		make_rule_t phony = make_rule_get_target(make, MRULE(MSTR(STRV(".PHONY"))));
		if (phony == MAKE_END) {
			phony = make_add_act(make, make_create_phony(make));
		}
		rule_add_depend(make, phony, data->val.rule.target);
	}

	if (make->root < make->acts.cnt) {
		list_set_next(&make->acts, make->root, act);
	} else {
		make->root = act;
	}

	return act;
}

make_var_t make_var_add_val(make_t *make, make_var_t var, make_create_str_t val)
{
	make_var_data_t *data = make_act_get_type(make, var, MAKE_ACT_VAR);

	if (data == NULL || data->ext) {
		return MAKE_END;
	}

	make_str_data_t *target;
	if (data->values < make->arrs.cnt) {
		target = list_add_next(&make->arrs, data->values, NULL);
	} else {
		target = list_add(&make->arrs, &data->values);
	}

	if (target == NULL) {
		return MAKE_END;
	}

	if (create_str(make, val, target)) {
		return MAKE_END;
	}

	return var;
}

make_str_t make_rule_add_depend(make_t *make, make_rule_t rule, make_create_rule_t depend)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_rule_target_data_t target = {.action = (uint)-1};

	if (create_str(make, depend.target, &target.target)) {
		return MAKE_END;
	}

	if (depend.action.data != NULL && strbuf_add(&make->strs, depend.action, &target.action)) {
		return MAKE_END;
	}

	return rule_add_depend(make, rule, target);
}

make_act_t make_rule_add_act(make_t *make, make_rule_t rule, make_act_t act)
{
	make_rule_data_t *data = make_act_get_type(make, rule, MAKE_ACT_RULE);

	if (data == NULL || make_act_get(make, act) == NULL) {
		return MAKE_END;
	}

	if (data->acts < make->acts.cnt) {
		list_set_next(&make->acts, data->acts, act);
	} else {
		data->acts = act;
	}

	return act;
}

static make_act_t make_if_add_act(make_t *make, make_if_t mif, int true_acts, make_act_t act)
{
	make_if_data_t *data = make_act_get_type(make, mif, MAKE_ACT_IF);

	if (data == NULL || make_act_get(make, act) == NULL) {
		return MAKE_END;
	}

	make_act_t *acts = true_acts ? &data->true_acts : &data->false_acts;

	if (*acts < make->acts.cnt) {
		list_set_next(&make->acts, *acts, act);
	} else {
		*acts = act;
	}

	return act;
}

make_act_t make_if_add_true_act(make_t *make, make_if_t mif, make_act_t act)
{
	return make_if_add_act(make, mif, 1, act);
}

make_act_t make_if_add_false_act(make_t *make, make_if_t mif, make_act_t act)
{
	return make_if_add_act(make, mif, 0, act);
}

make_act_t make_def_add_act(make_t *make, make_def_t def, make_act_t act)
{
	make_def_data_t *def_data = make_act_get_type(make, def, MAKE_ACT_DEF);
	make_act_data_t *act_data = make_act_get(make, act);

	if (def_data == NULL || act_data == NULL) {
		return MAKE_END;
	}

	if (act_data->type == MAKE_ACT_VAR) {
		act_data->val.var.def = -1;
	}

	if (def_data->acts < make->acts.cnt) {
		list_set_next(&make->acts, def_data->acts, act);
	} else {
		def_data->acts = act;
	}

	return act;
}

make_var_t make_eval_def_add_arg(make_t *make, make_eval_def_t def, make_create_str_t arg)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_str_data_t str;
	if (create_str(make, arg, &str)) {
		return MAKE_END;
	}

	return eval_def_add_arg(make, def, str);
}

make_act_t make_inc_add_act(make_t *make, make_inc_t inc, make_act_t act)
{
	make_inc_data_t *data = make_act_get_type(make, inc, MAKE_ACT_INCLUDE);

	if (data == NULL || make_act_get(make, act) == NULL) {
		return MAKE_END;
	}

	if (data->acts < make->acts.cnt) {
		list_set_next(&make->acts, data->acts, act);
	} else {
		data->acts = act;
	}

	return act;
}

make_str_t make_ext_set_val(make_t *make, uint id, make_create_str_t val)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_t act  = make_var_get_name(make, id);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL || !data->val.var.ext) {
		return MAKE_END;
	}

	make_str_t str;
	make_str_data_t *target;

	lnode_t values = ((make_var_data_t *)make_act_get_type(make, act, MAKE_ACT_VAR))->values;
	if (values < make->arrs.cnt) {
		str    = values;
		target = list_get(&make->arrs, str);
	} else {
		target = list_add(&make->arrs, &str);

		((make_var_data_t *)make_act_get_type(make, act, MAKE_ACT_VAR))->values = str;
	}

	if (target == NULL) {
		return MAKE_END;
	}

	if (create_str(make, val, target)) {
		return MAKE_END;
	}

	return str;
}

static int make_rule_target_eq(const make_t *make, make_rule_target_data_t target1, make_create_rule_t target2)
{
	if (target1.target.type != target2.target.type) {
		return 0;
	}

	if (!strv_eq(target1.action < make->strs.off.cnt ? strbuf_get(&make->strs, target1.action) : STRV_NULL, target2.action)) {
		return 0;
	}

	switch (target1.target.type) {
	case MAKE_STR_VAR:
		return ((make_var_data_t *)make_act_get_type(make, target1.target.val.var, MAKE_ACT_VAR))->id ==
		       ((make_var_data_t *)make_act_get_type(make, target2.target.val.var, MAKE_ACT_VAR))->id;
	default: return strv_eq(strbuf_get(&make->strs, target1.target.val.id), target2.target.val.str);
	}
}

make_rule_t make_rule_get_target(const make_t *make, make_create_rule_t target)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_data_t *act;
	make_act_t rule = MAKE_END;

	make_rule_t i = 0;
	list_foreach_all(&make->acts, i, act)
	{
		if (act->type == MAKE_ACT_RULE && make_rule_target_eq(make, act->val.rule.target, target)) {
			return i;
		}
	}

	return rule;
}

make_vars_t *make_vars_init(const make_t *make, make_vars_t *vars, alloc_t alloc)
{
	if (make == NULL || vars == NULL) {
		return NULL;
	}

	if (strbuf_init(&vars->names, make->strs.off.cnt, 16, alloc) == NULL ||
	    arr_init(&vars->flags, make->strs.off.cnt, sizeof(make_var_flags_t), alloc) == NULL ||
	    strbuf_init(&vars->expanded, make->strs.off.cnt, 16, alloc) == NULL ||
	    strbuf_init(&vars->resolved, make->strs.off.cnt, 16, alloc) == NULL) {
		return NULL;
	}

	return vars;
}

void make_vars_free(make_vars_t *vars)
{
	if (vars == NULL) {
		return;
	}

	strbuf_free(&vars->names);
	arr_free(&vars->flags);
	strbuf_free(&vars->expanded);
	strbuf_free(&vars->resolved);
}

typedef struct replace_vars_priv_s {
	const make_vars_t *vars;
	int resolve;
} replace_vars_priv_t;

static int replace_vars(const void *priv, strv_t name, strv_t *val)
{
	const replace_vars_priv_t *p = priv;
	uint index;
	if (strbuf_find(&p->vars->names, name, &index)) {
		return 1;
	}

	make_var_flags_t *flags = arr_get(&p->vars->flags, index);
	if (!p->resolve && flags->local) {
		return 1;
	}

	*val = strbuf_get(&p->vars->resolved, index);
	return 0;
}

typedef struct replace_args_priv_s {
	const make_t *make;
	lnode_t args;
	replace_vars_priv_t *vars;
} replace_args_priv_t;

static int replace_args(const void *priv, strv_t name, strv_t *val)
{
	const replace_args_priv_t *p = priv;

	if (p->args < p->make->arrs.cnt) {
		uint id = p->make->arrs.cnt;
		if (strv_eq(name, STRV("0"))) {
			id = 0;
		} else if (strv_eq(name, STRV("1"))) {
			id = 1;
		} else if (strv_eq(name, STRV("2"))) {
			id = 2;
		} else if (strv_eq(name, STRV("3"))) {
			id = 3;
		}

		if (id < p->make->arrs.cnt) {
			make_str_t index;
			const make_str_data_t *data = list_get_at(&p->make->arrs, p->args, id, &index);
			if (data == NULL) {
				return 1;
			}

			*val = strbuf_get(&p->make->strs, data->val.id);
			return 0;
		}
	}

	if (p->vars && replace_vars(p->vars, name, val) == 0) {
		return 0;
	}

	return 1;
}

typedef int (*replace_fn)(const void *priv, strv_t name, strv_t *val);

static int make_replace(str_t *str, size_t min_len, replace_fn replace, const void *priv)
{
	if (str->data == NULL) {
		return 1;
	}

	int ret = 0;

	for (size_t s = 0; str->len >= min_len + 3 && s <= str->len - min_len - 3; s++) {
		if (str->data[s] == '$' && str->data[s + 1] == '$') {
			str_subreplace(str, s, s + 2, STRV("$"));
			continue;
		}

		if (str->data[s] != '$' || str->data[s + 1] != '(') {
			continue;
		}

		size_t e = s + 2;
		while (e < str->len && str->data[e] != ')') {
			e++;
		}

		if (e >= str->len) {
			break;
		}

		strv_t name = STRVN(&str->data[s + 2], e - (s + 2));
		strv_t val  = STRV("");
		if (replace(priv, name, &val)) {
			log_warn("build", "var", NULL, "failed to get value of: '%.*s'", name.len, name.data);
			ret = 1;
		}

		if (str_subreplace(str, s, e + 1, val)) {
			// LCOV_EXCL_START
			log_warn("build", "var", NULL, "failed to replace '%.*s' with: '%.*s'", name.len, name.data, val.len, val.data);
			ret = 1;
			continue;
			// LCOV_EXCL_STOP
		}
		s--;
	}

	((char *)str->data)[str->len] = '\0';
	return ret;
}

static int eval_args(const make_t *make, const make_vars_t *vars, lnode_t args, str_t *buf)
{
	replace_vars_priv_t vars_priv = vars? (replace_vars_priv_t){
		.vars = vars,
		.resolve = 0,
	} : (replace_vars_priv_t){0};

	replace_args_priv_t args_priv = {
		.make = make,
		.args = args,
		.vars = vars ? &vars_priv : NULL,
	};

	return args == MAKE_END ? 0 : make_replace(buf, 1, replace_args, &args_priv);
}

static int eval_name(const make_t *make, const make_vars_t *vars, lnode_t args, str_t *buf)
{
	replace_vars_priv_t vars_priv = vars? (replace_vars_priv_t){
		.vars = vars,
		.resolve = 0,
	} : (replace_vars_priv_t){0};

	replace_args_priv_t args_priv = {
		.make = make,
		.args = args,
		.vars = vars ? &vars_priv : NULL,
	};

	return make_replace(buf, 1, replace_args, &args_priv);
}

static int make_str_expand(const make_t *make, const make_str_data_t *str, str_t *buf)
{
	int ret = 0;
	switch (str->type) {
	case MAKE_STR_VAR: {
		make_var_data_t *data = make_act_get_type(make, str->val.var, MAKE_ACT_VAR);
		if (data == NULL) {
			log_warn("cparse", "make", NULL, "variable not found");
			return 1;
		}
		str_cat(buf, data->def ? STRV("$$(") : STRV("$("));
		str_cat(buf, strbuf_get(&make->strs, data->id));
		str_cat(buf, STRV(")"));
		break;
	}
	default: {
		str_cat(buf, strbuf_get(&make->strs, str->val.id));
		break;
	}
	}

	return ret;
}

static int make_var_app(make_vars_t *vars, uint id, int app, str_t *buf)
{
	make_var_flags_t *flags = arr_get(&vars->flags, id);

	int ret = 0;

	if (strbuf_get(&vars->expanded, id).len > 0 && buf->len > 0) {
		strbuf_app(&vars->expanded, id, STRV(" "));
	}

	if (app) {
		if (strbuf_app(&vars->expanded, id, STRVS(*buf))) {
			log_error("cparse", "make", NULL, "failed to append variable"), ret = 1; // LCOV_EXCL_LINE
		}
	} else {
		if (strbuf_set(&vars->expanded, id, STRVS(*buf))) {
			log_error("cparse", "make", NULL, "failed to set variable"), ret = 1; // LCOV_EXCL_LINE
		}
	}

	if (!flags->ref) {
		replace_vars_priv_t priv = {
			.vars	 = vars,
			.resolve = 1,
		};

		ret |= make_replace(buf, 1, replace_vars, &priv);
	}

	if (strbuf_get(&vars->resolved, id).len > 0 && buf->len > 0) {
		strbuf_app(&vars->resolved, id, STRV(" "));
	}

	if (app) {
		if (strbuf_app(&vars->resolved, id, STRVS(*buf))) {
			log_error("cparse", "make", NULL, "failed to append variable"), ret = 1; // LCOV_EXCL_LINE
		}
	} else {
		if (strbuf_set(&vars->resolved, id, STRVS(*buf))) {
			log_error("cparse", "make", NULL, "failed to set variable"), ret = 1; // LCOV_EXCL_LINE
		}
	}

	return ret;
}

static int make_var_eval(const make_t *make, make_vars_t *vars, const make_var_data_t *var, uint id, lnode_t args, int app, str_t *buf)
{
	int ret = 0;

	buf->len = 0;

	const make_str_data_t *value;
	int first = 1;

	uint i = var->values;
	list_foreach(&make->arrs, i, value)
	{
		if (!first) {
			str_cat(buf, STRV(" "));
		}

		ret |= make_str_expand(make, value, buf);

		first = 0;
	}

	ret |= eval_args(make, vars, args, buf);
	ret |= make_var_app(vars, id, app, buf);

	return ret;
}

static int make_vars_add_var(make_vars_t *vars, strv_t name, int force, uint *index)
{
	if (strbuf_find(&vars->names, name, index) == 0 && !force) {
		return 0;
	}

	if (strbuf_add(&vars->names, name, index) || arr_add(&vars->flags, NULL) == NULL || strbuf_add(&vars->expanded, STRV(""), NULL) ||
	    strbuf_add(&vars->resolved, STRV(""), NULL)) {
		return 1;
	}

	make_var_flags_t *flags = arr_get(&vars->flags, *index);

	*flags = (make_var_flags_t){0};

	return 0;
}

static int make_vars_eval_act(const make_t *make, make_vars_t *vars, make_act_t root, lnode_t args, int def, str_t *buf)
{
	int ret = 0;

	const make_act_data_t *act;
	list_foreach(&make->acts, root, act)
	{
		switch (act->type) {
		case MAKE_ACT_VAR: {
			const make_var_data_t *var = &act->val.var;

			uint id;
			strv_t name = strbuf_get(&make->strs, var->id);

			buf->len = 0;
			str_cat(buf, name);
			ret |= eval_name(make, vars, args, buf);
			if (make_vars_add_var(vars, STRVS(*buf), 0, &id)) {
				ret = 1;
				break;
			}

			make_var_flags_t *flags = arr_get(&vars->flags, id);

			if (var->ext && var->values < make->arrs.cnt) {
				flags->ref = 1;
				ret |= make_var_eval(make, vars, var, id, args, 0, buf);
				flags->ext   = 1;
				flags->local = 0;
			}

			if (flags->ext) {
				continue;
			}

			flags->local = def ? 1 : 0;

			switch (var->type) {
			case MAKE_VAR_INST:
				flags->ref = 0;
				ret |= make_var_eval(make, vars, var, id, args, 0, buf);
				break;
			case MAKE_VAR_APP: ret |= make_var_eval(make, vars, var, id, args, 1, buf); break;
			case MAKE_VAR_REF:
				flags->ref = 1;
				ret |= make_var_eval(make, vars, var, id, args, 0, buf);
				break;
			default: ret = 1; break;
			}
			break;
		}
		case MAKE_ACT_IF: {
			const make_if_data_t *mif = &act->val.mif;

			uint lid, rid;
			if (make_vars_add_var(vars, STRV(".IF.L"), 1, &lid) || make_vars_add_var(vars, STRV(".IF.R"), 1, &rid)) {
				ret = 1;
				break;
			}

			make_var_flags_t *lflags = arr_get(&vars->flags, lid);
			make_var_flags_t *rflags = arr_get(&vars->flags, rid);

			buf->len    = 0;
			lflags->ext = 0;
			lflags->ref = 0;
			ret |= make_str_expand(make, &mif->l, buf);
			ret |= eval_args(make, vars, args, buf);
			ret |= make_var_app(vars, lid, 0, buf);

			buf->len    = 0;
			rflags->ext = 0;
			rflags->ref = 0;
			ret |= make_str_expand(make, &mif->r, buf);
			ret |= eval_args(make, vars, args, buf);
			ret |= make_var_app(vars, rid, 0, buf);

			strv_t l = strbuf_get(&vars->resolved, lid);
			strv_t r = strbuf_get(&vars->resolved, rid);

			if (strv_eq(l, r)) {
				ret |= make_vars_eval_act(make, vars, mif->true_acts, args, def, buf);
			} else {
				ret |= make_vars_eval_act(make, vars, mif->false_acts, args, def, buf);
			}
			break;
		}
		case MAKE_ACT_EVAL_DEF: {
			const make_eval_def_data_t *eval = &act->val.eval_def;
			const make_def_data_t *def	 = make_act_get_type(make, eval->def, MAKE_ACT_DEF);
			if (def == NULL) {
				ret = 1;
				break;
			}
			uint vars_cnt = vars->flags.cnt;
			ret |= make_vars_eval_act(make, vars, def->acts, eval->args, 1, buf);

			make_var_flags_t *flags;
			arr_foreach(&vars->flags, vars_cnt, flags)
			{
				flags->local = 0;
			}

			break;
		}
		case MAKE_ACT_INCLUDE: {
			ret |= make_vars_eval_act(make, vars, act->val.inc.acts, args, def, buf);
			break;
		}
		default: break;
		}
	}

	return ret;
}

int make_vars_eval(const make_t *make, make_vars_t *vars)
{
	if (make == NULL || vars == NULL) {
		return 1;
	}

	strbuf_reset(&vars->names, 0);
	arr_reset(&vars->flags, 0, 0);
	strbuf_reset(&vars->expanded, 0);
	strbuf_reset(&vars->resolved, 0);

	str_t buf = strz(16);
	int ret	  = make_vars_eval_act(make, vars, make->root, MAKE_END, 0, &buf);
	str_free(&buf);
	return ret;
}

strv_t make_vars_get_expanded(const make_vars_t *vars, uint id)
{
	if (vars == NULL) {
		return STRV_NULL;
	}

	return strbuf_get(&vars->expanded, id);
}

strv_t make_vars_get_resolved(const make_vars_t *vars, uint id)
{
	if (vars == NULL) {
		return STRV_NULL;
	}

	make_var_flags_t *flags = arr_get(&vars->flags, id);
	if (flags == NULL) {
		return STRV_NULL;
	}

	if (flags->ref) {
		str_t buf = strz(16);

		strv_t expanded = strbuf_get(&vars->expanded, id);
		str_cat(&buf, expanded);

		replace_vars_priv_t priv = {
			.vars = vars,
		};

		make_replace(&buf, 1, replace_vars, &priv);
		make_vars_t *v = (make_vars_t *)vars;
		strbuf_set(&v->resolved, id, STRVS(buf));

		str_free(&buf);
	}

	return strbuf_get(&vars->resolved, id);
}

size_t make_vars_print(const make_vars_t *vars, dst_t dst)
{
	size_t off = dst.off;

	for (uint i = 0; i < vars->names.off.cnt; i++) {
		strv_t name	= strbuf_get(&vars->names, i);
		strv_t expanded = strbuf_get(&vars->expanded, i);
		strv_t resolved = strbuf_get(&vars->resolved, i);
		dst.off +=
			dputf(dst, "%-16.*s %-64.*s %.*s\n", name.len, name.data, expanded.len, expanded.data, resolved.len, resolved.data);
	}
	return off - dst.off;
}

const char *make_var_type_to_str(make_var_type_t type)
{
	switch (type) {
	case MAKE_VAR_INST: return ":=";
	case MAKE_VAR_REF: return "=";
	// case MAKE_VAR_COND: return "?=";
	case MAKE_VAR_APP: return "+=";
	default: return NULL;
	}
}

static size_t make_rule_target_print(const make_t *make, const make_rule_target_data_t *target, dst_t dst, str_t *buf)
{
	size_t off = dst.off;

	buf->len = 0;
	make_str_expand(make, &target->target, buf);
	dst.off += dputs(dst, STRVS(*buf));
	if (target->action != (uint)-1) {
		dst.off += dputs(dst, strbuf_get(&make->strs, target->action));
	}

	return dst.off - off;
}

static size_t make_acts_print(const make_t *make, make_act_t acts, dst_t dst, int rule, str_t *buf)
{
	size_t off = dst.off;
	const make_act_data_t *act;
	list_foreach(&make->acts, acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_EMPTY: {
			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_VAR: {
			if (act->val.var.ext) {
				break;
			}

			const char *var_type_str = make_var_type_to_str(act->val.var.type);
			if (var_type_str == NULL) {
				break;
			}

			strv_t name = strbuf_get(&make->strs, act->val.var.id);
			dst.off += dputf(dst, "%.*s %s", name.len, name.data, var_type_str);

			const make_str_data_t *value;
			lnode_t i = act->val.var.values;
			list_foreach(&make->arrs, i, value)
			{
				buf->len = 0;
				make_str_expand(make, value, buf);
				dst.off += dputf(dst, " %.*s", buf->len, buf->data);
			}

			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_RULE: {
			dst.off += make_rule_target_print(make, &act->val.rule.target, dst, buf);
			dst.off += dputs(dst, STRV(":"));

			const make_rule_target_data_t *depend;
			lnode_t i = act->val.rule.depends;
			list_foreach(&make->targets, i, depend)
			{
				dst.off += dputs(dst, STRV(" "));
				dst.off += make_rule_target_print(make, depend, dst, buf);
			}

			dst.off += dputs(dst, STRV("\n"));
			dst.off += make_acts_print(make, act->val.rule.acts, dst, 1, buf);
			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_CMD: {
			switch (act->val.cmd.type) {
			case MAKE_CMD_CHILD: {
				if (act->val.cmd.arg2 < make->strs.off.cnt) {
					strv_t arg1 = strbuf_get(&make->strs, act->val.cmd.arg1);
					strv_t arg2 = strbuf_get(&make->strs, act->val.cmd.arg2);
					dst.off += dputf(dst, "\t@$(MAKE) -C %.*s %.*s", arg1.len, arg1.data, arg2.len, arg2.data);

				} else {
					strv_t arg1 = strbuf_get(&make->strs, act->val.cmd.arg1);
					dst.off += dputf(dst, "\t@$(MAKE) -C %.*s", arg1.len, arg1.data);
				}
				break;
			}
			case MAKE_CMD_ERR: {
				strv_t arg1 = strbuf_get(&make->strs, act->val.cmd.arg1);
				dst.off += dputf(dst, "%s$(error %.*s)", rule ? "\t" : "", arg1.len, arg1.data);
				break;
			}
			default: {
				if (rule) {
					dst.off += dputs(dst, STRV("\t"));
				}
				dst.off += dputs(dst, strbuf_get(&make->strs, act->val.cmd.arg1));
				break;
			}
			}
			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_IF: {
			buf->len = 0;
			make_str_expand(make, &act->val.mif.l, buf);
			dst.off += dputf(dst, "ifeq (%.*s", buf->len, buf->data);

			buf->len = 0;
			make_str_expand(make, &act->val.mif.r, buf);
			dst.off += dputf(dst, ",%.*s", buf->len, buf->data);

			dst.off += dputs(dst, STRV(")\n"));

			dst.off += make_acts_print(make, act->val.mif.true_acts, dst, rule, buf);

			if (act->val.mif.false_acts != MAKE_END) {
				dst.off += dputs(dst, STRV("else\n"));
				dst.off += make_acts_print(make, act->val.mif.false_acts, dst, rule, buf);
			}

			dst.off += dputs(dst, STRV("endif\n"));
			break;
		}
		case MAKE_ACT_DEF: {
			strv_t name = strbuf_get(&make->strs, act->val.def.name);
			dst.off += dputf(dst, "define %.*s\n", name.len, name.data);
			dst.off += make_acts_print(make, act->val.def.acts, dst, rule, buf);
			dst.off += dputs(dst, STRV("endef\n"));
			break;
		}
		case MAKE_ACT_EVAL_DEF: {
			make_def_data_t *data = make_act_get_type(make, act->val.eval_def.def, MAKE_ACT_DEF);
			if (data == NULL) {
				break;
			}

			strv_t name = strbuf_get(&make->strs, data->name);
			dst.off += dputf(dst, "$(eval $(call %.*s", name.len, name.data);

			const make_str_data_t *value;
			lnode_t i = act->val.eval_def.args;
			list_foreach(&make->arrs, i, value)
			{
				if (i == act->val.eval_def.args) {
					continue;
				}
				buf->len = 0;
				make_str_expand(make, value, buf);
				dst.off += dputf(dst, ",%.*s", buf->len, buf->data);
			}

			dst.off += dputs(dst, STRV("))\n"));
			break;
		}
		case MAKE_ACT_INCLUDE: {
			strv_t path = strbuf_get(&make->strs, act->val.inc.path);
			dst.off += dputf(dst, "include %.*s\n", path.len, path.data);
			break;
		}
		}
	}

	return dst.off - off;
}

size_t make_inc_print(const make_t *make, make_inc_t inc, dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	make_inc_data_t *data = make_act_get_type(make, inc, MAKE_ACT_INCLUDE);
	if (data == NULL) {
		return 0;
	}

	str_t buf  = strz(16);
	size_t ret = make_acts_print(make, data->acts, dst, 0, &buf);
	str_free(&buf);
	return ret;
}

size_t make_print(const make_t *make, dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	str_t buf  = strz(16);
	size_t ret = make_acts_print(make, make->root, dst, 0, &buf);
	str_free(&buf);

	return ret;
}

size_t make_dbg(const make_t *make, dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	str_t buf = strz(16);

	size_t off = dst.off;
	const make_act_data_t *act;
	uint i = 0;
	list_foreach_all(&make->acts, i, act)
	{
		switch (act->type) {
		case MAKE_ACT_EMPTY: {
			dst.off += dputs(dst, STRV("EMPTY\n"));
			break;
		}
		case MAKE_ACT_VAR: {
			strv_t name = strbuf_get(&make->strs, act->val.var.id);
			dst.off += dputf(dst,
					 "VAR\n"
					 "    NAME    : %.*s%s\n"
					 "    VALUES  :\n",
					 name.len,
					 name.data,
					 act->val.var.ext ? " (ext)" : "");
			const make_str_data_t *value;
			lnode_t i = act->val.var.values;
			list_foreach(&make->arrs, i, value)
			{
				buf.len = 0;
				make_str_expand(make, value, &buf);
				dst.off += dputf(dst, "        %.*s\n", buf.len, buf.data);
			}
			break;
		}
		case MAKE_ACT_RULE: {
			dst.off += dputf(dst,
					 "RULE\n"
					 "    TARGET: ");
			dst.off += make_rule_target_print(make, &act->val.rule.target, dst, &buf);

			dst.off += dputs(dst, STRV("\n    DEPENDS:\n"));
			const make_rule_target_data_t *depend;
			lnode_t i = act->val.rule.depends;
			list_foreach(&make->targets, i, depend)
			{
				dst.off += dputs(dst, STRV("        "));
				dst.off += make_rule_target_print(make, depend, dst, &buf);
				dst.off += dputs(dst, STRV("\n"));
			}
			break;
		}
		case MAKE_ACT_CMD: {
			strv_t arg1 = act->val.cmd.arg1 < make->strs.off.cnt ? strbuf_get(&make->strs, act->val.cmd.arg1) : STRV("");
			strv_t arg2 = act->val.cmd.arg2 < make->strs.off.cnt ? strbuf_get(&make->strs, act->val.cmd.arg2) : STRV("");
			dst.off += dputf(dst,
					 "CMD\n"
					 "    ARG1: %.*s\n"
					 "    ARG2: %.*s\n"
					 "    TYPE: %d\n",
					 arg1.len,
					 arg1.data,
					 arg2.len,
					 arg2.data,
					 act->val.cmd.type);
			break;
		}
		case MAKE_ACT_IF: {
			buf.len = 0;
			make_str_expand(make, &act->val.mif.l, &buf);
			dst.off += dputf(dst,
					 "IF\n"
					 "    L: '%.*s'\n",
					 buf.len,
					 buf.data);
			buf.len = 0;
			make_str_expand(make, &act->val.mif.r, &buf);
			dst.off += dputf(dst, "    R: '%.*s'\n", buf.len, buf.data);
			break;
		}
		case MAKE_ACT_DEF: {
			strv_t name = strbuf_get(&make->strs, act->val.def.name);
			dst.off += dputf(dst,
					 "DEF\n"
					 "    NAME: '%.*s'\n",
					 name.len,
					 name.data);
			break;
		}
		case MAKE_ACT_EVAL_DEF: {
			make_def_data_t *def = make_act_get_type(make, act->val.eval_def.def, MAKE_ACT_DEF);
			if (def == NULL) {
				break;
			}

			strv_t name = strbuf_get(&make->strs, def->name);
			dst.off += dputf(dst,
					 "EVAL DEF\n"
					 "    DEF: '%.*s'\n"
					 "    ARGS:\n",
					 name.len,
					 name.data);
			const make_str_data_t *arg;
			lnode_t i = act->val.eval_def.args;
			list_foreach(&make->arrs, i, arg)
			{
				buf.len = 0;
				make_str_expand(make, arg, &buf);
				dst.off += dputf(dst, "        %.*s\n", buf.len, buf.data);
			}
			break;
		}
		case MAKE_ACT_INCLUDE: {
			strv_t path = strbuf_get(&make->strs, act->val.inc.path);
			dst.off += dputf(dst,
					 "INCLUDE\n"
					 "    PATH: '%.*s'\n",
					 path.len,
					 path.data);
			break;
		}
		}
	}

	str_free(&buf);

	return dst.off - off;
}
