#include "file/make.h"
#include "file/make_priv.h"

#include "log.h"
#include "platform.h"

static inline void *make_act_get_type(const make_t *make, make_act_t act, make_act_type_t type)
{
	make_act_data_t *data = list_get(&make->acts, act);
	if (data == NULL) {
		log_error("cutils", "make", NULL, "failed to get action: %d", act);
		return NULL;
	}

	if (data->type != type) {
		log_error("cutils", "make", NULL, "failed to get action: %d: expected type: %d, got: %d", act, type, data->type);
		return NULL;
	}

	return &data->val;
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

	if (strvbuf_init(&make->strs, strs_cap, 8, alloc) == NULL) {
		return NULL;
	}

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
	strvbuf_free(&make->strs);
}

int make_empty(make_t *make, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	make_act_data_t *data = list_node(&make->acts, act);
	if (data == NULL) {
		return 1;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_EMPTY,
	};

	return 0;
}

static int make_var_ex(make_t *make, size_t name, make_var_type_t type, int ext, make_act_t *act)
{
	make_act_data_t *data = list_node(&make->acts, act);
	if (data == NULL) {
		return 1;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_VAR,
		.val.var =
			{
				.name = name,
				.type = type,
				.ext  = ext,
				.def  = 0,
			},
	};

	return 0;
}

static int add_str(make_t *make, strv_t str, size_t *off)
{
	buf_reset(&make->strs, make->strs_used);

	if (strvbuf_add(&make->strs, str, off)) {
		return 1;
	}

	make->strs_used = make->strs.used;
	return 0;
}

int make_var(make_t *make, strv_t name, make_var_type_t type, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	size_t off;
	if (add_str(make, name, &off)) {
		return 1;
	}

	return make_var_ex(make, off, type, 0, act);
}

int make_var_var(make_t *make, make_act_t var, make_var_type_t type, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	const make_act_data_t *data = list_get(&make->acts, var);
	if (data == NULL) {
		return 1;
	}

	return make_var_ex(make, data->val.var.name, type, 0, act);
}

int make_var_ext(make_t *make, strv_t name, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	size_t off;
	if (add_str(make, name, &off)) {
		return 1;
	}

	return make_var_ex(make, off, MAKE_VAR_REF, 1, act);
}

static int create_str(make_t *make, make_create_str_t str, make_str_data_t *data)
{
	if (str.type != MAKE_STR_STR) {
		*data = (make_str_data_t){.type = str.type, .val.var = str.val.var};
		return 0;
	}

	if (add_str(make, str.val.str, &data->val.val)) {
		return 1;
	}

	data->type = str.type;

	return 0;
}

int make_rule(make_t *make, make_create_rule_t target, int file, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	make_act_data_t *data = list_node(&make->acts, act);
	if (data == NULL) {
		return 1;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_RULE,
		.val.rule =
			{
				.file = file,
			},
	};

	if (create_str(make, target.target, &data->val.rule.target.target)) {
		return 1;
	}

	if (target.action.data) {
		if (add_str(make, target.action, &data->val.rule.target.action)) {
			return 1;
		}
		data->val.rule.target.has_action = 1;
	}

	return 0;
}

int make_phony(make_t *make, make_act_t *act)
{
	return make_rule(make, MRULE(MSTR(STRV(".PHONY"))), 1, act);
}

int make_cmd(make_t *make, make_create_cmd_t cmd, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	make_act_data_t *data = list_node(&make->acts, act);
	if (data == NULL) {
		return 1;
	}

	*data = (make_act_data_t){
		.type	 = MAKE_ACT_CMD,
		.val.cmd = {.type = cmd.type},
	};

	if (cmd.arg1.data) {
		if (add_str(make, cmd.arg1, &data->val.cmd.arg[0])) {
			return 1;
		}
		data->val.cmd.args = 1;
	}

	if (cmd.arg2.data) {
		if (add_str(make, cmd.arg2, &data->val.cmd.arg[1])) {
			return 1;
		}
		data->val.cmd.args = 2;
	}

	return 0;
}

int make_if(make_t *make, make_create_str_t l, make_create_str_t r, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	make_act_data_t *data = list_node(&make->acts, act);
	if (data == NULL) {
		return 1;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_IF,
	};

	if (create_str(make, l, &data->val.mif.l)) {
		return 1;
	}

	if (create_str(make, r, &data->val.mif.r)) {
		return 1;
	}

	return 0;
}

int make_def(make_t *make, strv_t name, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	make_act_data_t *data = list_node(&make->acts, act);
	if (data == NULL) {
		return 1;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_DEF,
	};

	if (add_str(make, name, &data->val.def.name)) {
		return 1;
	}

	return 0;
}

int make_eval_def(make_t *make, make_act_t def, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	make_def_data_t *def_data = make_act_get_type(make, def, MAKE_ACT_DEF);
	if (def_data == NULL) {
		return 1;
	}
	size_t name = def_data->name;

	uint acts_cnt = make->acts.cnt;

	make_act_t tmp;
	make_act_data_t *data = list_node(&make->acts, &tmp);
	if (data == NULL) {
		return 1;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_EVAL_DEF,
		.val.eval_def =
			{
				.def = def,
			},
	};

	make_str_data_t *arg = list_node(&make->arrs, &data->val.eval_def.args);
	if (arg == NULL) {
		list_reset(&make->acts, acts_cnt);
		return 1;
	}

	*arg = (make_str_data_t){.type = MAKE_STR_STR, .val.val = name};

	if (act) {
		*act = tmp;
	}

	return 0;
}

int make_inc(make_t *make, strv_t path, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	make_act_data_t *data = list_node(&make->acts, act);
	if (data == NULL) {
		return 1;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_INCLUDE,
	};

	if (add_str(make, path, &data->val.inc.path)) {
		return 1;
	}

	return 0;
}

static int rule_add_depend(make_t *make, make_act_t rule, make_rule_target_data_t depend)
{
	make_rule_data_t *data = make_act_get_type(make, rule, MAKE_ACT_RULE);

	if (data == NULL) {
		return 1;
	}

	make_rule_target_data_t *target;
	if (data->has_depends) {
		make_act_t str;
		target = list_node(&make->targets, &str);
		if (target == NULL) {
			return 1;
		}
		list_app(&make->targets, data->depends, str);
	} else {
		target = list_node(&make->targets, &data->depends);
		if (target == NULL) {
			return 1;
		}
		data->has_depends = 1;
	}

	*target = depend;

	return 0;
}

int make_add_act(make_t *make, make_act_t act, make_act_t next)
{
	if (make == NULL) {
		return 1;
	}

	make_act_data_t *data = list_get(&make->acts, next);
	if (data == NULL) {
		log_error("cutils", "make", NULL, "failed to add action: %d", next);
		return 1;
	}

	if (data->type == MAKE_ACT_RULE && !data->val.rule.file) {
		make_act_t phony;
		if (make_rule_get_target(make, MRULE(MSTR(STRV(".PHONY"))), &phony)) {
			make_phony(make, &phony);
			make_add_act(make, act, phony);
		}
		rule_add_depend(make, phony, data->val.rule.target);
	}

	return list_app(&make->acts, act, next);
}

int make_var_add_val(make_t *make, make_act_t var, make_create_str_t val)
{
	if (make == NULL) {
		return 1;
	}

	make_var_data_t *data = make_act_get_type(make, var, MAKE_ACT_VAR);
	if (data == NULL) {
		log_error("cutils", "make", NULL, "failed to add value to the variable: %d", var);
		return 1;
	}

	if (data->ext) {
		log_error("cutils", "make", NULL, "variable is external: %d", var);
		return 1;
	}

	make_str_data_t *target;
	if (data->has_values) {
		list_node_t value;
		target = list_node(&make->arrs, &value);
		if (target == NULL) {
			return 1;
		}
		list_app(&make->arrs, data->values, value);
	} else {
		target = list_node(&make->arrs, &data->values);
		if (target == NULL) {
			return 1;
		}
		data->has_values = 1;
	}

	if (create_str(make, val, target)) {
		return 1;
	}

	return 0;
}

int make_rule_add_depend(make_t *make, make_act_t rule, make_create_rule_t depend)
{
	if (make == NULL) {
		return 1;
	}

	make_rule_target_data_t target = {0};

	if (create_str(make, depend.target, &target.target)) {
		return 1;
	}

	if (depend.action.data) {
		if (add_str(make, depend.action, &target.action)) {
			return 1;
		}
		target.has_action = 1;
	}

	return rule_add_depend(make, rule, target);
}

int make_rule_add_act(make_t *make, make_act_t rule, make_act_t act)
{
	if (make == NULL) {
		return 1;
	}

	make_rule_data_t *data = make_act_get_type(make, rule, MAKE_ACT_RULE);
	if (data == NULL) {
		log_error("cutils", "make", NULL, "failed to add action to the rule: %d", rule);
		return 1;
	}

	if (list_get(&make->acts, act) == NULL) {
		log_error("cutils", "make", NULL, "failed to add action: %d", act);
		return 1;
	}

	if (data->has_acts) {
		list_app(&make->acts, data->acts, act);
	} else {
		data->acts     = act;
		data->has_acts = 1;
	}

	return 0;
}

static int make_if_add_act(make_t *make, make_act_t mif, int true_acts, make_act_t act)
{
	if (make == NULL) {
		return 1;
	}

	make_if_data_t *data = make_act_get_type(make, mif, MAKE_ACT_IF);
	if (data == NULL) {
		log_error("cutils", "make", NULL, "failed to add %s action to if: %d", true_acts ? "true" : "false", mif);
		return 1;
	}

	if (list_get(&make->acts, act) == NULL) {
		log_error("cutils", "make", NULL, "failed to add action: %d", act);
		return 1;
	}

	byte has_acts	 = true_acts ? data->has_true_acts : data->has_false_acts;
	make_act_t *acts = true_acts ? &data->true_acts : &data->false_acts;

	if (has_acts) {
		list_app(&make->acts, *acts, act);
	} else {
		*acts = act;
		if (true_acts) {
			data->has_true_acts = 1;
		} else {
			data->has_false_acts = 1;
		}
	}

	return 0;
}

int make_if_add_true_act(make_t *make, make_act_t mif, make_act_t act)
{
	return make_if_add_act(make, mif, 1, act);
}

int make_if_add_false_act(make_t *make, make_act_t mif, make_act_t act)
{
	return make_if_add_act(make, mif, 0, act);
}

int make_def_add_act(make_t *make, make_act_t def, make_act_t act)
{
	if (make == NULL) {
		return 1;
	}

	make_def_data_t *def_data = make_act_get_type(make, def, MAKE_ACT_DEF);
	if (def_data == NULL) {
		log_error("cutils", "make", NULL, "failed to add action to def: %d", def);
		return 1;
	}

	make_act_data_t *act_data = list_get(&make->acts, act);
	if (act_data == NULL) {
		log_error("cutils", "make", NULL, "failed to add action: %d", act);
		return 1;
	}

	if (act_data->type == MAKE_ACT_VAR) {
		act_data->val.var.def = -1;
	}

	if (def_data->has_acts) {
		list_app(&make->acts, def_data->acts, act);
	} else {
		def_data->acts	   = act;
		def_data->has_acts = 1;
	}

	return 0;
}

int make_eval_def_add_arg(make_t *make, make_act_t def, make_create_str_t arg)
{
	if (make == NULL) {
		return 1;
	}

	make_eval_def_data_t *data = make_act_get_type(make, def, MAKE_ACT_EVAL_DEF);
	if (data == NULL) {
		return 1;
	}

	list_node_t a;
	make_str_data_t *str = list_node(&make->arrs, &a);
	if (str == NULL) {
		return 1;
	}

	if (create_str(make, arg, str)) {
		return 1;
	}

	list_app(&make->arrs, data->args, a);

	return 0;
}

int make_inc_add_act(make_t *make, make_act_t inc, make_act_t act)
{
	if (make == NULL) {
		return 1;
	}

	make_inc_data_t *data = make_act_get_type(make, inc, MAKE_ACT_INCLUDE);
	if (data == NULL) {
		log_error("cutils", "make", NULL, "failed to add action to include: %d", inc);
		return 1;
	}

	if (list_get(&make->acts, act) == NULL) {
		log_error("cutils", "make", NULL, "failed to add action: %d", act);
		return 1;
	}

	if (data->has_acts) {
		list_app(&make->acts, data->acts, act);
	} else {
		data->acts     = act;
		data->has_acts = 1;
	}

	return 0;
}

int make_ext_set_val(make_t *make, make_act_t var, make_create_str_t val)
{
	if (make == NULL) {
		return 1;
	}

	make_act_data_t *data = list_get(&make->acts, var);
	if (data == NULL) {
		log_error("cutils", "make", NULL, "failed to set external value: %d", var);
		return 1;
	}

	if (!data->val.var.ext) {
		log_error("cutils", "make", NULL, "variable is not external: %d", var);
		return 1;
	}

	make_str_data_t *target;

	make_var_data_t *var_data = make_act_get_type(make, var, MAKE_ACT_VAR);
	if (var_data->has_values) {
		target = list_get(&make->arrs, var_data->values);
	} else {
		target = list_node(&make->arrs, &var_data->values);
	}

	if (target == NULL) {
		return 1;
	}

	var_data->has_values = 1;

	if (create_str(make, val, target)) {
		return 1;
	}

	return 0;
}

static int make_rule_target_eq(const make_t *make, make_rule_target_data_t target1, make_create_rule_t target2)
{
	if (target1.target.type != target2.target.type) {
		return 0;
	}

	if (!strv_eq(target1.has_action ? strvbuf_get(&make->strs, target1.action) : STRV_NULL, target2.action)) {
		return 0;
	}

	switch (target1.target.type) {
	case MAKE_STR_VAR:
		return ((make_var_data_t *)make_act_get_type(make, target1.target.val.var, MAKE_ACT_VAR))->name ==
		       ((make_var_data_t *)make_act_get_type(make, target2.target.val.var, MAKE_ACT_VAR))->name;
	default: return strv_eq(strvbuf_get(&make->strs, target1.target.val.val), target2.target.val.str);
	}
}

int make_rule_get_target(const make_t *make, make_create_rule_t target, make_act_t *act)
{
	if (make == NULL) {
		return 1;
	}

	make_act_t i = 0;
	const make_act_data_t *data;
	list_foreach_all(&make->acts, i, data)
	{
		if (data->type == MAKE_ACT_RULE && make_rule_target_eq(make, data->val.rule.target, target)) {
			if (act) {
				*act = i;
			}
			return 0;
		}
	}

	return 1;
}

static make_var_data_t *vars_find_name(const make_t *make, strv_t name)
{
	uint i = 0;
	make_act_data_t *act;
	list_foreach_all(&make->acts, i, act)
	{
		if (strv_eq(strvbuf_get(&make->strs, act->val.var.name), name)) {
			return &act->val.var;
		}
	}
	return NULL;
}

typedef struct replace_vars_priv_s {
	const make_t *make;
	int resolve;
} replace_vars_priv_t;

static int replace_vars(const void *priv, strv_t name, strv_t *val)
{
	const replace_vars_priv_t *p = priv;

	const make_var_data_t *var = vars_find_name(p->make, name);
	if (var == NULL) {
		return 1;
	}

	if (!p->resolve && var->scope) {
		return 1;
	}

	*val = strvbuf_get(&p->make->strs, var->resolved);
	return 0;
}

typedef struct replace_args_priv_s {
	const make_t *make;
	list_node_t args;
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
			make_act_t index;
			const make_str_data_t *data = list_get_at(&p->make->arrs, p->args, id, &index);
			if (data == NULL) {
				return 1;
			}

			*val = strvbuf_get(&p->make->strs, data->val.val);
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

static int eval_args(const make_t *make, list_node_t args, int has_args, str_t *buf)
{
	replace_vars_priv_t vars_priv = (replace_vars_priv_t){
		.make	 = make,
		.resolve = 0,
	};

	replace_args_priv_t args_priv = {
		.make = make,
		.args = args,
		.vars = &vars_priv,
	};

	return has_args ? make_replace(buf, 1, replace_args, &args_priv) : 0;
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
		str_cat(buf, strvbuf_get(&make->strs, data->name));
		str_cat(buf, STRV(")"));
		break;
	}
	default: {
		str_cat(buf, strvbuf_get(&make->strs, str->val.val));
		break;
	}
	}

	return ret;
}

static void update_strs(make_t *make, make_var_data_t *var, size_t off, int diff)
{
	if (var->expanded > off) {
		var->expanded += diff; // LCOV_EXCL_LINE
	}
	if (var->resolved > off) {
		var->resolved += diff;
	}

	uint i = 0;
	make_act_data_t *act;
	list_foreach_all(&make->acts, i, act)
	{
		switch (act->type) {
		case MAKE_ACT_VAR:
			if (act->val.var.expanded > off) {
				act->val.var.expanded += diff;
			}
			if (act->val.var.resolved > off) {
				act->val.var.resolved += diff;
			}
			break;
		case MAKE_ACT_IF:
			if (act->val.mif.lexpanded > off) {
				act->val.mif.lexpanded += diff;
			}
			if (act->val.mif.lresolved > off) {
				act->val.mif.lresolved += diff;
			}
			if (act->val.mif.rexpanded > off) {
				act->val.mif.rexpanded += diff;
			}
			if (act->val.mif.rresolved > off) {
				act->val.mif.rresolved += diff;
			}
			break;
		default: break;
		}
	}
}

static int vars_app(make_t *make, make_var_data_t *var, size_t off, strv_t strv)
{
	if (strvbuf_app(&make->strs, off, strv)) {
		log_error("cutils", "make", NULL, "failed to append variable");
		return 1;
	}

	update_strs(make, var, off, (int)strv.len);

	return 0;
}

static int vars_set(make_t *make, make_var_data_t *var, size_t off, strv_t strv)
{
	int diff = (int)strv.len - (int)strvbuf_get(&make->strs, off).len;

	if (strvbuf_set(&make->strs, off, strv)) {
		log_error("cutils", "make", NULL, "failed to set variable");
		return 1;
	}

	update_strs(make, var, off, diff);

	return 0;
}

static int make_var_app(make_t *make, make_var_data_t *var, int app, str_t *buf)
{
	int ret = 0;

	if (strvbuf_get(&make->strs, var->expanded).len > 0 && buf->len > 0) {
		vars_app(make, var, var->expanded, STRV(" "));
	}

	if (app) {
		if (vars_app(make, var, var->expanded, STRVS(*buf))) {
			log_error("cparse", "make", NULL, "failed to append variable"), ret = 1; // LCOV_EXCL_LINE
		}
	} else {
		if (vars_set(make, var, var->expanded, STRVS(*buf))) {
			log_error("cparse", "make", NULL, "failed to set variable"), ret = 1; // LCOV_EXCL_LINE
		}
	}

	if (!var->flags.ref) {
		replace_vars_priv_t priv = {
			.make	 = make,
			.resolve = 1,
		};

		ret |= make_replace(buf, 1, replace_vars, &priv);
	}

	if (strvbuf_get(&make->strs, var->resolved).len > 0 && buf->len > 0) {
		vars_app(make, var, var->resolved, STRV(" "));
	}

	if (app) {
		if (vars_app(make, var, var->resolved, STRVS(*buf))) {
			log_error("cparse", "make", NULL, "failed to append variable"), ret = 1; // LCOV_EXCL_LINE
		}
	} else {
		if (vars_set(make, var, var->resolved, STRVS(*buf))) {
			log_error("cparse", "make", NULL, "failed to set variable"), ret = 1; // LCOV_EXCL_LINE
		}
	}

	return ret;
}

static int make_var_eval(make_t *make, make_act_data_t *act, make_var_data_t *var, list_node_t args, int has_args, int app, str_t *buf)
{
	int ret = 0;

	buf->len = 0;

	if (act->val.var.has_values) {
		const make_str_data_t *value;
		int first = 1;
		uint i	  = act->val.var.values;
		list_foreach(&make->arrs, i, value)
		{
			if (!first) {
				str_cat(buf, STRV(" "));
			}

			ret |= make_str_expand(make, value, buf);

			first = 0;
		}
	}

	ret |= eval_args(make, args, has_args, buf);
	ret |= make_var_app(make, var, app, buf);

	return ret;
}

static make_var_data_t *vars_find(const make_t *make, size_t name)
{
	make_var_data_t *ret = NULL;

	uint i = 0;
	make_act_data_t *act;
	list_foreach_all(&make->acts, i, act)
	{
		if (act->type != MAKE_ACT_VAR) {
			continue;
		}

		if (act->val.var.name != name) {
			continue;
		}

		ret = &act->val.var;
		break;
	}

	return ret;
}

static int make_vars_eval_act(make_t *make, make_act_t root, list_node_t args, int has_args, int def, int scope, str_t *buf)
{
	int ret = 0;

	make_act_t i = root;
	make_act_data_t *act;
	int first = 1;
	list_foreach(&make->acts, i, act)
	{
		if (i == root && !first) {
			log_error("cparse", "make", NULL, "loop detected: %d", i);
			ret = 1;
			break;
		}

		switch (act->type) {
		case MAKE_ACT_VAR: {
			make_var_data_t *var = vars_find(make, act->val.var.name);

			if (act->val.var.ext && act->val.var.has_values) {
				var->flags.ref	  = 1;
				make_var_data_t v = *var;
				ret |= make_var_eval(make, act, &v, args, has_args, 0, buf);
				var->flags.ext = 1;
				var->scope     = 0;
			}

			if (var->flags.ext) {
				continue;
			}

			var->scope = def ? scope : 0;

			switch (act->val.var.type) {
			case MAKE_VAR_INST: {
				var->flags.ref	  = 0;
				make_var_data_t v = *var;
				ret |= make_var_eval(make, act, &v, args, has_args, 0, buf);
				break;
			}
			case MAKE_VAR_APP: {
				make_var_data_t v = *var;
				ret |= make_var_eval(make, act, &v, args, has_args, 1, buf);
				break;
			}
			case MAKE_VAR_REF: {
				var->flags.ref	  = 1;
				make_var_data_t v = *var;
				ret |= make_var_eval(make, act, &v, args, has_args, 0, buf);
				break;
			}
			default: ret = 1; break;
			}
			break;
		}
		case MAKE_ACT_IF: {
			make_if_data_t *mif = &act->val.mif;

			buf->len = 0;
			ret |= make_str_expand(make, &mif->l, buf);
			ret |= eval_args(make, args, has_args, buf);
			make_var_data_t lvar = {
				.expanded = mif->lexpanded,
				.resolved = mif->lresolved,
			};
			ret |= make_var_app(make, &lvar, 0, buf);

			buf->len = 0;
			ret |= make_str_expand(make, &mif->r, buf);
			ret |= eval_args(make, args, has_args, buf);
			make_var_data_t rvar = {
				.expanded = mif->rexpanded,
				.resolved = mif->rresolved,
			};
			ret |= make_var_app(make, &rvar, 0, buf);

			strv_t l = strvbuf_get(&make->strs, lvar.resolved);
			strv_t r = strvbuf_get(&make->strs, rvar.resolved);

			if (strv_eq(l, r)) {
				if (mif->has_true_acts) {
					ret |= make_vars_eval_act(make, mif->true_acts, args, has_args, def, scope, buf);
				}
			} else {
				if (mif->has_false_acts) {
					ret |= make_vars_eval_act(make, mif->false_acts, args, has_args, def, scope, buf);
				}
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

			if (def->has_acts) {
				ret |= make_vars_eval_act(make, def->acts, eval->args, 1, 1, scope + 1, buf);

				uint j = 0;
				make_act_data_t *act;
				list_foreach_all(&make->acts, j, act)
				{
					if (act->type != MAKE_ACT_VAR) {
						continue;
					}

					if (act->val.var.scope > scope) {
						act->val.var.scope = 0;
					}
				}
			}

			break;
		}
		case MAKE_ACT_INCLUDE: {
			if (act->val.inc.has_acts) {
				ret |= make_vars_eval_act(make, act->val.inc.acts, args, has_args, def, scope, buf);
			}
			break;
		}
		default: break;
		}

		first = 0;
	}

	return ret;
}

int make_eval(make_t *make, make_act_t acts, str_t *buf)
{
	if (make == NULL || buf == NULL) {
		return 1;
	}

	strvbuf_reset(&make->strs, make->strs_used);

	int ret = 0;

	uint i = 0;
	make_act_data_t *act;
	list_foreach_all(&make->acts, i, act)
	{
		switch (act->type) {
		case MAKE_ACT_VAR:
			ret |= strvbuf_add(&make->strs, STRV_NULL, &act->val.var.expanded);
			ret |= strvbuf_add(&make->strs, STRV_NULL, &act->val.var.resolved);
			break;
		case MAKE_ACT_IF:
			ret |= strvbuf_add(&make->strs, STRV_NULL, &act->val.mif.lexpanded);
			ret |= strvbuf_add(&make->strs, STRV_NULL, &act->val.mif.lresolved);
			ret |= strvbuf_add(&make->strs, STRV_NULL, &act->val.mif.rexpanded);
			ret |= strvbuf_add(&make->strs, STRV_NULL, &act->val.mif.rresolved);
			break;
		default: break;
		}
	}

	return ret || make_vars_eval_act(make, acts, 0, 0, 0, 0, buf);
}

strv_t make_get_expanded(const make_t *make, make_act_t act)
{
	if (make == NULL) {
		return STRV_NULL;
	}

	const make_var_data_t *var = make_act_get_type(make, act, MAKE_ACT_VAR);
	if (var == NULL) {
		log_error("cparse", "make", NULL, "failed to get expanded variable: %d", act);
		return STRV_NULL;
	}

	strv_t val = strvbuf_get(&make->strs, var->expanded);
	return val.len > 0 ? val : STRV("");
}

strv_t make_get_resolved(const make_t *make, make_act_t act, str_t *buf)
{
	if (make == NULL) {
		return STRV_NULL;
	}

	make_var_data_t *var = make_act_get_type(make, act, MAKE_ACT_VAR);
	if (var == NULL) {
		log_error("cparse", "make", NULL, "failed to get resolved variable: %d", act);
		return STRV_NULL;
	}

	if (var->flags.ref) {
		strv_t expanded = strvbuf_get(&make->strs, var->expanded);

		buf->len = 0;
		str_cat(buf, expanded);

		replace_vars_priv_t priv = {
			.make = make,
		};

		if (make_replace(buf, 1, replace_vars, &priv)) {
			return STRV_NULL;
		}
		vars_set((make_t *)make, var, var->resolved, STRVS(*buf));
	}

	strv_t val = strvbuf_get(&make->strs, var->resolved);
	return val.len > 0 ? val : STRV("");
}

size_t make_print_vars(const make_t *make, dst_t dst)
{
	size_t off = dst.off;

	uint i = 0;
	const make_act_data_t *act;
	list_foreach_all(&make->acts, i, act)
	{
		if (act->type != MAKE_ACT_VAR) {
			continue;
		}

		const make_var_data_t *var = &act->val.var;

		strv_t name	= strvbuf_get(&make->strs, var->name);
		strv_t expanded = strvbuf_get(&make->strs, var->expanded);
		strv_t resolved = strvbuf_get(&make->strs, var->resolved);
		dst.off +=
			dputf(dst, "%-16.*s %-64.*s %.*s\n", name.len, name.data, expanded.len, expanded.data, resolved.len, resolved.data);
	}
	return off - dst.off;
}

static strv_t make_var_type_to_str(make_var_type_t type)
{
	switch (type) {
	case MAKE_VAR_INST: return STRV(" :=");
	case MAKE_VAR_REF: return STRV(" =");
	// case MAKE_VAR_COND: return STRV(" ?=");
	case MAKE_VAR_APP: return STRV(" +=");
	default: return STRV_NULL;
	}
}

static strv_t make_cmd_type_to_str(make_cmd_type_t type)
{
	switch (type) {
	case MAKE_CMD_NORMAL: return STRV("NORMAL");
	case MAKE_CMD_CHILD: return STRV("CHILD");
	case MAKE_CMD_ERR: return STRV("ERROR");
	default: return STRV_NULL;
	}
}

static size_t make_str_print(const make_t *make, const make_str_data_t *str, dst_t dst)
{
	size_t off = dst.off;

	switch (str->type) {
	case MAKE_STR_VAR: {
		make_var_data_t *data = make_act_get_type(make, str->val.var, MAKE_ACT_VAR);
		if (data == NULL) {
			log_warn("cparse", "make", NULL, "variable not found");
			return 0;
		}
		dst.off += dputs(dst, data->def ? STRV("$$(") : STRV("$("));
		dst.off += dputs(dst, strvbuf_get(&make->strs, data->name));
		dst.off += dputs(dst, STRV(")"));
		break;
	}
	default: {
		dst.off += dputs(dst, strvbuf_get(&make->strs, str->val.val));
		break;
	}
	}

	return dst.off - off;
}

static size_t make_rule_target_print(const make_t *make, const make_rule_target_data_t *target, dst_t dst)
{
	size_t off = dst.off;

	dst.off += make_str_print(make, &target->target, dst);
	if (target->has_action) {
		dst.off += dputs(dst, strvbuf_get(&make->strs, target->action));
	}

	return dst.off - off;
}

static size_t make_acts_print(const make_t *make, make_act_t acts, dst_t dst, int rule)
{
	size_t off = dst.off;
	const make_act_data_t *act;
	make_act_t i = acts;
	int first    = 1;
	list_foreach(&make->acts, i, act)
	{
		if (i == acts && !first) {
			log_error("cparse", "make", NULL, "loop detected: %d", i);
			break;
		}

		switch (act->type) {
		case MAKE_ACT_EMPTY: {
			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_VAR: {
			if (act->val.var.ext) {
				break;
			}

			strv_t var_type_str = make_var_type_to_str(act->val.var.type);
			if (var_type_str.data == NULL) {
				break;
			}

			dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.var.name));
			dst.off += dputs(dst, var_type_str);

			if (act->val.var.has_values) {
				const make_str_data_t *value;
				list_node_t j = act->val.var.values;
				list_foreach(&make->arrs, j, value)
				{
					dst.off += dputs(dst, STRV(" "));
					dst.off += make_str_print(make, value, dst);
				}
			}

			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_RULE: {
			dst.off += make_rule_target_print(make, &act->val.rule.target, dst);
			dst.off += dputs(dst, STRV(":"));

			if (act->val.rule.has_depends) {
				const make_rule_target_data_t *depend;
				list_node_t j = act->val.rule.depends;
				list_foreach(&make->targets, j, depend)
				{
					dst.off += dputs(dst, STRV(" "));
					dst.off += make_rule_target_print(make, depend, dst);
				}
			}

			dst.off += dputs(dst, STRV("\n"));
			if (act->val.rule.has_acts) {
				dst.off += make_acts_print(make, act->val.rule.acts, dst, 1);
			}
			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_CMD: {
			switch (act->val.cmd.type) {
			case MAKE_CMD_CHILD: {
				dst.off += dputs(dst, STRV("\t@$(MAKE) -C"));
				for (uint j = 0; j < act->val.cmd.args; j++) {
					dst.off += dputs(dst, STRV(" "));
					dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.cmd.arg[j]));
				}
				break;
			}
			case MAKE_CMD_ERR: {
				strv_t arg1 = strvbuf_get(&make->strs, act->val.cmd.arg[0]);
				if (rule) {
					dst.off += dputs(dst, STRV("\t"));
				}
				dst.off += dputs(dst, STRV("$(error "));
				dst.off += dputs(dst, arg1);
				dst.off += dputs(dst, STRV(")"));
				break;
			}
			default: {
				if (rule) {
					dst.off += dputs(dst, STRV("\t"));
				}
				dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.cmd.arg[0]));
				break;
			}
			}
			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_IF: {
			dst.off += dputs(dst, STRV("ifeq ("));
			dst.off += make_str_print(make, &act->val.mif.l, dst);
			dst.off += dputs(dst, STRV(","));
			dst.off += make_str_print(make, &act->val.mif.r, dst);
			dst.off += dputs(dst, STRV(")\n"));

			if (act->val.mif.has_true_acts) {
				dst.off += make_acts_print(make, act->val.mif.true_acts, dst, rule);
			}

			if (act->val.mif.has_false_acts) {
				dst.off += dputs(dst, STRV("else\n"));
				dst.off += make_acts_print(make, act->val.mif.false_acts, dst, rule);
			}

			dst.off += dputs(dst, STRV("endif\n"));
			break;
		}
		case MAKE_ACT_DEF: {
			dst.off += dputs(dst, STRV("define "));
			dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.def.name));
			dst.off += dputs(dst, STRV("\n"));
			if (act->val.def.has_acts) {
				dst.off += make_acts_print(make, act->val.def.acts, dst, rule);
			}
			dst.off += dputs(dst, STRV("endef\n"));
			break;
		}
		case MAKE_ACT_EVAL_DEF: {
			make_def_data_t *data = make_act_get_type(make, act->val.eval_def.def, MAKE_ACT_DEF);
			if (data == NULL) {
				break;
			}

			strv_t name = strvbuf_get(&make->strs, data->name);
			dst.off += dputs(dst, STRV("$(eval $(call "));
			dst.off += dputs(dst, name);

			const make_str_data_t *value;
			list_node_t j = act->val.eval_def.args;
			list_foreach(&make->arrs, j, value)
			{
				if (j == act->val.eval_def.args) {
					continue;
				}
				dst.off += dputs(dst, STRV(","));
				dst.off += make_str_print(make, value, dst);
			}

			dst.off += dputs(dst, STRV("))\n"));
			break;
		}
		case MAKE_ACT_INCLUDE: {
			dst.off += dputs(dst, STRV("include "));
			dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.inc.path));
			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		}

		first = 0;
	}

	return dst.off - off;
}

size_t make_inc_print(const make_t *make, make_act_t inc, dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	make_inc_data_t *data = make_act_get_type(make, inc, MAKE_ACT_INCLUDE);
	if (data == NULL) {
		return 0;
	}

	return data->has_acts ? make_acts_print(make, data->acts, dst, 0) : 0;
}

size_t make_print(const make_t *make, make_act_t acts, dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	return make_acts_print(make, acts, dst, 0);
}

size_t make_dbg(const make_t *make, dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

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
			dst.off += dputs(dst,
					 STRV("VAR\n"
					      "    NAME    : "));
			dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.var.name));
			if (act->val.var.ext) {
				dst.off += dputs(dst, STRV(" (ext)"));
			}
			dst.off += dputs(dst,
					 STRV("\n"
					      "    VALUES  :\n"));

			if (act->val.var.has_values) {
				const make_str_data_t *value;
				list_node_t j = act->val.var.values;
				int first     = 1;
				list_foreach(&make->arrs, j, value)
				{
					if (j == act->val.var.values && !first) {
						log_error("cparse", "make", NULL, "loop detected: %d", j);
						break;
					}
					dst.off += dputs(dst, STRV("        "));
					dst.off += make_str_print(make, value, dst);
					dst.off += dputs(dst, STRV("\n"));
					first = 0;
				}
			}

			break;
		}
		case MAKE_ACT_RULE: {
			dst.off += dputs(dst,
					 STRV("RULE\n"
					      "    TARGET: "));
			dst.off += make_rule_target_print(make, &act->val.rule.target, dst);

			dst.off += dputs(dst, STRV("\n    DEPENDS:\n"));
			if (act->val.rule.has_depends) {
				const make_rule_target_data_t *depend;
				list_node_t j = act->val.rule.depends;
				int first     = 1;
				list_foreach(&make->targets, j, depend)
				{
					if (j == act->val.rule.depends && !first) {
						log_error("cparse", "make", NULL, "loop detected: %d", j);
						break;
					}
					dst.off += dputs(dst, STRV("        "));
					dst.off += make_rule_target_print(make, depend, dst);
					dst.off += dputs(dst, STRV("\n"));
					first = 0;
				}
			}
			break;
		}
		case MAKE_ACT_CMD: {
			dst.off += dputs(dst,
					 STRV("CMD\n"
					      "    ARG1: "));
			if (act->val.cmd.args > 0) {
				dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.cmd.arg[0]));
			}
			dst.off += dputs(dst,
					 STRV("\n"
					      "    ARG2: "));
			if (act->val.cmd.args > 1) {
				dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.cmd.arg[1]));
			}
			dst.off += dputs(dst,
					 STRV("\n"
					      "    TYPE: "));
			dst.off += dputs(dst, make_cmd_type_to_str(act->val.cmd.type));
			dst.off += dputs(dst, STRV("\n"));
			break;
		}
		case MAKE_ACT_IF: {
			dst.off += dputs(dst,
					 STRV("IF\n"
					      "    L: '"));
			dst.off += make_str_print(make, &act->val.mif.l, dst);
			dst.off += dputs(dst, STRV("'\n"));
			dst.off += dputs(dst, STRV("    R: '"));
			dst.off += make_str_print(make, &act->val.mif.r, dst);
			dst.off += dputs(dst, STRV("'\n"));
			break;
		}
		case MAKE_ACT_DEF: {
			dst.off += dputs(dst,
					 STRV("DEF\n"
					      "    NAME: '"));
			dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.def.name));
			dst.off += dputs(dst, STRV("'\n"));
			break;
		}
		case MAKE_ACT_EVAL_DEF: {
			make_def_data_t *def = make_act_get_type(make, act->val.eval_def.def, MAKE_ACT_DEF);
			if (def == NULL) {
				break;
			}

			dst.off += dputs(dst,
					 STRV("EVAL DEF\n"
					      "    DEF: '"));
			dst.off += dputs(dst, strvbuf_get(&make->strs, def->name));
			dst.off += dputs(dst,
					 STRV("'\n"
					      "    ARGS:\n"));
			const make_str_data_t *arg;
			list_node_t j = act->val.eval_def.args;
			int first     = 1;
			list_foreach(&make->arrs, j, arg)
			{
				if (j == act->val.eval_def.args && !first) {
					log_error("cparse", "make", NULL, "loop detected: %d", j);
					break;
				}
				dst.off += dputs(dst, STRV("        "));
				dst.off += make_str_print(make, arg, dst);
				dst.off += dputs(dst, STRV("\n"));
				first = 0;
			}
			break;
		}
		case MAKE_ACT_INCLUDE: {
			dst.off += dputs(dst,
					 STRV("INCLUDE\n"
					      "    PATH: '"));
			dst.off += dputs(dst, strvbuf_get(&make->strs, act->val.inc.path));
			dst.off += dputs(dst, STRV("'\n"));
			break;
		}
		}
	}

	return dst.off - off;
}
