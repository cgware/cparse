#ifndef TOK_H
#define TOK_H

#include "print.h"
#include "strv.h"
#include "type.h"

typedef enum tok_type_e {
	TOK_UNKNOWN,
	TOK_NULL,
	TOK_TAB,
	TOK_NL,
	TOK_CR,
	TOK_WS,
	TOK_SYMBOL,
	TOK_QUOTE,
	TOK_PARENT,
	TOK_DIGIT,
	TOK_ALPHA,
	TOK_UPPER,
	TOK_LOWER,
	TOK_WORD,
	TOK_EOF,
	__TOK_MAX,
} tok_type_t;

typedef struct tok_s {
	uint type;
	uint len;
	size_t start;
} tok_t;

typedef struct tok_loc_s {
	uint line_off;
	uint line_len;
	uint line_nr;
	uint col;
} tok_loc_t;

size_t tok_type_print(uint type, dst_t dst);
tok_type_t tok_type_enum(strv_t str);

#endif
