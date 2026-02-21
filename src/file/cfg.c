#include "file/cfg.h"

#include "log.h"

cfg_t *cfg_init(cfg_t *cfg, uint strs_cap, uint vars_cap, alloc_t alloc)
{
	if (cfg == NULL) {
		return NULL;
	}

	if (strvbuf_init(&cfg->strs, strs_cap, strs_cap * 8, alloc) == NULL) {
		log_error("cparse", "cfg", NULL, "failed to initialize stings");
		return NULL;
	}

	if (list_init(&cfg->vars, vars_cap, sizeof(cfg_var_data_t), alloc) == NULL) {
		log_error("cparse", "cfg", NULL, "failed to initialize values");
		return NULL;
	}

	return cfg;
}

void cfg_free(cfg_t *cfg)
{
	if (cfg == NULL) {
		return;
	}

	list_free(&cfg->vars);
	strvbuf_free(&cfg->strs);
}

static int add_var(cfg_t *cfg, strv_t key, cfg_var_t *var, cfg_var_data_t data)
{
	if (cfg == NULL) {
		return 1;
	}

	data.key = -1;
	if (key.data) {
		if (strvbuf_add(&cfg->strs, key, &data.key)) {
			log_error("cparse", "cfg", NULL, "failed to add key");
			return 1;
		}
	}

	cfg_var_data_t *ptr = list_node(&cfg->vars, var);
	if (ptr == NULL) {
		log_error("cparse", "cfg", NULL, "failed to add value");
		return 1;
	}

	*ptr = data;

	return 0;
}

int cfg_root(cfg_t *cfg, cfg_var_t *var)
{
	return add_var(cfg, STRV_NULL, var, (cfg_var_data_t){.type = CFG_VAR_ROOT, .val.child = (uint)-1});
}

int cfg_lit(cfg_t *cfg, strv_t key, cfg_mode_t mode, strv_t str, cfg_var_t *var)
{
	if (cfg == NULL) {
		return 1;
	}

	size_t str_id;
	if (strvbuf_add(&cfg->strs, str, &str_id)) {
		log_error("cparse", "cfg", NULL, "failed to add string");
		return 1;
	}

	return add_var(cfg, key, var, (cfg_var_data_t){.type = CFG_VAR_LIT, .mode = mode, .val.str = str_id});
}

int cfg_str(cfg_t *cfg, strv_t key, cfg_mode_t mode, strv_t str, cfg_var_t *var)
{
	if (cfg == NULL) {
		return 1;
	}

	size_t str_id;
	if (strvbuf_add(&cfg->strs, str, &str_id)) {
		log_error("cparse", "cfg", NULL, "failed to add string");
		return 1;
	}

	return add_var(cfg, key, var, (cfg_var_data_t){.type = CFG_VAR_STR, .mode = mode, .val.str = str_id});
}

int cfg_int(cfg_t *cfg, strv_t key, cfg_mode_t mode, int val, cfg_var_t *var)
{
	return add_var(cfg, key, var, (cfg_var_data_t){.type = CFG_VAR_INT, .mode = mode, .val.i = val});
}

int cfg_arr(cfg_t *cfg, strv_t key, cfg_mode_t mode, int multiline, cfg_var_t *var)
{
	return add_var(cfg, key, var, (cfg_var_data_t){.type = CFG_VAR_ARR, .mode = mode, .multiline = multiline});
}

int cfg_obj(cfg_t *cfg, strv_t key, cfg_var_t *var)
{
	return add_var(cfg, key, var, (cfg_var_data_t){.type = CFG_VAR_OBJ});
}

int cfg_tbl(cfg_t *cfg, strv_t key, cfg_var_t *var)
{
	return add_var(cfg, key, var, (cfg_var_data_t){.type = CFG_VAR_TBL});
}

int cfg_add_var(cfg_t *cfg, cfg_var_t parent, cfg_var_t var)
{
	if (cfg == NULL) {
		return 1;
	}

	cfg_var_data_t *data = list_get(&cfg->vars, parent);
	if (data == NULL) {
		log_error("cparse", "cfg", NULL, "failed to get parent: %d", parent);
		return 1;
	}

	switch (data->type) {
	case CFG_VAR_ROOT:
	case CFG_VAR_ARR:
	case CFG_VAR_OBJ:
	case CFG_VAR_TBL: break;
	default: return 1;
	}

	if (data->has_val) {
		list_app(&cfg->vars, data->val.child, var);
	} else {
		data->val.child = var;
		data->has_val	= 1;
	}

	return 0;
}

static const char *val_type_str[] = {
	[CFG_VAR_UNKNOWN] = "unknown",
	[CFG_VAR_ROOT]	  = "root",
	[CFG_VAR_LIT]	  = "literal",
	[CFG_VAR_STR]	  = "string",
	[CFG_VAR_INT]	  = "int",
	[CFG_VAR_ARR]	  = "array",
	[CFG_VAR_OBJ]	  = "object",
	[CFG_VAR_TBL]	  = "table",
};

static const char *val_type_to_str(cfg_var_type_t type)
{
	if (type < CFG_VAR_UNKNOWN || type > CFG_VAR_TBL) {
		type = CFG_VAR_UNKNOWN; // LCOV_EXCL_LINE
	}

	return val_type_str[type];
}

int cfg_has_var(const cfg_t *cfg, cfg_var_t parent, strv_t key, cfg_var_t *var)
{
	if (cfg == NULL) {
		return 0;
	}

	cfg_var_data_t *data = list_get(&cfg->vars, parent);
	if (data == NULL) {
		log_error("cparse", "cfg", NULL, "failed to get parent: %d", parent);
		return 0;
	}

	switch (data->type) {
	case CFG_VAR_ROOT:
	case CFG_VAR_ARR:
	case CFG_VAR_OBJ:
	case CFG_VAR_TBL: break;
	default: return 0;
	}

	if (!data->has_val) {
		return 0;
	}

	cfg_var_t i = data->val.child;
	list_foreach(&cfg->vars, i, data)
	{
		if (!strv_eq(strvbuf_get(&cfg->strs, data->key), key)) {
			continue;
		}

		if (var) {
			*var = i;
		}
		return 1;
	}

	return 0;
}

strv_t cfg_get_key(const cfg_t *cfg, cfg_var_t var)
{
	const cfg_var_data_t *data = list_get(&cfg->vars, var);
	if (data == NULL) {
		log_error("cparse", "cfg", NULL, "failed to get variable: %d", var);
		return STRV_NULL;
	}

	return strvbuf_get(&cfg->strs, data->key);
}

const cfg_var_data_t *cfg_get_type(const cfg_t *cfg, cfg_var_t var, cfg_var_type_t type)
{
	if (cfg == NULL) {
		return NULL;
	}

	const cfg_var_data_t *data = list_get(&cfg->vars, var);
	if (data == NULL) {
		log_error("cparse", "cfg", NULL, "failed to get variable: %d", var);
		return NULL;
	}

	if (data->type != type) {
		log_error("cparse", "cfg", NULL, "expected %s, got %s", val_type_to_str(type), val_type_to_str(data->type));
		return NULL;
	}

	return data;
}

int cfg_get_lit(const cfg_t *cfg, cfg_var_t var, strv_t *val)
{
	const cfg_var_data_t *data = cfg_get_type(cfg, var, CFG_VAR_LIT);
	if (data == NULL) {
		return 1;
	}

	if (val) {
		*val = strvbuf_get(&cfg->strs, data->val.str);
	}

	return 0;
}

int cfg_get_str(const cfg_t *cfg, cfg_var_t var, strv_t *val)
{
	const cfg_var_data_t *data = cfg_get_type(cfg, var, CFG_VAR_STR);
	if (data == NULL) {
		return 1;
	}

	if (val) {
		*val = strvbuf_get(&cfg->strs, data->val.str);
	}

	return 0;
}

int cfg_get_int(const cfg_t *cfg, cfg_var_t var, int *val)
{
	const cfg_var_data_t *data = cfg_get_type(cfg, var, CFG_VAR_INT);
	if (data == NULL) {
		return 1;
	}

	if (val) {
		*val = data->val.i;
	}

	return 0;
}

cfg_var_data_t *cfg_it_begin(const cfg_t *cfg, cfg_var_t var, cfg_var_t *it)
{
	if (cfg == NULL) {
		return NULL;
	}

	cfg_var_data_t *data = list_get(&cfg->vars, var);
	if (data == NULL) {
		log_error("cparse", "cfg", NULL, "failed to get variable: %d", var);
		return NULL;
	}

	switch (data->type) {
	case CFG_VAR_ROOT:
	case CFG_VAR_ARR:
	case CFG_VAR_OBJ:
	case CFG_VAR_TBL: break;
	default: log_error("cparse", "cfg", NULL, "variable is not iteratable: %d", var); return NULL;
	}

	if (!data->has_val) {
		return NULL;
	}

	if (it) {
		*it = data->val.child;
	}

	return data;
}

cfg_var_data_t *cfg_it_next(const cfg_t *cfg, cfg_var_t *it)
{
	if (cfg == NULL || it == NULL) {
		return NULL;
	}

	return list_get_next(&cfg->vars, *it, it);
}

typedef enum val_mode_e {
	VAL_MODE_NONE,
	VAL_MODE_NL,
	VAL_MODE_OBJ,
} var_mode_t;

static const char *s_cfg_mode_str[] = {
	[CFG_MODE_UNKNOWN] = "",
	[CFG_MODE_SET]	   = "",
	[CFG_MODE_ADD]	   = "+",
	[CFG_MODE_SUB]	   = "-",
	[CFG_MODE_ENS]	   = "?",
};

static const char *cfg_mode_str(cfg_mode_t mode)
{
	if (mode < CFG_MODE_UNKNOWN || mode >= __CFG_MODE_CNT) {
		mode = CFG_MODE_UNKNOWN;
	}

	return s_cfg_mode_str[mode];
}

static size_t cfg_print_vars(const cfg_t *cfg, cfg_var_t parent, int tbl, int print_key, int multiline, dst_t dst);

static size_t cfg_print_var(const cfg_t *cfg, const cfg_var_data_t *data, int first, int print_key, dst_t dst)
{
	size_t off = dst.off;

	if (print_key) {
		switch (data->type) {
		case CFG_VAR_LIT:
		case CFG_VAR_STR:
		case CFG_VAR_INT:
		case CFG_VAR_ARR: {
			strv_t key = strvbuf_get(&cfg->strs, data->key);
			if (key.len > 0) {
				if (data->multiline) {
					dst.off += dputf(dst, "%.*s:\n", key.len, key.data);
				} else {
					dst.off += dputf(dst, "%.*s %s= ", key.len, key.data, cfg_mode_str(data->mode));
				}
			}
			break;
		}
		case CFG_VAR_OBJ: {
			strv_t key = strvbuf_get(&cfg->strs, data->key);
			if (key.len > 0) {
				dst.off += dputf(dst, "%.*s = ", key.len, key.data);
			}
			break;
		}
		case CFG_VAR_TBL: {
			strv_t key = strvbuf_get(&cfg->strs, data->key);
			if (!first) {
				dst.off += dputs(dst, STRV("\n"));
			}
			dst.off += dputf(dst, "[%.*s]\n", key.len, key.data);
			break;
		}
		default: log_error("cparse", "cfg", NULL, "unknown type: %d", data->type); break;
		}
	}

	switch (data->type) {
	case CFG_VAR_LIT: {
		dst.off += dputs(dst, strvbuf_get(&cfg->strs, data->val.str));
		break;
	}
	case CFG_VAR_STR: {
		strv_t str = strvbuf_get(&cfg->strs, data->val.str);
		dst.off += dputf(dst, "\"%.*s\"", str.len, str.data);
		break;
	}
	case CFG_VAR_INT: {
		dst.off += dputf(dst, "%d", data->val.i);
		break;
	}
	case CFG_VAR_ARR: {
		if (data->multiline) {
			if (data->has_val) {
				dst.off += cfg_print_vars(cfg, data->val.child, 0, 0, 1, dst);
			}
		} else {
			dst.off += dputs(dst, STRV("["));
			if (data->has_val) {
				dst.off += cfg_print_vars(cfg, data->val.child, 0, 0, 0, dst);
			}
			dst.off += dputs(dst, STRV("]"));
		}
		break;
	}
	case CFG_VAR_OBJ: {
		dst.off += dputs(dst, STRV("{"));
		if (data->has_val) {
			dst.off += cfg_print_vars(cfg, data->val.child, 0, 1, 0, dst);
		}
		dst.off += dputs(dst, STRV("}"));
		break;
	}
	case CFG_VAR_TBL: {
		if (data->has_val) {
			dst.off += cfg_print_vars(cfg, data->val.child, 1, 1, 0, dst);
		}
		break;
	}
	default: log_error("cparse", "cfg", NULL, "unknown type: %d", data->type); break;
	}

	return dst.off - off;
}

static size_t cfg_print_vars(const cfg_t *cfg, cfg_var_t parent, int tbl, int print_key, int multiline, dst_t dst)
{
	size_t off = dst.off;

	int first = 1;
	const cfg_var_data_t *data;
	list_foreach(&cfg->vars, parent, data)
	{
		if (!first && !tbl && !multiline) {
			dst.off += dputs(dst, STRV(", "));
		}

		dst.off += cfg_print_var(cfg, data, first, print_key, dst);

		if ((tbl && data->type != CFG_VAR_TBL) || multiline) {
			dst.off += dputs(dst, STRV("\n"));
		}

		first = 0;
	}

	return dst.off - off;
}

size_t cfg_print(const cfg_t *cfg, cfg_var_t var, dst_t dst)
{
	if (cfg == NULL) {
		return 0;
	}

	const cfg_var_data_t *data = list_get(&cfg->vars, var);
	if (data == NULL) {
		log_error("cparse", "cfg", NULL, "failed to print variable: %d", var);
		return 0;
	}

	switch (data->type) {
	case CFG_VAR_ROOT:
	case CFG_VAR_TBL: {
		if (data->has_val) {
			return cfg_print_vars(cfg, data->val.child, 1, 1, data->multiline, dst);
		}
		break;
	}
	default: return cfg_print_var(cfg, data, 1, 0, dst);
	}

	return 0;
}
