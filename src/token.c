#include "token.h"

#include "str.h"

static strv_t token_type_str[] = {
	[TOKEN_UNKNOWN] = STRVT("UNKNOWN"),
	[TOKEN_TAB]	= STRVT("TAB"),
	[TOKEN_NL]	= STRVT("NL"),
	[TOKEN_CR]	= STRVT("CR"),
	[TOKEN_WS]	= STRVT("WS"),
	[TOKEN_SYMBOL]	= STRVT("SYMBOL"),
	[TOKEN_QUOTE]	= STRVT("QUOTE"),
	[TOKEN_PARENT]	= STRVT("PARENT"),
	[TOKEN_DIGIT]	= STRVT("DIGIT"),
	[TOKEN_ALPHA]	= STRVT("ALPHA"),
	[TOKEN_UPPER]	= STRVT("UPPER"),
	[TOKEN_LOWER]	= STRVT("LOWER"),
	[TOKEN_WORD]	= STRVT("WORD"),
	[TOKEN_EOF]	= STRVT("EOF"),
};

int token_type_print(uint type, print_dst_t dst)
{
	int off = dst.off;

	for (token_type_t i = TOKEN_UNKNOWN; i < __TOKEN_MAX; i++) {
		if (type & (1 << i)) {
			dst.off += c_dprintf(dst, off == dst.off ? "%.*s" : "|%.*s", token_type_str[i].len, token_type_str[i].data);
		}
	}

	if (dst.off == off) {
		dst.off += c_dprintf(dst, "%.*s", token_type_str[TOKEN_UNKNOWN].len, token_type_str[TOKEN_UNKNOWN].data);
	}

	return dst.off - off;
}

token_type_t token_type_enum(strv_t str)
{
	for (token_type_t type = TOKEN_UNKNOWN; type < __TOKEN_MAX; type++) {
		if (strv_eq(str, token_type_str[type])) {
			return type;
		}
	}

	return TOKEN_UNKNOWN;
}
