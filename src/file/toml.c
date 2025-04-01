#include "file/toml.h"

#include "log.h"

typedef struct var_data_s {
	uint key;
	toml_val_type_t type;
	union {
		toml_var_t child;
		uint str;
		int i;
	} val;
} var_data_t;

toml_t *toml_init(toml_t *toml, uint strs_cap, uint vars_cap, alloc_t alloc)
{
	if (toml == NULL) {
		return NULL;
	}

	if (strbuf_init(&toml->strs, strs_cap, strs_cap * 8, alloc) == NULL) {
		log_error("cparse", "toml", NULL, "failed to initialize stings");
		return NULL;
	}

	if (list_init(&toml->vars, vars_cap, sizeof(var_data_t), alloc) == NULL) {
		log_error("cparse", "toml", NULL, "failed to initialize values");
		return NULL;
	}

	return toml;
}

void toml_free(toml_t *toml)
{
	if (toml == NULL) {
		return;
	}

	list_free(&toml->vars);
	strbuf_free(&toml->strs);
}

toml_var_t toml_var_init(toml_t *toml, strv_t key, toml_val_t val)
{
	if (toml == NULL) {
		return TOML_VAL_END;
	}

	uint key_id = -1;
	if (key.data) {
		if (strbuf_add(&toml->strs, key, &key_id)) {
			log_error("cparse", "toml", NULL, "failed to add key");
			return TOML_VAL_END;
		}
	}

	toml_var_t var_id = list_add(&toml->vars);
	var_data_t *data  = list_get_data(&toml->vars, var_id);
	if (data == NULL) {
		log_error("cparse", "toml", NULL, "failed to add value");
		return TOML_VAL_END;
	}

	switch (val.type) {
	case TOML_VAL_STRL: {
		uint str_id;
		if (strbuf_add(&toml->strs, val.val.str, &str_id)) {
			log_error("cparse", "toml", NULL, "failed to add string");
			return TOML_VAL_END;
		}

		*data = (var_data_t){
			.key	 = key_id,
			.type	 = val.type,
			.val.str = str_id,
		};

		break;
	}
	case TOML_VAL_INT: {
		*data = (var_data_t){
			.key   = key_id,
			.type  = val.type,
			.val.i = val.val.i,
		};

		break;
	}
	case TOML_VAL_ROOT:
	case TOML_VAL_ARR:
	case TOML_VAL_INL:
	case TOML_VAL_TBL:
	case TOML_VAL_TBL_ARR: {
		*data = (var_data_t){
			.key	   = key_id,
			.type	   = val.type,
			.val.child = LIST_END,
		};

		break;
	}
	default: {
		log_error("cparse", "toml", NULL, "unknown val type: %d", val.type);
		return TOML_VAL_END;
	}
	}

	return var_id;
}

toml_var_t toml_add_var(toml_t *toml, toml_var_t parent, toml_var_t var)
{
	if (toml == NULL) {
		return TOML_VAL_END;
	}

	var_data_t *data = list_get_data(&toml->vars, parent);
	if (data == NULL) {
		return TOML_VAL_END;
	}

	switch (data->type) {
	case TOML_VAL_ROOT:
	case TOML_VAL_ARR:
	case TOML_VAL_INL:
	case TOML_VAL_TBL:
	case TOML_VAL_TBL_ARR: break;
	default: return TOML_VAL_END;
	}

	return list_set_next_node(&toml->vars, data->val.child, var);
}

static const char *val_type_str[] = {
	[TOML_VAL_UNKNOWN] = "unknown",
	[TOML_VAL_ROOT]	   = "root",
	[TOML_VAL_STRL]	   = "string",
	[TOML_VAL_INT]	   = "int",
	[TOML_VAL_ARR]	   = "array",
	[TOML_VAL_INL]	   = "inline",
	[TOML_VAL_TBL]	   = "table",
	[TOML_VAL_TBL_ARR] = "table array",
};

static const char *val_type_to_str(toml_val_type_t type)
{
	if (type < TOML_VAL_UNKNOWN || type > TOML_VAL_TBL_ARR) {
		type = TOML_VAL_UNKNOWN; // LCOV_EXCL_LINE
	}

	return val_type_str[type];
}

int toml_get_var(const toml_t *toml, toml_var_t parent, strv_t key, toml_var_t *var)
{
	if (toml == NULL) {
		return 1;
	}

	uint index;
	if (strbuf_get_index(&toml->strs, key, &index)) {
		return 1;
	}

	const var_data_t *data = list_get_data(&toml->vars, parent);
	switch (data->type) {
	case TOML_VAL_ROOT:
	case TOML_VAL_ARR:
	case TOML_VAL_INL:
	case TOML_VAL_TBL:
	case TOML_VAL_TBL_ARR: break;
	default: return 1;
	}

	list_foreach(&toml->vars, data->val.child, data)
	{
		if (data->key == index) {
			if (var) {
				*var = _i;
			}
			return 0;
		}
	}

	return 1;
}

const var_data_t *toml_get_type(const toml_t *toml, toml_var_t var, toml_val_type_t type)
{
	if (toml == NULL) {
		return NULL;
	}

	const var_data_t *data = list_get_data(&toml->vars, var);
	if (data == NULL) {
		return NULL;
	}

	if (data->type != type) {
		log_error("cparse", "toml", NULL, "expected %s, got %s", val_type_to_str(type), val_type_to_str(data->type));
		return NULL;
	}

	return data;
}

int toml_get_str(const toml_t *toml, toml_var_t var, strv_t *val)
{
	const var_data_t *data = toml_get_type(toml, var, TOML_VAL_STRL);
	if (data == NULL) {
		return 1;
	}

	if (val) {
		*val = strbuf_get(&toml->strs, data->val.str);
	}

	return 0;
}

int toml_get_int(const toml_t *toml, toml_var_t var, int *val)
{
	const var_data_t *data = toml_get_type(toml, var, TOML_VAL_INT);
	if (data == NULL) {
		return 1;
	}

	if (val) {
		*val = data->val.i;
	}

	return 0;
}

int toml_get_arr(const toml_t *toml, toml_var_t arr, toml_var_t *var)
{
	const var_data_t *data = toml_get_type(toml, arr, TOML_VAL_ARR);
	if (data == NULL || var == NULL) {
		return 1;
	}

	if (*var >= toml->vars.cnt) {
		*var = data->val.child;
		return 0;
	}

	*var = list_get_next(&toml->vars, *var);
	if (*var >= toml->vars.cnt) {
		return 1;
	}

	return 0;
}

typedef enum val_mode_e {
	VAL_MODE_NONE,
	VAL_MODE_NL,
	VAL_MODE_NL_ARR,
	VAL_MODE_INL,
} var_mode_t;

static int toml_print_key(const toml_t *toml, const var_data_t *data, var_mode_t mode, print_dst_t dst)
{
	int off = dst.off;
	strv_t key;

	switch (mode) {
	case VAL_MODE_NONE: break;
	case VAL_MODE_NL:
		key = strbuf_get(&toml->strs, data->key);
		dst.off += c_dprintf(dst, "[%.*s]\n", key.len, key.data);
		break;
	case VAL_MODE_NL_ARR:
		key = strbuf_get(&toml->strs, data->key);
		dst.off += c_dprintf(dst, "[[%.*s]]\n", key.len, key.data);
		break;
	case VAL_MODE_INL:
		key = strbuf_get(&toml->strs, data->key);
		dst.off += c_dprintf(dst, "%.*s = ", key.len, key.data);
		break;
	}

	return dst.off - off;
}

static int toml_print_vars(const toml_t *toml, toml_var_t parent, var_mode_t mode, int top, print_dst_t dst);

static int toml_print_var(const toml_t *toml, toml_var_t var, var_mode_t mode, print_dst_t dst)
{
	int off = dst.off;
	int nl	= 0;

	const var_data_t *data = list_get_data(&toml->vars, var);

	switch (data->type) {
	case TOML_VAL_STRL:
	case TOML_VAL_INT:
	case TOML_VAL_ARR:
	case TOML_VAL_INL: dst.off += toml_print_key(toml, data, mode == VAL_MODE_NONE ? VAL_MODE_NONE : VAL_MODE_INL, dst); break;
	case TOML_VAL_TBL: dst.off += toml_print_key(toml, data, mode, dst); break;
	case TOML_VAL_TBL_ARR: dst.off += toml_print_key(toml, data, mode == VAL_MODE_NL ? VAL_MODE_NL_ARR : mode, dst); break;
	default: log_error("cparse", "toml", NULL, "unknown type: %d", data->type); break;
	}

	switch (data->type) {
	case TOML_VAL_STRL: {
		strv_t str = strbuf_get(&toml->strs, data->val.str);
		dst.off += c_dprintf(dst, "'%.*s'", str.len, str.data);
		nl = 1;
		break;
	}
	case TOML_VAL_INT: {
		dst.off += c_dprintf(dst, "%d", data->val.i);
		nl = 1;
		break;
	}
	case TOML_VAL_ARR: {
		dst.off += c_dprintf(dst, "[");
		dst.off += toml_print_vars(toml, data->val.child, VAL_MODE_NONE, 0, dst);
		dst.off += c_dprintf(dst, "]");
		nl = 1;
		break;
	}
	case TOML_VAL_INL: {
		dst.off += c_dprintf(dst, "{");
		dst.off += toml_print_vars(toml, data->val.child, VAL_MODE_INL, 0, dst);
		dst.off += c_dprintf(dst, "}");
		nl = 1;
		break;
	}
	case TOML_VAL_TBL: {
		if (mode == VAL_MODE_NL) {
			dst.off += toml_print_vars(toml, data->val.child, VAL_MODE_NL, 0, dst);
		} else {
			dst.off += c_dprintf(dst, "{");
			dst.off += toml_print_vars(toml, data->val.child, VAL_MODE_INL, 0, dst);
			dst.off += c_dprintf(dst, "}");
		}
		nl = 0;
		break;
	}
	case TOML_VAL_TBL_ARR: {
		if (mode == VAL_MODE_NL) {
			dst.off += toml_print_vars(toml, data->val.child, VAL_MODE_NL, 0, dst);
		} else {
			dst.off += c_dprintf(dst, "[");
			dst.off += toml_print_vars(toml, data->val.child, VAL_MODE_NONE, 0, dst);
			dst.off += c_dprintf(dst, "]");
		}
		nl = 0;
		break;
	}
	default: log_error("cparse", "toml", NULL, "unknown type: %d", data->type); break;
	}

	if (mode == VAL_MODE_NL && nl) {
		dst.off += c_dprintf(dst, "\n");
	}

	return dst.off - off;
}

static int toml_print_vars(const toml_t *toml, toml_var_t parent, var_mode_t mode, int top, print_dst_t dst)
{
	int off = dst.off;

	int first = 1;
	const var_data_t *data;
	list_foreach(&toml->vars, parent, data)
	{
		if (!first) {
			if (mode == VAL_MODE_NL || mode == VAL_MODE_NL_ARR) {
				if (top && (data->type == TOML_VAL_TBL || data->type == TOML_VAL_TBL_ARR)) {
					dst.off += c_dprintf(dst, "\n");
				}
			} else {
				dst.off += c_dprintf(dst, ", ");
			}
		}

		dst.off += toml_print_var(toml, _i, mode, dst);

		first = 0;
	}

	return dst.off - off;
}

int toml_print(const toml_t *toml, toml_var_t var, print_dst_t dst)
{
	if (toml == NULL) {
		return 0;
	}

	const var_data_t *data = list_get_data(&toml->vars, var);
	if (data == NULL) {
		return 0;
	}

	switch (data->type) {
	case TOML_VAL_ROOT:
	case TOML_VAL_ARR:
	case TOML_VAL_INL:
	case TOML_VAL_TBL:
	case TOML_VAL_TBL_ARR: {
		return toml_print_vars(toml, data->val.child, VAL_MODE_NL, 1, dst);
		break;
	}
	default: return toml_print_var(toml, var, VAL_MODE_NL, dst);
	}

	return 0;
}
