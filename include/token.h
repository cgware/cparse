#ifndef TOKEN_H
#define TOKEN_H

#include "print.h"
#include "strv.h"
#include "type.h"

typedef enum token_type_e {
	TOKEN_UNKNOWN,
	TOKEN_NULL,
	TOKEN_TAB,
	TOKEN_NL,
	TOKEN_CR,
	TOKEN_WS,
	TOKEN_SYMBOL,
	TOKEN_QUOTE,
	TOKEN_PARENT,
	TOKEN_DIGIT,
	TOKEN_ALPHA,
	TOKEN_UPPER,
	TOKEN_LOWER,
	TOKEN_WORD,
	TOKEN_EOF,
	__TOKEN_MAX,
} token_type_t;

typedef struct token_s {
	uint type;
	uint start;
	uint len;
} token_t;

typedef struct token_loc_s {
	uint line_off;
	uint line_len;
	uint line_nr;
	uint col;
} token_loc_t;

int token_type_print(uint type, print_dst_t dst);
token_type_t token_type_enum(strv_t str);

#endif
