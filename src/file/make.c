#include "file/make.h"

#include "log.h"
#include "platform.h"

#define MAX_VAR_VALUE_LEN 256

typedef struct make_var_data_s {
	uint id;
	make_var_type_t type;
	lnode_t values;
	int ext : 1;
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

typedef enum make_act_type_e {
	MAKE_ACT_EMPTY,
	MAKE_ACT_VAR,
	MAKE_ACT_RULE,
	MAKE_ACT_CMD,
	MAKE_ACT_IF,
	MAKE_ACT_DEF,
	MAKE_ACT_EVAL_DEF,
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
	} val;
} make_act_data_t;

typedef struct make_var_flags_s {
	uint8_t ext : 1;
	uint8_t ref : 1;
} make_var_flags_t;

static inline make_act_data_t *make_act_get(const make_t *make, make_act_t act)
{
	if (make == NULL) {
		return NULL;
	}

	return list_get_data(&make->acts, act);
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
	list_foreach_all(&make->acts, act)
	{
		if (act->type == MAKE_ACT_VAR && act->val.var.id == id) {
			return _i;
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

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

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

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

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

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_RULE,
		.val.rule =
			{
				.target.action = ARR_END,
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

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_CMD,
		.val.cmd =
			{
				.type = cmd.type,
				.arg1 = ARR_END,
				.arg2 = ARR_END,
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

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

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

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

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

	make_str_t str;
	list_add_next_node(&make->arrs, data->args, str);
	make_str_data_t *target = list_get_data(&make->arrs, str);

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

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

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

static make_str_t rule_add_depend(make_t *make, make_rule_t rule, make_rule_target_data_t depend)
{
	make_rule_data_t *data = make_act_get_type(make, rule, MAKE_ACT_RULE);

	if (data == NULL) {
		return MAKE_END;
	}

	make_str_t str;
	list_add_next_node(&make->targets, data->depends, str);
	make_rule_target_data_t *target = list_get_data(&make->targets, str);

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

	return list_set_next_node(&make->acts, make->root, act);
}

make_var_t make_var_add_val(make_t *make, make_var_t var, make_create_str_t val)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_var_data_t *data = make_act_get_type(make, var, MAKE_ACT_VAR);

	if (data == NULL || data->ext) {
		return MAKE_END;
	}

	make_str_t str;
	list_add_next_node(&make->arrs, data->values, str);
	make_str_data_t *target = list_get_data(&make->arrs, str);

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

	make_rule_target_data_t target = {.action = ARR_END};

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
	if (make == NULL) {
		return MAKE_END;
	}

	make_rule_data_t *data = make_act_get_type(make, rule, MAKE_ACT_RULE);

	if (data == NULL || make_act_get(make, act) == NULL) {
		return MAKE_END;
	}

	return list_set_next_node(&make->acts, data->acts, act);
}

static make_act_t make_if_add_act(make_t *make, make_if_t mif, int true_acts, make_act_t act)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_if_data_t *data = make_act_get_type(make, mif, MAKE_ACT_IF);

	if (data == NULL || make_act_get(make, act) == NULL) {
		return MAKE_END;
	}

	make_act_t *acts = true_acts ? &data->true_acts : &data->false_acts;
	return list_set_next_node(&make->acts, *acts, act);
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
	if (make == NULL) {
		return MAKE_END;
	}

	make_def_data_t *data = make_act_get_type(make, def, MAKE_ACT_DEF);

	if (data == NULL || make_act_get(make, act) == NULL) {
		return MAKE_END;
	}

	return list_set_next_node(&make->acts, data->acts, act);
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
	list_add_node(&make->arrs, ((make_var_data_t *)make_act_get_type(make, act, MAKE_ACT_VAR))->values, str);
	make_str_data_t *target = list_get_data(&make->arrs, str);

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

	list_foreach_all(&make->acts, act)
	{
		if (act->type == MAKE_ACT_RULE && make_rule_target_eq(make, act->val.rule.target, target)) {
			return _i;
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
	strbuf_t names;
	strbuf_t vals;
} replace_vars_priv_t;

static int replace_vars(const void *priv, strv_t name, strv_t *val)
{
	const replace_vars_priv_t *p = priv;
	uint index;
	if (strbuf_get_index(&p->names, name, &index)) {
		return 1;
	}

	*val = strbuf_get(&p->vals, index);
	return 0;
}

typedef struct replace_args_priv_s {
	const make_t *make;
	lnode_t args;
} replace_args_priv_t;

static int replace_args(const void *priv, strv_t name, strv_t *val)
{
	const replace_args_priv_t *p = priv;
	uint id;
	if (strv_eq(name, STRV("0"))) {
		id = 0;
	} else if (strv_eq(name, STRV("1"))) {
		id = 1;
	} else if (strv_eq(name, STRV("2"))) {
		id = 2;
	} else if (strv_eq(name, STRV("3"))) {
		id = 3;
	} else {
		id = LIST_END;
	}

	make_str_t index = list_get_at(&p->make->arrs, p->args, id);

	const make_str_data_t *data = list_get_data(&p->make->arrs, index);
	if (data == NULL) {
		return 1;
	}

	*val = strbuf_get(&p->make->strs, data->val.id);
	return 0;
}

typedef int (*replace_fn)(const void *priv, strv_t name, strv_t *val);

static int make_replace(str_t *str, size_t min_len, replace_fn replace, const void *priv)
{
	int ret = 0;

	for (size_t s = 0; str->len >= min_len + 3 && s <= str->len - min_len - 3; s++) {
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
			log_warn("build", "var", NULL, "failed to replace '%.*s' with: '%.*s'", name.len, name.data, val.len, val.data);
			ret = 1;
			continue;
		}
		s--;
	}

	((char *)str->data)[str->len] = '\0';
	return ret;
}

static int eval_args(const make_t *make, strv_t val, lnode_t args, str_t *buf)
{
	buf->len = 0;
	str_cat(buf, val);

	replace_args_priv_t priv = {
		.make = make,
		.args = args,
	};

	return args == MAKE_END ? 0 : make_replace(buf, 1, replace_args, &priv);
}

static int make_str_expand(const make_t *make, const make_str_data_t *str, lnode_t args, str_t *buf)
{
	int ret = 0;
	switch (str->type) {
	case MAKE_STR_VAR: {
		make_var_data_t *data = make_act_get_type(make, str->val.var, MAKE_ACT_VAR);
		if (data == NULL) {
			log_warn("cparse", "make", NULL, "variable not found");
			return 1;
		}
		str_cat(buf, STRV("$("));
		str_cat(buf, strbuf_get(&make->strs, data->id));
		str_cat(buf, STRV(")"));
		break;
	}
	default: {
		char b[256] = {0};
		str_t tmp   = strb(b, sizeof(b), 0);
		ret |= eval_args(make, strbuf_get(&make->strs, str->val.id), args, &tmp);
		str_cat(buf, STRV_STR(tmp));
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
		strbuf_app(&vars->expanded, STRV(" "), id);
	}

	if (app) {
		if (strbuf_app(&vars->expanded, STRV_STR(*buf), id)) {
			log_error("cparse", "make", NULL, "failed to append variable");
			ret = 1;
		}
	} else {
		if (strbuf_set(&vars->expanded, STRV_STR(*buf), id)) {
			log_error("cparse", "make", NULL, "failed to set variable");
			ret = 1;
		}
	}

	if (!flags->ref) {
		replace_vars_priv_t priv = {
			.names = vars->names,
			.vals  = vars->resolved,
		};

		ret |= make_replace(buf, 1, replace_vars, &priv);
	}

	if (strbuf_get(&vars->resolved, id).len > 0 && buf->len > 0) {
		strbuf_app(&vars->resolved, STRV(" "), id);
	}

	if (app) {
		if (strbuf_app(&vars->resolved, STRV_STR(*buf), id)) {
			log_error("cparse", "make", NULL, "failed to append variable");
			ret = 1;
		}
	} else {
		if (strbuf_set(&vars->resolved, STRV_STR(*buf), id)) {
			log_error("cparse", "make", NULL, "failed to set variable");
			ret = 1;
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
	list_foreach(&make->arrs, var->values, value)
	{
		if (!first) {
			str_cat(buf, STRV(" "));
		}

		ret |= make_str_expand(make, value, args, buf);

		first = 0;
	}

	ret |= make_var_app(vars, id, app, buf);

	return ret;
}

static int make_vars_add_var(make_vars_t *vars, strv_t name, int force, uint *index)
{
	if (strbuf_get_index(&vars->names, name, index) == 0 && !force) {
		return 0;
	}

	if (strbuf_add(&vars->names, name, index) || arr_add(&vars->flags) == NULL || strbuf_add(&vars->expanded, STRV(""), NULL) ||
	    strbuf_add(&vars->resolved, STRV(""), NULL)) {
		return 1;
	}

	make_var_flags_t *flags = arr_get(&vars->flags, *index);

	*flags = (make_var_flags_t){0};

	return 0;
}

static int make_vars_eval_act(const make_t *make, make_vars_t *vars, make_act_t root, str_t *buf, lnode_t args)
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
			ret |= eval_args(make, name, args, buf);
			if (make_vars_add_var(vars, STRV_STR(*buf), 0, &id)) {
				ret = 1;
				break;
			}

			make_var_flags_t *flags = arr_get(&vars->flags, id);

			if (var->ext && var->values < make->arrs.cnt) {
				flags->ref = 1;
				ret |= make_var_eval(make, vars, var, id, args, 0, buf);
				flags->ext = 1;
			}

			if (flags->ext) {
				continue;
			}

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
			ret |= make_str_expand(make, &mif->l, args, buf);
			ret |= make_var_app(vars, lid, 0, buf);

			buf->len    = 0;
			rflags->ext = 0;
			rflags->ref = 0;
			ret |= make_str_expand(make, &mif->r, args, buf);
			ret |= make_var_app(vars, rid, 0, buf);

			strv_t l = strbuf_get(&vars->resolved, lid);
			strv_t r = strbuf_get(&vars->resolved, rid);

			if (strv_eq(l, r)) {
				ret |= make_vars_eval_act(make, vars, mif->true_acts, buf, args);
			} else {
				ret |= make_vars_eval_act(make, vars, mif->false_acts, buf, args);
			}
			break;
		}
		case MAKE_ACT_EVAL_DEF: {
			const make_eval_def_data_t *eval = &act->val.eval_def;

			const make_def_data_t *def = make_act_get_type(make, eval->def, MAKE_ACT_DEF);
			if (def == NULL) {
				ret = 1;
				break;
			}
			ret |= make_vars_eval_act(make, vars, def->acts, buf, eval->args);
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

	char buf[256] = {0};
	str_t buf_str = strb(buf, sizeof(buf), 0);

	return make_vars_eval_act(make, vars, make->root, &buf_str, MAKE_END);
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
		char buf[256] = {0};
		str_t strbuf  = strb(buf, sizeof(buf), 0);

		strv_t expanded = strbuf_get(&vars->expanded, id);
		str_cat(&strbuf, expanded);

		replace_vars_priv_t priv = {
			.names = vars->names,
			.vals  = vars->resolved,
		};

		make_replace(&strbuf, 1, replace_vars, &priv);
		make_vars_t *v = (make_vars_t *)vars;
		strbuf_set(&v->resolved, STRV_STR(strbuf), id);
	}

	return strbuf_get(&vars->resolved, id);
}

int make_vars_print(const make_vars_t *vars, print_dst_t dst)
{
	int off = dst.off;

	for (uint i = 0; i < vars->names.off.cnt; i++) {
		strv_t name	= strbuf_get(&vars->names, i);
		strv_t expanded = strbuf_get(&vars->expanded, i);
		strv_t resolved = strbuf_get(&vars->resolved, i);
		dst.off += c_dprintf(
			dst, "%-16.*s %-64.*s %.*s\n", name.len, name.data, expanded.len, expanded.data, resolved.len, resolved.data);
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

static int make_rule_target_print(const make_t *make, const make_rule_target_data_t *target, print_dst_t dst, str_t *buf)
{
	int off = dst.off;

	buf->len = 0;
	make_str_expand(make, &target->target, MAKE_END, buf);
	dst.off += c_dprintf(dst, "%.*s", buf->len, buf->data);
	if (target->action != ARR_END) {
		strv_t action = strbuf_get(&make->strs, target->action);
		dst.off += c_dprintf(dst, "%.*s", action.len, action.data);
	}

	return dst.off - off;
}

static int make_acts_print(const make_t *make, make_act_t acts, print_dst_t dst, int rule, str_t *buf)
{
	int off = dst.off;
	const make_act_data_t *act;
	list_foreach(&make->acts, acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_EMPTY: {
			dst.off += c_dprintf(dst, "\n");
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
			dst.off += c_dprintf(dst, "%.*s %s", name.len, name.data, var_type_str);

			const make_str_data_t *value;
			list_foreach(&make->arrs, act->val.var.values, value)
			{
				buf->len = 0;
				make_str_expand(make, value, MAKE_END, buf);
				dst.off += c_dprintf(dst, " %.*s", buf->len, buf->data);
			}

			dst.off += c_dprintf(dst, "\n");
			break;
		}
		case MAKE_ACT_RULE: {
			dst.off += make_rule_target_print(make, &act->val.rule.target, dst, buf);
			dst.off += c_dprintf(dst, ":");

			const make_rule_target_data_t *depend;
			list_foreach(&make->targets, act->val.rule.depends, depend)
			{
				dst.off += c_dprintf(dst, " ");
				dst.off += make_rule_target_print(make, depend, dst, buf);
			}

			dst.off += c_dprintf(dst, "\n");
			dst.off += make_acts_print(make, act->val.rule.acts, dst, 1, buf);
			dst.off += c_dprintf(dst, "\n");
			break;
		}
		case MAKE_ACT_CMD: {
			switch (act->val.cmd.type) {
			case MAKE_CMD_CHILD: {
				if (act->val.cmd.arg2 < make->strs.off.cnt) {
					strv_t arg1 = strbuf_get(&make->strs, act->val.cmd.arg1);
					strv_t arg2 = strbuf_get(&make->strs, act->val.cmd.arg2);
					dst.off += c_dprintf(dst, "\t@$(MAKE) -C %.*s %.*s\n", arg1.len, arg1.data, arg2.len, arg2.data);

				} else {
					strv_t arg1 = strbuf_get(&make->strs, act->val.cmd.arg1);
					dst.off += c_dprintf(dst, "\t@$(MAKE) -C %.*s\n", arg1.len, arg1.data);
				}
				break;
			}
			case MAKE_CMD_ERR: {
				strv_t arg1 = strbuf_get(&make->strs, act->val.cmd.arg1);
				dst.off += c_dprintf(dst, "%s$(error %.*s)\n", rule ? "\t" : "", arg1.len, arg1.data);
				break;
			}
			default: {
				strv_t arg1 = strbuf_get(&make->strs, act->val.cmd.arg1);
				dst.off += c_dprintf(dst, "%s%.*s\n", rule ? "\t" : "", arg1.len, arg1.data);
				break;
			}
			}
			break;
		}
		case MAKE_ACT_IF: {
			buf->len = 0;
			make_str_expand(make, &act->val.mif.l, MAKE_END, buf);
			dst.off += c_dprintf(dst, "ifeq (%.*s", buf->len, buf->data);

			buf->len = 0;
			make_str_expand(make, &act->val.mif.r, MAKE_END, buf);
			dst.off += c_dprintf(dst, ",%.*s", buf->len, buf->data);

			dst.off += c_dprintf(dst, ")\n");

			dst.off += make_acts_print(make, act->val.mif.true_acts, dst, rule, buf);

			if (act->val.mif.false_acts != MAKE_END) {
				dst.off += c_dprintf(dst, "else\n");
				dst.off += make_acts_print(make, act->val.mif.false_acts, dst, rule, buf);
			}

			dst.off += c_dprintf(dst, "endif\n");
			break;
		}
		case MAKE_ACT_DEF: {
			strv_t name = strbuf_get(&make->strs, act->val.def.name);
			dst.off += c_dprintf(dst, "define %.*s\n", name.len, name.data);
			dst.off += make_acts_print(make, act->val.def.acts, dst, rule, buf);
			dst.off += c_dprintf(dst, "endef\n");
			break;
		}
		case MAKE_ACT_EVAL_DEF: {
			make_def_data_t *def = make_act_get_type(make, act->val.eval_def.def, MAKE_ACT_DEF);
			if (def == NULL) {
				break;
			}

			strv_t name = strbuf_get(&make->strs, def->name);
			dst.off += c_dprintf(dst, "$(eval $(call %.*s", name.len, name.data);

			const make_str_data_t *value;
			list_foreach(&make->arrs, act->val.eval_def.args, value)
			{
				if (_i == act->val.eval_def.args) {
					continue;
				}
				buf->len = 0;
				make_str_expand(make, value, MAKE_END, buf);
				dst.off += c_dprintf(dst, ",%.*s", buf->len, buf->data);
			}

			dst.off += c_dprintf(dst, "))\n");
			break;
		}
		}
	}

	return dst.off - off;
}

int make_print(const make_t *make, print_dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	char buf[256] = {0};
	str_t buf_str = strb(buf, sizeof(buf), 0);

	return make_acts_print(make, make->root, dst, 0, &buf_str);
}

int make_dbg(const make_t *make, print_dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	char buff[256] = {0};
	str_t buf      = strb(buff, sizeof(buff), 0);

	int off = dst.off;
	const make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_EMPTY: {
			dst.off += c_dprintf(dst, "EMPTY\n");
			break;
		}
		case MAKE_ACT_VAR: {
			strv_t name = strbuf_get(&make->strs, act->val.var.id);
			dst.off += c_dprintf(dst,
					     "VAR\n"
					     "    NAME    : %.*s %s\n"
					     "    VALUES  :\n",
					     name.len,
					     name.data,
					     act->val.var.ext ? "(ext)" : "");
			const make_str_data_t *value;
			list_foreach(&make->arrs, act->val.var.values, value)
			{
				buf.len = 0;
				make_str_expand(make, value, MAKE_END, &buf);
				dst.off += c_dprintf(dst, "        %.*s\n", buf.len, buf.data);
			}
			break;
		}
		case MAKE_ACT_RULE: {
			dst.off += c_dprintf(dst,
					     "RULE\n"
					     "    TARGET: ");
			dst.off += make_rule_target_print(make, &act->val.rule.target, dst, &buf);

			dst.off += c_dprintf(dst, "\n    DEPENDS:\n");
			const make_rule_target_data_t *depend;
			list_foreach(&make->targets, act->val.rule.depends, depend)
			{
				dst.off += c_dprintf(dst, "        ");
				dst.off += make_rule_target_print(make, depend, dst, &buf);
				dst.off += c_dprintf(dst, "\n");
			}
			break;
		}
		case MAKE_ACT_CMD: {
			strv_t arg1 = act->val.cmd.arg1 < make->strs.off.cnt ? strbuf_get(&make->strs, act->val.cmd.arg1) : STRV("");
			strv_t arg2 = act->val.cmd.arg2 < make->strs.off.cnt ? strbuf_get(&make->strs, act->val.cmd.arg2) : STRV("");
			dst.off += c_dprintf(dst,
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
			make_str_expand(make, &act->val.mif.l, MAKE_END, &buf);
			dst.off += c_dprintf(dst,
					     "IF\n"
					     "    L: '%.*s'\n",
					     buf.len,
					     buf.data);
			buf.len = 0;
			make_str_expand(make, &act->val.mif.r, MAKE_END, &buf);
			dst.off += c_dprintf(dst, "    R: '%.*s'\n", buf.len, buf.data);
			break;
		}
		case MAKE_ACT_DEF: {
			strv_t name = strbuf_get(&make->strs, act->val.def.name);
			dst.off += c_dprintf(dst,
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
			dst.off += c_dprintf(dst,
					     "EVAL DEF\n"
					     "    DEF: '%.*s'\n"
					     "    ARGS:\n",
					     name.len,
					     name.data);
			const make_str_data_t *arg;
			list_foreach(&make->arrs, act->val.eval_def.args, arg)
			{
				buf.len = 0;
				make_str_expand(make, arg, MAKE_END, &buf);
				dst.off += c_dprintf(dst, "        %.*s\n", buf.len, buf.data);
			}
			break;
		}
		}
	}

	return dst.off - off;
}
