#include "file/toml.h"

#include "log.h"

typedef struct val_data_s {
	uint key;
	toml_val_type_t type;
	union {
		uint str;
		int i;
	} val;
} val_data_t;

toml_t *toml_init(toml_t *toml, size_t strs_size, size_t vals_cap, alloc_t alloc)
{
	if (toml == NULL) {
		return NULL;
	}

	if (strbuf_init(&toml->strs, strs_size, alloc) == NULL) {
		log_error("cparse", "toml", NULL, "failed to initialize stings");
		return NULL;
	}

	if (tree_init(&toml->vals, vals_cap + 1, sizeof(val_data_t), alloc) == NULL) {
		log_error("cparse", "toml", NULL, "failed to initialize values");
		return NULL;
	}

	toml->root = tree_add(&toml->vals);

	return toml;
}

void toml_free(toml_t *toml)
{
	if (toml == NULL) {
		return;
	}

	tree_free(&toml->vals);
	strbuf_free(&toml->strs);
}

static int create_data(toml_t *toml, uint key, toml_add_val_t val, val_data_t *data)
{
	switch (val.type) {
	case TOML_VAL_NONE: return 2;
	case TOML_VAL_STR: {
		uint str_id = toml->strs.buf.used;
		if (strbuf_add(&toml->strs, val.val.str.data, val.val.str.len, NULL)) {
			log_error("cparse", "toml", NULL, "failed to add string");
			return 1;
		}

		*data = (val_data_t){
			.key	 = key,
			.type	 = val.type,
			.val.str = str_id,
		};

		break;
	}
	case TOML_VAL_INT: {
		*data = (val_data_t){
			.key   = key,
			.type  = val.type,
			.val.i = val.val.i,
		};

		break;
	}
	case TOML_VAL_ARR:
	case TOML_VAL_INL:
	case TOML_VAL_TBL:
	case TOML_VAL_TBL_ARR: {
		*data = (val_data_t){
			.key  = key,
			.type = val.type,
		};

		break;
	}
	default: {
		log_error("cparse", "toml", NULL, "unknown val type: %d", val.type);
		return 1;
	}
	}

	return 0;
}

static int add_data(tree_t *vals, val_data_t val, toml_val_t id)
{
	val_data_t *data = tree_get_data(vals, id);
	if (data == NULL) {
		log_error("cparse", "toml", NULL, "failed to add value");
		return 1;
	}

	*data = val;

	return 0;
}

int toml_add_val(toml_t *toml, strv_t key, toml_add_val_t val, toml_val_t *id)
{
	if (toml == NULL) {
		return 1;
	}

	uint key_id = toml->strs.buf.used;
	if (strbuf_add(&toml->strs, key.data, key.len, NULL)) {
		log_error("cparse", "toml", NULL, "failed to add key");
		return 1;
	}

	val_data_t data;
	switch (create_data(toml, key_id, val, &data)) {
	case 1: return 1;
	case 2: return 0;
	}

	toml_val_t val_id = tree_add_child(&toml->vals, toml->root);
	if (add_data(&toml->vals, data, val_id)) {
		return 1;
	}

	if (id) {
		*id = val_id;
	}

	return 0;
}

int toml_val_add_val(toml_t *toml, toml_val_t parent, uint key, toml_add_val_t val, toml_val_t *id)
{
	if (toml == NULL) {
		return 1;
	}

	val_data_t data;
	int res = create_data(toml, key, val, &data);
	switch (res) {
	case 1: return 1;
	case 2: return 0;
	}

	val_data_t *arr_data = tree_get_data(&toml->vals, parent);
	if (arr_data == NULL) {
		log_error("cparse", "toml", NULL, "failed to get parent");
		return 1;
	}

	toml_val_t val_id = tree_add_child(&toml->vals, parent);

	if (add_data(&toml->vals, data, val_id)) {
		return 1;
	}

	if (id) {
		*id = val_id;
	}

	return 0;
}

int toml_arr_add_val(toml_t *toml, toml_val_t arr, toml_add_val_t val, toml_val_t *id)
{
	return toml_val_add_val(toml, arr, arr, val, id);
}

int toml_tbl_add_val(toml_t *toml, toml_val_t tbl, strv_t key, toml_add_val_t val, toml_val_t *id)
{
	if (toml == NULL) {
		return 1;
	}

	uint key_id = toml->strs.buf.used;
	if (strbuf_add(&toml->strs, key.data, key.len, NULL)) {
		log_error("cparse", "toml", NULL, "failed to add key");
		return 1;
	}

	return toml_val_add_val(toml, tbl, key_id, val, id);
}

typedef enum val_mode_e {
	VAL_MODE_NONE,
	VAL_MODE_NL,
	VAL_MODE_NL_ARR,
	VAL_MODE_INL,
} val_mode_t;

static int toml_print_key(const toml_t *toml, const val_data_t *data, val_mode_t mode, print_dst_t dst)
{
	int off = dst.off;
	uint start, len;

	switch (mode) {
	case VAL_MODE_NONE: break;
	case VAL_MODE_NL:
		strbuf_get(&toml->strs, data->key, start, len);
		dst.off += c_dprintf(dst, "[%.*s]\n", len, &toml->strs.buf.data[start]);
		break;
	case VAL_MODE_NL_ARR:
		strbuf_get(&toml->strs, data->key, start, len);
		dst.off += c_dprintf(dst, "[[%.*s]]\n", len, &toml->strs.buf.data[start]);
		break;
	case VAL_MODE_INL:
		strbuf_get(&toml->strs, data->key, start, len);
		dst.off += c_dprintf(dst, "%.*s = ", len, &toml->strs.buf.data[start]);
		break;
	}

	return dst.off - off;
}

static int toml_print_vals(const toml_t *toml, toml_val_t parent, val_mode_t mode, int top, print_dst_t dst);

static int toml_print_val(const toml_t *toml, toml_val_t val, val_mode_t mode, int top, print_dst_t dst)
{
	int off = dst.off;
	int nl	= 0;

	const val_data_t *data = tree_get_data(&toml->vals, val);

	switch (data->type) {
	case TOML_VAL_STR:
	case TOML_VAL_INT:
	case TOML_VAL_ARR:
	case TOML_VAL_INL: dst.off += toml_print_key(toml, data, mode == VAL_MODE_NONE ? VAL_MODE_NONE : VAL_MODE_INL, dst); break;
	case TOML_VAL_TBL: dst.off += toml_print_key(toml, data, mode, dst); break;
	case TOML_VAL_TBL_ARR: dst.off += toml_print_key(toml, data, mode == VAL_MODE_NL ? VAL_MODE_NL_ARR : mode, dst); break;
	default: log_error("cparse", "toml", NULL, "unknown type: %d", data->type); break;
	}

	switch (data->type) {
	case TOML_VAL_STR: {
		uint start, len;
		strbuf_get(&toml->strs, data->val.str, start, len);
		dst.off += c_dprintf(dst, "\"%.*s\"", len, &toml->strs.buf.data[start]);
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
		dst.off += toml_print_vals(toml, val, VAL_MODE_NONE, 0, dst);
		dst.off += c_dprintf(dst, "]");
		nl = 1;
		break;
	}
	case TOML_VAL_INL: {
		dst.off += c_dprintf(dst, "{");
		dst.off += toml_print_vals(toml, val, VAL_MODE_INL, 0, dst);
		dst.off += c_dprintf(dst, "}");
		nl = 1;
		break;
	}
	case TOML_VAL_TBL: {
		if (mode == VAL_MODE_NL) {
			dst.off += toml_print_vals(toml, val, VAL_MODE_NL, 0, dst);
		} else {
			dst.off += c_dprintf(dst, "{");
			dst.off += toml_print_vals(toml, val, VAL_MODE_INL, 0, dst);
			dst.off += c_dprintf(dst, "}");
		}
		nl = top;
		break;
	}
	case TOML_VAL_TBL_ARR: {
		if (mode == VAL_MODE_NL) {
			dst.off += toml_print_vals(toml, val, VAL_MODE_NL, 0, dst);
		} else {
			dst.off += c_dprintf(dst, "[");
			dst.off += toml_print_vals(toml, val, VAL_MODE_NONE, 0, dst);
			dst.off += c_dprintf(dst, "]");
		}
		nl = top;
		break;
	}
	default: log_error("cparse", "toml", NULL, "unknown type: %d", data->type); break;
	}

	if (mode == VAL_MODE_NL && nl) {
		dst.off += c_dprintf(dst, "\n");
	}

	return dst.off - off;
}

static int toml_print_vals(const toml_t *toml, toml_val_t parent, val_mode_t mode, int top, print_dst_t dst)
{
	int off = dst.off;

	toml_val_t child;
	int first;

	tree_foreach_child(&toml->vals, parent, child)
	{
		if (mode != 1 && !first) {
			dst.off += c_dprintf(dst, ", ");
		}

		dst.off += toml_print_val(toml, child, mode, top, dst);

		first = 0;
	}

	return dst.off - off;
}

int toml_print(const toml_t *toml, print_dst_t dst)
{
	if (toml == NULL) {
		return 0;
	}

	return toml_print_vals(toml, toml->root, VAL_MODE_NL, 1, dst);
}
