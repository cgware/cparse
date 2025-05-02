#include "file/cfg.h"

#include "log.h"

typedef struct var_data_s {
	uint key;
	cfg_val_type_t type;
	union {
		cfg_var_t child;
		uint str;
		int i;
	} val;
} var_data_t;

cfg_t *cfg_init(cfg_t *cfg, uint strs_cap, uint vars_cap, alloc_t alloc)
{
	if (cfg == NULL) {
		return NULL;
	}

	if (strbuf_init(&cfg->strs, strs_cap, strs_cap * 8, alloc) == NULL) {
		log_error("cparse", "cfg", NULL, "failed to initialize stings");
		return NULL;
	}

	if (list_init(&cfg->vars, vars_cap, sizeof(var_data_t), alloc) == NULL) {
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
	strbuf_free(&cfg->strs);
}

cfg_var_t cfg_var_init(cfg_t *cfg, strv_t key, cfg_val_t val)
{
	if (cfg == NULL) {
		return CFG_VAR_END;
	}

	uint key_id = -1;
	if (key.data) {
		if (strbuf_add(&cfg->strs, key, &key_id)) {
			log_error("cparse", "cfg", NULL, "failed to add key");
			return CFG_VAR_END;
		}
	}

	cfg_var_t var_id;
	var_data_t *data = list_add(&cfg->vars, &var_id);
	if (data == NULL) {
		log_error("cparse", "cfg", NULL, "failed to add value");
		return CFG_VAR_END;
	}

	switch (val.type) {
	case CFG_VAL_LIT:
	case CFG_VAL_STR: {
		uint str_id;
		if (strbuf_add(&cfg->strs, val.val.str, &str_id)) {
			log_error("cparse", "cfg", NULL, "failed to add string");
			return CFG_VAR_END;
		}

		*data = (var_data_t){
			.key	 = key_id,
			.type	 = val.type,
			.val.str = str_id,
		};

		break;
	}
	case CFG_VAL_INT: {
		*data = (var_data_t){
			.key   = key_id,
			.type  = val.type,
			.val.i = val.val.i,
		};

		break;
	}
	case CFG_VAL_ROOT:
	case CFG_VAL_ARR:
	case CFG_VAL_OBJ:
	case CFG_VAL_TBL: {
		*data = (var_data_t){
			.key	   = key_id,
			.type	   = val.type,
			.val.child = (uint)-1,
		};

		break;
	}
	default: {
		log_error("cparse", "cfg", NULL, "unknown val type: %d", val.type);
		return CFG_VAR_END;
	}
	}

	return var_id;
}

cfg_var_t cfg_add_var(cfg_t *cfg, cfg_var_t parent, cfg_var_t var)
{
	if (cfg == NULL) {
		return CFG_VAR_END;
	}

	var_data_t *data = list_get(&cfg->vars, parent);
	if (data == NULL) {
		return CFG_VAR_END;
	}

	switch (data->type) {
	case CFG_VAL_ROOT:
	case CFG_VAL_ARR:
	case CFG_VAL_OBJ:
	case CFG_VAL_TBL: break;
	default: return CFG_VAR_END;
	}

	if (data->val.child < cfg->vars.cnt) {
		list_set_next(&cfg->vars, data->val.child, var);
	} else {
		data->val.child = var;
	}

	return var;
}

static const char *val_type_str[] = {
	[CFG_VAL_UNKNOWN] = "unknown",
	[CFG_VAL_ROOT]	  = "root",
	[CFG_VAL_LIT]	  = "literal",
	[CFG_VAL_STR]	  = "string",
	[CFG_VAL_INT]	  = "int",
	[CFG_VAL_ARR]	  = "array",
	[CFG_VAL_OBJ]	  = "object",
	[CFG_VAL_TBL]	  = "table",
};

static const char *val_type_to_str(cfg_val_type_t type)
{
	if (type < CFG_VAL_UNKNOWN || type > CFG_VAL_TBL) {
		type = CFG_VAL_UNKNOWN; // LCOV_EXCL_LINE
	}

	return val_type_str[type];
}

int cfg_get_var(const cfg_t *cfg, cfg_var_t parent, strv_t key, cfg_var_t *var)
{
	if (cfg == NULL) {
		return 1;
	}

	uint index;
	if (strbuf_find(&cfg->strs, key, &index)) {
		return 1;
	}

	const var_data_t *data = list_get(&cfg->vars, parent);
	switch (data->type) {
	case CFG_VAL_ROOT:
	case CFG_VAL_ARR:
	case CFG_VAL_OBJ:
	case CFG_VAL_TBL: break;
	default: return 1;
	}

	cfg_var_t i = data->val.child;
	list_foreach(&cfg->vars, i, data)
	{
		if (data->key != index) {
			continue;
		}

		if (var) {
			*var = i;
		}
		return 0;
	}

	return 1;
}

const var_data_t *cfg_get_type(const cfg_t *cfg, cfg_var_t var, cfg_val_type_t type)
{
	if (cfg == NULL) {
		return NULL;
	}

	const var_data_t *data = list_get(&cfg->vars, var);
	if (data == NULL) {
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
	const var_data_t *data = cfg_get_type(cfg, var, CFG_VAL_LIT);
	if (data == NULL) {
		return 1;
	}

	if (val) {
		*val = strbuf_get(&cfg->strs, data->val.str);
	}

	return 0;
}

int cfg_get_str(const cfg_t *cfg, cfg_var_t var, strv_t *val)
{
	const var_data_t *data = cfg_get_type(cfg, var, CFG_VAL_STR);
	if (data == NULL) {
		return 1;
	}

	if (val) {
		*val = strbuf_get(&cfg->strs, data->val.str);
	}

	return 0;
}

int cfg_get_int(const cfg_t *cfg, cfg_var_t var, int *val)
{
	const var_data_t *data = cfg_get_type(cfg, var, CFG_VAL_INT);
	if (data == NULL) {
		return 1;
	}

	if (val) {
		*val = data->val.i;
	}

	return 0;
}

int cfg_get_arr(const cfg_t *cfg, cfg_var_t arr, cfg_var_t *var)
{
	const var_data_t *data = cfg_get_type(cfg, arr, CFG_VAL_ARR);
	if (data == NULL || var == NULL) {
		return 1;
	}

	if (*var >= cfg->vars.cnt) {
		*var = data->val.child;
		return 0;
	}

	return list_get_next(&cfg->vars, *var, var) == NULL;
}

typedef enum val_mode_e {
	VAL_MODE_NONE,
	VAL_MODE_NL,
	VAL_MODE_OBJ,
} var_mode_t;

static size_t cfg_print_vars(const cfg_t *cfg, cfg_var_t parent, int tbl, int print_key, dst_t dst);

static size_t cfg_print_var(const cfg_t *cfg, const var_data_t *data, int first, int print_key, dst_t dst)
{
	size_t off = dst.off;

	if (print_key) {
		switch (data->type) {
		case CFG_VAL_LIT:
		case CFG_VAL_STR:
		case CFG_VAL_INT:
		case CFG_VAL_ARR:
		case CFG_VAL_OBJ: {
			strv_t key = strbuf_get(&cfg->strs, data->key);
			dst.off += dputf(dst, "%.*s = ", key.len, key.data);
			break;
		}
		case CFG_VAL_TBL: {
			strv_t key = strbuf_get(&cfg->strs, data->key);
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
	case CFG_VAL_LIT: {
		dst.off += dputs(dst, strbuf_get(&cfg->strs, data->val.str));
		break;
	}
	case CFG_VAL_STR: {
		strv_t str = strbuf_get(&cfg->strs, data->val.str);
		dst.off += dputf(dst, "\"%.*s\"", str.len, str.data);
		break;
	}
	case CFG_VAL_INT: {
		dst.off += dputf(dst, "%d", data->val.i);
		break;
	}
	case CFG_VAL_ARR: {
		dst.off += dputs(dst, STRV("["));
		dst.off += cfg_print_vars(cfg, data->val.child, 0, 0, dst);
		dst.off += dputs(dst, STRV("]"));
		break;
	}
	case CFG_VAL_OBJ: {
		dst.off += dputs(dst, STRV("{"));
		dst.off += cfg_print_vars(cfg, data->val.child, 0, 1, dst);
		dst.off += dputs(dst, STRV("}"));
		break;
	}
	case CFG_VAL_TBL: {
		dst.off += cfg_print_vars(cfg, data->val.child, 1, 1, dst);
		break;
	}
	default: log_error("cparse", "cfg", NULL, "unknown type: %d", data->type); break;
	}

	return dst.off - off;
}

static size_t cfg_print_vars(const cfg_t *cfg, cfg_var_t parent, int tbl, int print_key, dst_t dst)
{
	size_t off = dst.off;

	int first = 1;
	const var_data_t *data;
	list_foreach(&cfg->vars, parent, data)
	{
		if (!first && !tbl) {
			dst.off += dputs(dst, STRV(", "));
		}

		dst.off += cfg_print_var(cfg, data, first, print_key, dst);

		if (tbl && data->type != CFG_VAL_TBL) {
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

	const var_data_t *data = list_get(&cfg->vars, var);
	if (data == NULL) {
		return 0;
	}

	switch (data->type) {
	case CFG_VAL_ROOT:
	case CFG_VAL_TBL: {
		return cfg_print_vars(cfg, data->val.child, 1, 1, dst);
	}
	default: return cfg_print_var(cfg, data, 1, 0, dst);
	}

	return 0;
}
