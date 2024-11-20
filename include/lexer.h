#ifndef LEXER_H
#define LEXER_H

#include "arr.h"
#include "print.h"
#include "str.h"
#include "token.h"

#define LEX_TOKEN_END ARR_END

typedef struct lex_s {
	str_t file;
	str_t src;
	uint line_off;
} lex_t;

lex_t *lex_init(lex_t *lex, str_t file, str_t src, uint line_off);
void lex_free(lex_t *lex);

token_t lex_get_token(const lex_t *lex, uint index);
str_t lex_get_token_val(const lex_t *lex, token_t token);
token_loc_t lex_get_token_loc(const lex_t *lex, uint index);
int lex_token_loc_print_loc(const lex_t *lex, token_loc_t loc, print_dst_t dst);
int lex_token_loc_print_src(const lex_t *lex, token_loc_t loc, print_dst_t dst);

#endif
