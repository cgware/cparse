#include "token.h"

#include "str.h"

static strv_t token_type_str[] = {
	[TOKEN_UNKNOWN] = STRVS("UNKNOWN"),
	[TOKEN_TAB]	= STRVS("TAB"),
	[TOKEN_NL]	= STRVS("NL"),
	[TOKEN_CR]	= STRVS("CR"),
	[TOKEN_WS]	= STRVS("WS"),
	[TOKEN_SYMBOL]	= STRVS("SYMBOL"),
	[TOKEN_QUOTE]	= STRVS("QUOTE"),
	[TOKEN_PARENT]	= STRVS("PARENT"),
	[TOKEN_DIGIT]	= STRVS("DIGIT"),
	[TOKEN_ALPHA]	= STRVS("ALPHA"),
	[TOKEN_UPPER]	= STRVS("UPPER"),
	[TOKEN_LOWER]	= STRVS("LOWER"),
	[TOKEN_WORD]	= STRVS("WORD"),
	[TOKEN_EOF]	= STRVS("EOF"),
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
