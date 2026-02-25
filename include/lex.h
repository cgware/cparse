#ifndef LEX_H
#define LEX_H

#include "arr.h"
#include "str.h"
#include "strbuf.h"
#include "tok.h"

typedef struct lex_s {
	const uint *chars;
	uint chars_len;
	strv_t src;
	strv_t file;
	uint line_off;
	strbuf_t words;
	arr_t toks;
} lex_t;

lex_t *lex_init(lex_t *lex, uint words_cap, uint toks_cap, alloc_t alloc);
void lex_free(lex_t *lex);

void lex_reset(lex_t *lex);

#define lex_add_word(_lex, _str, _index) strbuf_add(&(_lex)->words, _str, _index)

#define lex_get_tok(_lex, _index) (_index < (_lex)->toks.cnt ? *(tok_t *)arr_get(&(_lex)->toks, _index) : ((tok_t){.type = (1 << TOK_EOF)}))
strv_t lex_get_tok_val(const lex_t *lex, tok_t tok);
tok_loc_t lex_get_tok_loc(const lex_t *lex, uint index);

void lex_set_src(lex_t *lex, strv_t src, strv_t file, uint line_off);
int lex_tokenize(lex_t *lex, strv_t src, strv_t file, uint line_off);

size_t lex_print_tok(const lex_t *lex, tok_t toc, dst_t dst);
size_t lex_print(const lex_t *lex, dst_t dst);

size_t lex_tok_loc_print_loc(const lex_t *lex, tok_loc_t loc, dst_t dst);
size_t lex_tok_loc_print_src(const lex_t *lex, tok_loc_t loc, dst_t dst);

#endif
