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
	str_t *src;
	str_t file;
	uint line_off;
	strbuf_t words;
	arr_t tokens;
} lex_t;

lex_t *lex_init(lex_t *lex, uint words_cap, uint tokens_cap, alloc_t alloc);
void lex_free(lex_t *lex);

#define lex_add_word(_lex, _str, _index) strbuf_add(&(_lex)->words, _str, _index)

int lex_add_token(lex_t *lex, token_type_t type, strv_t val, uint *index);
#define lex_get_token(_lex, _index)                                                                                                        \
	(_index < (_lex)->tokens.cnt ? *(token_t *)arr_get(&(_lex)->tokens, _index) : ((token_t){.type = (1 << TOKEN_EOF)}))
strv_t lex_get_token_val(const lex_t *lex, token_t token);
token_loc_t lex_get_token_loc(const lex_t *lex, uint index);

void lex_set_src(lex_t *lex, str_t *src, str_t file, uint line_off);
int lex_tokenize(lex_t *lex, str_t *src, str_t file, uint line_off);

int lex_print_token(const lex_t *lex, token_t toc, print_dst_t dst);
int lex_print(const lex_t *lex, print_dst_t dst);

int lex_token_loc_print_loc(const lex_t *lex, token_loc_t loc, print_dst_t dst);
int lex_token_loc_print_src(const lex_t *lex, token_loc_t loc, print_dst_t dst);

#endif
