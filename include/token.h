#ifndef TOKEN_H
#define TOKEN_H

#include "print.h"
#include "str.h"
#include "type.h"

typedef enum token_type_e {
	TOKEN_UNKNOWN,

	TOKEN_ALPHA,
	TOKEN_UPPER,
	TOKEN_LOWER,

	TOKEN_DIGIT,

	TOKEN_SQUOTE,
	TOKEN_DQUOTE,

	TOKEN_COMMA,

	TOKEN_WS,
	TOKEN_SPACE,
	TOKEN_TAB,
	TOKEN_CR,
	TOKEN_NL,

	TOKEN_NULL,

	TOKEN_SYMBOL,

	TOKEN_EOF,

	__TOKEN_MAX,
} token_type_t;

typedef struct token_s {
	uint start;
	uint len;
	uint type;
} token_t;

typedef struct token_loc_s {
	uint line_off;
	uint line_len;
	uint line_nr;
	uint col;
} token_loc_t;

str_t token_type_str(token_type_t type);
token_type_t token_type_enum(str_t str);

#endif
