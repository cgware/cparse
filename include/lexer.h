#ifndef LEXER_H
#define LEXER_H

#include "arr.h"
#include "str.h"
#include "strbuf.h"
#include "token.h"

#define LEX_TOKEN_END ARR_END

typedef struct lex_s {
	const uint *chars;
	uint chars_len;
	strbuf_t words;
	str_t file;
	str_t *src;
	uint line_off;
	arr_t tokens;
} lex_t;

lex_t *lex_init(lex_t *lex, str_t file, str_t *src, uint line_off, size_t words_size, uint tokens_cap, alloc_t alloc);
void lex_free(lex_t *lex);

#define lex_add_word(_lex, _str, _len, _index) strbuf_add(&(_lex)->words, _str, _len, _index)

int lex_add_token(lex_t *lex, token_type_t type, str_t val, uint *index);
static inline token_t lex_get_token(const lex_t *lex, uint index)
{
	token_t *token = index < lex->src->len ? arr_get(&lex->tokens, index) : NULL;
	return token == NULL ? (token_t){.type = (1 << TOKEN_EOF)} : *token;
}

#define lex_get_token_val(_lex, _token) strc(&(_lex)->src->data[(_token)->start], (_token)->len)
token_loc_t lex_get_token_loc(const lex_t *lex, uint index);

void lex_tokenize(lex_t *lex);

int lex_print_token(const lex_t *lex, token_t toc, print_dst_t dst);
int lex_print(const lex_t *lex, print_dst_t dst);

int lex_token_loc_print_loc(const lex_t *lex, token_loc_t loc, print_dst_t dst);
int lex_token_loc_print_src(const lex_t *lex, token_loc_t loc, print_dst_t dst);

#endif
