#include "token.h"

#include "str.h"

static str_t token_type_str[] = {
	[TOKEN_UNKNOWN] = STRS("UNKNOWN"),
	[TOKEN_TAB]	= STRS("TAB"),
	[TOKEN_NL]	= STRS("NL"),
	[TOKEN_CR]	= STRS("CR"),
	[TOKEN_WS]	= STRS("WS"),
	[TOKEN_SYMBOL]	= STRS("SYMBOL"),
	[TOKEN_QUOTE]	= STRS("QUOTE"),
	[TOKEN_PARENT]	= STRS("PARENT"),
	[TOKEN_DIGIT]	= STRS("DIGIT"),
	[TOKEN_ALPHA]	= STRS("ALPHA"),
	[TOKEN_UPPER]	= STRS("UPPER"),
	[TOKEN_LOWER]	= STRS("LOWER"),
	[TOKEN_WORD]	= STRS("WORD"),
	[TOKEN_EOF]	= STRS("EOF"),
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

token_type_t token_type_enum(str_t str)
{
	for (token_type_t type = TOKEN_UNKNOWN; type < __TOKEN_MAX; type++) {
		if (str_eq(str, token_type_str[type])) {
			return type;
		}
	}

	return TOKEN_UNKNOWN;
}
