#include "tok.h"

#include "str.h"

static strv_t tok_type_str[] = {
	[TOK_UNKNOWN] = STRVT("UNKNOWN"),
	[TOK_TAB]     = STRVT("TAB"),
	[TOK_NL]      = STRVT("NL"),
	[TOK_CR]      = STRVT("CR"),
	[TOK_WS]      = STRVT("WS"),
	[TOK_SYMBOL]  = STRVT("SYMBOL"),
	[TOK_QUOTE]   = STRVT("QUOTE"),
	[TOK_PARENT]  = STRVT("PARENT"),
	[TOK_DIGIT]   = STRVT("DIGIT"),
	[TOK_ALPHA]   = STRVT("ALPHA"),
	[TOK_UPPER]   = STRVT("UPPER"),
	[TOK_LOWER]   = STRVT("LOWER"),
	[TOK_WORD]    = STRVT("WORD"),
	[TOK_EOF]     = STRVT("EOF"),
};

size_t tok_type_print(uint type, dst_t dst)
{
	size_t off = dst.off;

	for (tok_type_t i = TOK_UNKNOWN; i < __TOK_MAX; i++) {
		if (type & (1 << i)) {
			if (dst.off > off) {
				dst.off += dputs(dst, STRV("|"));
			}
			dst.off += dputs(dst, tok_type_str[i]);
		}
	}

	if (dst.off == off) {
		dst.off += dputs(dst, tok_type_str[TOK_UNKNOWN]);
	}

	return dst.off - off;
}

tok_type_t tok_type_enum(strv_t str)
{
	for (tok_type_t type = TOK_UNKNOWN; type < __TOK_MAX; type++) {
		if (strv_eq(str, tok_type_str[type])) {
			return type;
		}
	}

	return TOK_UNKNOWN;
}
