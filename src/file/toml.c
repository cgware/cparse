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

	if (tree_init(&toml->vals, vals_cap, sizeof(val_data_t), alloc) == NULL) {
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

	tree_free(&toml->vals);
	strbuf_free(&toml->strs);
}

toml_val_t toml_val_init(toml_t *toml, strv_t key, toml_add_val_t val)
{
	if (toml == NULL) {
		return TOML_VAL_END;
	}

	uint key_id = -1;
	if (key.data) {
		key_id = toml->strs.buf.used;
		if (strbuf_add(&toml->strs, key.data, key.len, NULL)) {
			log_error("cparse", "toml", NULL, "failed to add key");
			return TOML_VAL_END;
		}
	}

	toml_val_t val_id = tree_add(&toml->vals);
	val_data_t *data  = tree_get_data(&toml->vals, val_id);
	if (data == NULL) {
		log_error("cparse", "toml", NULL, "failed to add value");
		return TOML_VAL_END;
	}

	switch (val.type) {
	case TOML_VAL_STRL: {
		uint str_id = toml->strs.buf.used;
		if (strbuf_add(&toml->strs, val.val.str.data, val.val.str.len, NULL)) {
			log_error("cparse", "toml", NULL, "failed to add string");
			return TOML_VAL_END;
		}

		*data = (val_data_t){
			.key	 = key_id,
			.type	 = val.type,
			.val.str = str_id,
		};

		break;
	}
	case TOML_VAL_INT: {
		*data = (val_data_t){
			.key   = key_id,
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
			.key  = key_id,
			.type = val.type,
		};

		break;
	}
	default: {
		log_error("cparse", "toml", NULL, "unknown val type: %d", val.type);
		return TOML_VAL_END;
	}
	}

	return val_id;
}

toml_val_t toml_add_val(toml_t *toml, toml_val_t parent, toml_val_t val)
{
	if (toml == NULL) {
		return TOML_VAL_END;
	}

	return parent >= toml->vals.cnt ? tree_add(&toml->vals) : tree_set_child(&toml->vals, parent, val);
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

static int toml_print_val(const toml_t *toml, toml_val_t val, val_mode_t mode, print_dst_t dst)
{
	int off = dst.off;
	int nl	= 0;

	const val_data_t *data = tree_get_data(&toml->vals, val);

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
		uint start, len;
		strbuf_get(&toml->strs, data->val.str, start, len);
		dst.off += c_dprintf(dst, "'%.*s'", len, &toml->strs.buf.data[start]);
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
		nl = 0;
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

static int toml_print_vals(const toml_t *toml, toml_val_t parent, val_mode_t mode, int top, print_dst_t dst)
{
	int off = dst.off;

	toml_val_t child;
	int first = 1;

	tree_foreach_child(&toml->vals, parent, child)
	{
		if (!first) {
			if (mode == VAL_MODE_NL || mode == VAL_MODE_NL_ARR) {
				const val_data_t *data = tree_get_data(&toml->vals, child);
				if (top && (data->type == TOML_VAL_TBL || data->type == TOML_VAL_TBL_ARR)) {
					dst.off += c_dprintf(dst, "\n");
				}
			} else {
				dst.off += c_dprintf(dst, ", ");
			}
		}

		dst.off += toml_print_val(toml, child, mode, dst);

		first = 0;
	}

	return dst.off - off;
}

int toml_print(const toml_t *toml, toml_val_t val, print_dst_t dst)
{
	if (toml == NULL) {
		return 0;
	}

	return toml_print_vals(toml, val, VAL_MODE_NL, 1, dst);
}
