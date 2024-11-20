#include "token.h"

#include <stdarg.h>

static str_t token_type_strs[] = {
	[TOKEN_UNKNOWN] = STRS("UNKNOWN"),
	[TOKEN_ALPHA]	= STRS("ALPHA"),
	[TOKEN_UPPER]	= STRS("UPPER"),
	[TOKEN_LOWER]	= STRS("LOWER"),
	[TOKEN_DIGIT]	= STRS("DIGIT"),
	[TOKEN_SQUOTE]	= STRS("SINGLE_QUOTE"),
	[TOKEN_DQUOTE]	= STRS("DOUBLE_QUOTE"),
	[TOKEN_COMMA]	= STRS("COMMA"),
	[TOKEN_WS]	= STRS("WS"),
	[TOKEN_SPACE]	= STRS("SPACE"),
	[TOKEN_TAB]	= STRS("TAB"),
	[TOKEN_CR]	= STRS("CR"),
	[TOKEN_NL]	= STRS("NL"),
	[TOKEN_NULL]	= STRS("NULL"),
	[TOKEN_SYMBOL]	= STRS("SYMBOL"),
	[TOKEN_EOF]	= STRS("EOF"),
};

str_t token_type_str(token_type_t type)
{
	return token_type_strs[type >= TOKEN_UNKNOWN && type < __TOKEN_MAX ? type : TOKEN_UNKNOWN];
}

token_type_t token_type_enum(str_t str)
{
	for (token_type_t type = TOKEN_UNKNOWN; type < __TOKEN_MAX; type++) {
		if (str_eq(str, token_type_strs[type])) {
			return type;
		}
	}

	return TOKEN_UNKNOWN;
}
