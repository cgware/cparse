#include "file/make.h"

#include "log.h"
#include "platform.h"

#define MAX_VAR_VALUE_LEN 256

typedef struct make_var_data_s {
	uint id;
	make_var_type_t type;
	lnode_t values;
	int ext : 1;
	int init : 1;
} make_var_data_t;

typedef struct make_rule_data_s {
	make_rule_target_data_t target;
	lnode_t depends;
	make_act_t acts;
	int file : 1;
} make_rule_data_t;

typedef struct make_if_data_s {
	make_str_data_t l;
	make_str_data_t r;
	make_act_t true_acts;
	make_act_t false_acts;
	uint l_id;
	uint r_id;
} make_if_data_t;

typedef enum make_act_type_e {
	MAKE_ACT_EMPTY,
	MAKE_ACT_VAR,
	MAKE_ACT_RULE,
	MAKE_ACT_CMD,
	MAKE_ACT_IF,
} make_act_type_t;

typedef struct make_act_data_s {
	make_act_type_t type;
	union {
		make_var_data_t var;
		make_rule_data_t rule;
		make_cmd_data_t cmd;
		make_if_data_t mif;
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

static inline make_var_data_t *make_var_get(const make_t *make, make_var_t var)
{
	make_act_data_t *act = make_act_get(make, var);
	if (act == NULL || act->type != MAKE_ACT_VAR) {
		return NULL;
	}

	return &act->val.var;
}

static inline make_rule_data_t *make_rule_get(const make_t *make, make_rule_t rule)
{
	make_act_data_t *act = make_act_get(make, rule);
	if (act == NULL || act->type != MAKE_ACT_RULE) {
		return NULL;
	}

	return &act->val.rule;
}

static inline make_if_data_t *make_if_get(const make_t *make, make_if_t mif)
{
	make_act_data_t *act = make_act_get(make, mif);
	if (act == NULL || act->type != MAKE_ACT_IF) {
		return NULL;
	}

	return &act->val.mif;
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

make_t *make_init(make_t *make, uint strs_cap, uint acts_cap, uint targets_cap, uint vars_cap, alloc_t alloc)
{
	if (make == NULL) {
		return NULL;
	}

	if (list_init(&make->strs, strs_cap, sizeof(make_str_data_t), alloc) == NULL) {
		return NULL;
	}

	if (list_init(&make->acts, acts_cap, sizeof(make_act_data_t), alloc) == NULL) {
		return NULL;
	}

	if (list_init(&make->targets, targets_cap, sizeof(make_rule_target_data_t), alloc) == NULL) {
		return NULL;
	}

	if (strbuf_init(&make->vars, vars_cap, 8, alloc) == NULL) {
		return NULL;
	}

	make->root = MAKE_END;

	return make;
}

static void make_str_free(make_str_data_t *str)
{
	if (str->type != MAKE_STR_STR) {
		return;
	}

	str_free(&str->val.str);
}

static inline void make_var_free(make_var_data_t *var)
{
	(void)var;
}

static inline void make_rule_free(make_rule_data_t *rule)
{
	make_str_free(&rule->target.target);
	str_free(&rule->target.action);
}

static inline void make_cmd_free(make_cmd_data_t *cmd)
{
	str_free(&cmd->arg1);
	str_free(&cmd->arg2);
}

static inline void make_if_free(make_if_data_t *mif)
{
	make_str_free(&mif->l);
	make_str_free(&mif->r);
}

void make_free(make_t *make)
{
	if (make == NULL) {
		return;
	}

	make_str_data_t *str;
	list_foreach_all(&make->strs, str)
	{
		make_str_free(str);
	}
	list_free(&make->strs);

	make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_VAR: make_var_free(&act->val.var); break;
		case MAKE_ACT_RULE: make_rule_free(&act->val.rule); break;
		case MAKE_ACT_CMD: make_cmd_free(&act->val.cmd); break;
		case MAKE_ACT_IF: make_if_free(&act->val.mif); break;
		default: break;
		}
	}
	list_free(&make->acts);

	make_rule_target_data_t *target;
	list_foreach_all(&make->targets, target)
	{
		make_str_free(&target->target);
		str_free(&target->action);
	}
	list_free(&make->targets);

	strbuf_free(&make->vars);

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
	if (id && *id < make->vars.off.cnt) {
		index = *id;
	} else if (strbuf_add(&make->vars, name, &index)) {
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

make_rule_t make_create_rule(make_t *make, make_rule_target_data_t target, int file)
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
				.target	 = target,
				.depends = MAKE_END,
				.acts	 = MAKE_END,
				.file	 = file,
			},
	};

	return act;
}

make_rule_t make_create_phony(make_t *make)
{
	return make_create_rule(make, MRULE(MSTR(STR(".PHONY"))), 1);
}

make_cmd_t make_create_cmd(make_t *make, make_cmd_data_t cmd)
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
		.type	 = MAKE_ACT_CMD,
		.val.cmd = cmd,
	};

	return act;
}

make_if_t make_create_if(make_t *make, make_str_data_t l, make_str_data_t r)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL) {
		return MAKE_END;
	}

	uint l_id;
	if (strbuf_add(&make->vars, STRV("IF_L"), &l_id)) {
		return MAKE_END;
	}

	uint r_id;
	if (strbuf_add(&make->vars, STRV("IF_R"), &r_id)) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_IF,
		.val.mif =
			{
				.l	    = l,
				.r	    = r,
				.true_acts  = MAKE_END,
				.false_acts = MAKE_END,
				.l_id	    = l_id,
				.r_id	    = r_id,
			},
	};

	return act;
}

make_act_t make_add_act(make_t *make, make_act_t act)
{
	make_act_data_t *data = make_act_get(make, act);
	if (data == NULL) {
		return MAKE_END;
	}

	if (data->type == MAKE_ACT_RULE && !data->val.rule.file) {
		make_rule_t phony = make_rule_get_target(make, MRULE(MSTR(STR(".PHONY"))));
		if (phony == MAKE_END) {
			phony = make_add_act(make, make_create_phony(make));
		}
		make_rule_target_data_t depend = data->val.rule.target;
		if (depend.target.type == MAKE_STR_STR) {
			depend.target.val.str.ref = 1;
			depend.action.ref	  = 1;
		}
		make_rule_add_depend(make, phony, depend);
	}

	return list_set_next_node(&make->acts, make->root, act);
}

make_var_t make_var_add_val(make_t *make, make_var_t var, make_str_data_t val)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_var_data_t *data = make_var_get(make, var);

	if (data == NULL || data->ext) {
		return MAKE_END;
	}

	make_str_t str;
	list_add_next_node(&make->strs, data->values, str);
	make_str_data_t *target = list_get_data(&make->strs, str);

	if (target == NULL) {
		return MAKE_END;
	}

	*target = val;

	return var;
}

make_str_t make_rule_add_depend(make_t *make, make_rule_t rule, make_rule_target_data_t depend)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_rule_data_t *data = make_rule_get(make, rule);

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

make_act_t make_rule_add_act(make_t *make, make_rule_t rule, make_act_t act)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_rule_data_t *data = make_rule_get(make, rule);

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

	make_if_data_t *data = make_if_get(make, mif);

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

make_str_t make_ext_set_val(make_t *make, uint id, make_str_data_t val)
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
	list_add_node(&make->strs, make_var_get(make, act)->values, str);
	make_str_data_t *target = list_get_data(&make->strs, str);

	if (target == NULL) {
		return MAKE_END;
	}

	*target = val;

	return str;
}

static int make_rule_target_eq(const make_t *make, make_rule_target_data_t target1, make_rule_target_data_t target2)
{
	if (target1.target.type != target2.target.type) {
		return 0;
	}

	if (!str_eq(target1.action, target2.action)) {
		return 0;
	}

	switch (target1.target.type) {
	case MAKE_STR_VAR: return make_var_get(make, target1.target.val.var)->id == make_var_get(make, target2.target.val.var)->id;
	default: return str_eq(target1.target.val.str, target2.target.val.str);
	}
}

make_rule_t make_rule_get_target(const make_t *make, make_rule_target_data_t target)
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

	if (arr_init(&vars->flags, make->vars.off.cnt, sizeof(make_var_flags_t), alloc) == NULL ||
	    strbuf_init(&vars->expanded, make->vars.off.cnt, 16, alloc) == NULL ||
	    strbuf_init(&vars->resolved, make->vars.off.cnt, 16, alloc) == NULL) {
		return NULL;
	}

	return vars;
}

void make_vars_free(make_vars_t *vars)
{
	if (vars == NULL) {
		return;
	}

	arr_free(&vars->flags);
	strbuf_free(&vars->expanded);
	strbuf_free(&vars->resolved);
}

static int make_str_expand(const make_t *make, const make_str_data_t *str, str_t *buf)
{
	switch (str->type) {
	case MAKE_STR_VAR: {
		make_var_data_t *data = make_var_get(make, str->val.var);
		if (data == NULL) {
			log_warn("cparse", "make", NULL, "variable not found");
			return 1;
		}
		strv_t name = strbuf_get(&make->vars, data->id);
		str_cat(buf, STR("$("));
		str_cat(buf, strc(name.data, name.len));
		str_cat(buf, STR(")"));
		break;
	}
	default: str_cat(buf, str->val.str); break;
	}

	return 0;
}

static int make_var_replace(str_t *str, const strbuf_t *names, const strbuf_t *vals, size_t min_len)
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
		uint index;
		if (strbuf_get_index(names, name, &index)) {
			continue;
		}

		strv_t val = strbuf_get(vals, index);

		if (str_subreplace(str, s, e + 1, val)) {
			log_error(
				"build", "var", NULL, "failed to replace %.*s with value: '%.*s'", name.len, name.data, val.len, val.data);
			ret = 1;
			continue;
		}
		s--;
	}

	((char *)str->data)[str->len] = '\0';
	return ret;
}

static int make_var_app(const make_t *make, make_vars_t *vars, uint id, int app, str_t *buf)
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
			log_error("cparse", "make", NULL, "failed to append variable");
			ret = 1;
		}
	}

	if (!flags->ref) {
		ret |= make_var_replace(buf, &make->vars, &vars->resolved, 1);
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
			log_error("cparse", "make", NULL, "failed to append variable");
			ret = 1;
		}
	}

	return ret;
}

static int make_var_eval(const make_t *make, make_vars_t *vars, const make_var_data_t *var, int app, str_t *buf)
{
	int ret = 0;

	buf->len = 0;

	const make_str_data_t *value;
	int first = 1;
	list_foreach(&make->strs, var->values, value)
	{
		if (!first) {
			str_cat(buf, STR(" "));
		}

		ret |= make_str_expand(make, value, buf);

		first = 0;
	}

	ret |= make_var_app(make, vars, var->id, app, buf);

	return ret;
}

static int make_vars_eval_act(const make_t *make, make_vars_t *vars, make_act_t root, str_t *buf)
{
	int ret = 0;

	const make_act_data_t *act;
	list_foreach(&make->acts, root, act)
	{
		switch (act->type) {
		case MAKE_ACT_VAR: {
			const make_var_data_t *var = &act->val.var;

			make_var_flags_t *flags = arr_get(&vars->flags, act->val.var.id);

			if (var->ext && var->values < make->strs.cnt) {
				flags->ref = 1;
				ret |= make_var_eval(make, vars, var, 0, buf);
				flags->ext = 1;
			}

			if (flags->ext) {
				continue;
			}

			switch (var->type) {
			case MAKE_VAR_INST:
				flags->ref = 0;
				ret |= make_var_eval(make, vars, var, 0, buf);
				break;
			case MAKE_VAR_APP: ret |= make_var_eval(make, vars, var, 1, buf); break;
			case MAKE_VAR_REF:
				flags->ref = 1;
				ret |= make_var_eval(make, vars, var, 0, buf);
				break;
			default: ret = 1; break;
			}
			break;
		}
		case MAKE_ACT_IF: {
			const make_if_data_t *mif = &act->val.mif;
			make_var_flags_t *lflags  = arr_get(&vars->flags, act->val.mif.l_id);
			make_var_flags_t *rflags  = arr_get(&vars->flags, act->val.mif.r_id);

			buf->len    = 0;
			lflags->ext = 0;
			lflags->ref = 0;
			ret |= make_str_expand(make, &mif->l, buf);
			ret |= make_var_app(make, vars, act->val.mif.l_id, 0, buf);

			buf->len    = 0;
			rflags->ext = 0;
			rflags->ref = 0;
			ret |= make_str_expand(make, &mif->r, buf);
			ret |= make_var_app(make, vars, act->val.mif.r_id, 0, buf);

			strv_t l = strbuf_get(&vars->resolved, act->val.mif.l_id);
			strv_t r = strbuf_get(&vars->resolved, act->val.mif.r_id);

			if (strv_eq(l, r)) {
				ret |= make_vars_eval_act(make, vars, mif->true_acts, buf);
			} else {
				ret |= make_vars_eval_act(make, vars, mif->false_acts, buf);
			}
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

	arr_reset(&vars->flags, 0, make->vars.off.cnt);
	strbuf_reset(&vars->expanded, make->vars.off.cnt);
	strbuf_reset(&vars->resolved, make->vars.off.cnt);

	char buf[256] = {0};
	str_t buf_str = strb(buf, sizeof(buf), 0);

	return make_vars_eval_act(make, vars, make->root, &buf_str);
}

strv_t make_vars_get_expanded(const make_vars_t *vars, uint id)
{
	if (vars == NULL) {
		return STRV_NULL;
	}

	return strbuf_get(&vars->expanded, id);
}

strv_t make_vars_get_resolved(const make_t *make, const make_vars_t *vars, uint id)
{
	if (make == NULL || vars == NULL) {
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
		str_cat(&strbuf, strc(expanded.data, expanded.len));
		make_var_replace(&strbuf, &make->vars, &vars->resolved, 1);
		make_vars_t *v = (make_vars_t *)vars;
		strbuf_set(&v->resolved, STRV_STR(strbuf), id);
	}

	return strbuf_get(&vars->resolved, id);
}

int make_vars_print(const make_t *make, const make_vars_t *vars, print_dst_t dst)
{
	int off = dst.off;

	for (uint i = 0; i < make->vars.off.cnt; i++) {
		strv_t name	= strbuf_get(&make->vars, i);
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
	make_str_expand(make, &target->target, buf);
	dst.off += c_dprintf(dst, "%.*s", buf->len, buf->data);
	if (target->action.data != NULL) {
		dst.off += c_dprintf(dst, "%.*s", target->action.len, target->action.data);
	}

	return dst.off - off;
}

static inline int make_var_print(const make_t *make, const make_var_data_t *var, print_dst_t dst, str_t *buf)
{
	int off = dst.off;

	if (var->ext) {
		return 0;
	}

	const char *var_type_str = make_var_type_to_str(var->type);
	if (var_type_str == NULL) {
		return 0;
	}

	strv_t name = strbuf_get(&make->vars, var->id);
	dst.off += c_dprintf(dst, "%.*s %s", name.len, name.data, var_type_str);

	const make_str_data_t *value;
	list_foreach(&make->strs, var->values, value)
	{
		buf->len = 0;
		make_str_expand(make, value, buf);
		dst.off += c_dprintf(dst, " %.*s", buf->len, buf->data);
	}

	dst.off += c_dprintf(dst, "\n");

	return dst.off - off;
}

static int make_acts_print(const make_t *make, const make_act_t acts, print_dst_t dst, int rule, str_t *buf);

static inline int make_rule_print(const make_t *make, const make_rule_data_t *rule, print_dst_t dst, str_t *buf)
{
	int off = dst.off;
	dst.off += make_rule_target_print(make, &rule->target, dst, buf);
	dst.off += c_dprintf(dst, ":");

	const make_rule_target_data_t *depend;
	list_foreach(&make->targets, rule->depends, depend)
	{
		dst.off += c_dprintf(dst, " ");
		dst.off += make_rule_target_print(make, depend, dst, buf);
	}

	dst.off += c_dprintf(dst, "\n");
	dst.off += make_acts_print(make, rule->acts, dst, 1, buf);
	dst.off += c_dprintf(dst, "\n");

	return dst.off - off;
}

static inline int make_cmd_print(const make_t *make, const make_cmd_data_t *cmd, print_dst_t dst, int rule)
{
	(void)make;

	switch (cmd->type) {
	case MAKE_CMD_CHILD:
		if (cmd->arg2.data == NULL) {
			return c_dprintf(dst, "\t@$(MAKE) -C %.*s\n", cmd->arg1.len, cmd->arg1.data);
		} else {
			return c_dprintf(dst, "\t@$(MAKE) -C %.*s %.*s\n", cmd->arg1.len, cmd->arg1.data, cmd->arg2.len, cmd->arg2.data);
		}
	case MAKE_CMD_ERR: return c_dprintf(dst, "%s$(error %.*s)\n", rule ? "\t" : "", cmd->arg1.len, cmd->arg1.data);
	default: return c_dprintf(dst, "%s%.*s\n", rule ? "\t" : "", cmd->arg1.len, cmd->arg1.data);
	}
}

static inline int make_if_print(const make_t *make, const make_if_data_t *mif, print_dst_t dst, int rule, str_t *buf)
{
	int off = dst.off;

	buf->len = 0;
	make_str_expand(make, &mif->l, buf);
	dst.off += c_dprintf(dst, "ifeq (%.*s", buf->len, buf->data);

	buf->len = 0;
	make_str_expand(make, &mif->r, buf);
	dst.off += c_dprintf(dst, ",%.*s", buf->len, buf->data);

	dst.off += c_dprintf(dst, ")\n");

	dst.off += make_acts_print(make, mif->true_acts, dst, rule, buf);

	if (mif->false_acts != MAKE_END) {
		dst.off += c_dprintf(dst, "else\n");
		dst.off += make_acts_print(make, mif->false_acts, dst, rule, buf);
	}

	dst.off += c_dprintf(dst, "endif\n");

	return dst.off - off;
}

static int make_acts_print(const make_t *make, make_act_t acts, print_dst_t dst, int rule, str_t *buf)
{
	int off = dst.off;
	const make_act_data_t *act;
	list_foreach(&make->acts, acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_EMPTY: dst.off += c_dprintf(dst, "\n"); break;
		case MAKE_ACT_VAR: dst.off += make_var_print(make, &act->val.var, dst, buf); break;
		case MAKE_ACT_RULE: dst.off += make_rule_print(make, &act->val.rule, dst, buf); break;
		case MAKE_ACT_CMD: dst.off += make_cmd_print(make, &act->val.cmd, dst, rule); break;
		case MAKE_ACT_IF: dst.off += make_if_print(make, &act->val.mif, dst, rule, buf); break;
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

static inline int make_empty_dbg(const make_t *make, print_dst_t dst)
{
	(void)make;
	return c_dprintf(dst, "EMPTY\n\n");
}

static inline int make_var_dbg(const make_t *make, const make_var_data_t *var, print_dst_t dst, str_t *buf)
{
	int off = dst.off;

	strv_t name = strbuf_get(&make->vars, var->id);
	dst.off += c_dprintf(dst,
			     "VAR\n"
			     "    NAME    : %.*s %s\n"
			     "    VALUES  :\n",
			     name.len,
			     name.data,
			     var->ext ? "(ext)" : "");
	const make_str_data_t *value;
	list_foreach(&make->strs, var->values, value)
	{
		buf->len = 0;
		make_str_expand(make, value, buf);
		dst.off += c_dprintf(dst, "        %.*s\n", buf->len, buf->data);
	}

	return dst.off - off;
}

static inline int make_rule_dbg(const make_t *make, const make_rule_data_t *rule, print_dst_t dst, str_t *buf)
{
	int off = dst.off;
	dst.off += c_dprintf(dst,
			     "RULE\n"
			     "    TARGET: ");
	dst.off += make_rule_target_print(make, &rule->target, dst, buf);

	dst.off += c_dprintf(dst, "\n    DEPENDS:\n");
	const make_rule_target_data_t *depend;
	list_foreach(&make->targets, rule->depends, depend)
	{
		dst.off += c_dprintf(dst, "        ");
		dst.off += make_rule_target_print(make, depend, dst, buf);
		dst.off += c_dprintf(dst, "\n");
	}

	return dst.off - off;
}

static inline int make_cmd_dbg(const make_t *make, const make_cmd_data_t *cmd, print_dst_t dst)
{
	(void)make;
	int off = dst.off;
	dst.off += c_dprintf(dst,
			     "CMD\n"
			     "    ARG1: %.*s\n"
			     "    ARG2: %.*s\n"
			     "    TYPE: %d\n",
			     cmd->arg1.len,
			     cmd->arg1.data,
			     cmd->arg2.len,
			     cmd->arg2.data,
			     cmd->type);

	return dst.off - off;
}

static inline int make_if_dbg(const make_t *make, const make_if_data_t *mif, print_dst_t dst, str_t *buf)
{
	int off	 = dst.off;
	buf->len = 0;
	make_str_expand(make, &mif->l, buf);
	dst.off += c_dprintf(dst,
			     "IF\n"
			     "    L: '%.*s'",
			     buf->len,
			     buf->data);
	buf->len = 0;
	make_str_expand(make, &mif->r, buf);
	dst.off += c_dprintf(dst,
			     "\n"
			     "    R: '%.*s'\n"
			     "\n",
			     buf->len,
			     buf->data);

	return dst.off - off;
}

int make_dbg(const make_t *make, print_dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	char buf[256] = {0};
	str_t buf_str = strb(buf, sizeof(buf), 0);

	int off = dst.off;
	const make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_EMPTY: dst.off += make_empty_dbg(make, dst); break;
		case MAKE_ACT_VAR: dst.off += make_var_dbg(make, &act->val.var, dst, &buf_str); break;
		case MAKE_ACT_RULE: dst.off += make_rule_dbg(make, &act->val.rule, dst, &buf_str); break;
		case MAKE_ACT_CMD: dst.off += make_cmd_dbg(make, &act->val.cmd, dst); break;
		case MAKE_ACT_IF: dst.off += make_if_dbg(make, &act->val.mif, dst, &buf_str); break;
		}
	}

	return dst.off - off;
}
