#include "lex.h"

#include "mem.h"
#include "tok.h"

typedef struct word_s {
	uint start;
	uint len;
} word_t;

static const uint s_chars[128] = {
	['\0'] = (1 << TOK_NULL),
	['\t'] = (1 << TOK_WS) | (1 << TOK_TAB),
	['\n'] = (1 << TOK_WS) | (1 << TOK_NL),
	['\r'] = (1 << TOK_WS) | (1 << TOK_CR),
	[' ']  = (1 << TOK_WS),
	['!']  = (1 << TOK_SYMBOL),
	['"']  = (1 << TOK_QUOTE),
	['#']  = (1 << TOK_SYMBOL),
	['$']  = (1 << TOK_SYMBOL),
	['%']  = (1 << TOK_SYMBOL),
	['&']  = (1 << TOK_SYMBOL),
	['\''] = (1 << TOK_QUOTE),
	['(']  = (1 << TOK_SYMBOL) | (1 << TOK_PARENT),
	[')']  = (1 << TOK_SYMBOL) | (1 << TOK_PARENT),
	['*']  = (1 << TOK_SYMBOL),
	['+']  = (1 << TOK_SYMBOL),
	[',']  = (1 << TOK_SYMBOL),
	['-']  = (1 << TOK_SYMBOL),
	['.']  = (1 << TOK_SYMBOL),
	['/']  = (1 << TOK_SYMBOL),
	['0']  = (1 << TOK_DIGIT),
	['1']  = (1 << TOK_DIGIT),
	['2']  = (1 << TOK_DIGIT),
	['3']  = (1 << TOK_DIGIT),
	['4']  = (1 << TOK_DIGIT),
	['5']  = (1 << TOK_DIGIT),
	['6']  = (1 << TOK_DIGIT),
	['7']  = (1 << TOK_DIGIT),
	['8']  = (1 << TOK_DIGIT),
	['9']  = (1 << TOK_DIGIT),
	[':']  = (1 << TOK_SYMBOL),
	[';']  = (1 << TOK_SYMBOL),
	['<']  = (1 << TOK_SYMBOL),
	['=']  = (1 << TOK_SYMBOL),
	['>']  = (1 << TOK_SYMBOL),
	['?']  = (1 << TOK_SYMBOL),
	['@']  = (1 << TOK_SYMBOL),
	['A']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['B']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['C']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['D']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['E']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['F']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['G']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['H']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['I']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['J']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['K']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['L']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['M']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['N']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['O']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['P']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['Q']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['R']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['S']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['T']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['U']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['V']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['W']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['X']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['Y']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['Z']  = (1 << TOK_ALPHA) | (1 << TOK_UPPER),
	['[']  = (1 << TOK_SYMBOL) | (1 << TOK_PARENT),
	['\\'] = (1 << TOK_SYMBOL),
	[']']  = (1 << TOK_SYMBOL) | (1 << TOK_PARENT),
	['^']  = (1 << TOK_SYMBOL),
	['_']  = (1 << TOK_SYMBOL),
	['`']  = (1 << TOK_SYMBOL),
	['a']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['b']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['c']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['d']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['e']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['f']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['g']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['h']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['i']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['j']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['k']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['l']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['m']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['n']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['o']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['p']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['q']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['r']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['s']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['t']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['u']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['v']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['w']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['x']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['y']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['z']  = (1 << TOK_ALPHA) | (1 << TOK_LOWER),
	['{']  = (1 << TOK_SYMBOL) | (1 << TOK_PARENT),
	['|']  = (1 << TOK_SYMBOL),
	['}']  = (1 << TOK_SYMBOL) | (1 << TOK_PARENT),
	['~']  = (1 << TOK_SYMBOL),
};

lex_t *lex_init(lex_t *lex, uint words_cap, uint toks_cap, alloc_t alloc)
{
	if (lex == NULL) {
		return NULL;
	}

	lex->chars     = s_chars;
	lex->chars_len = sizeof(s_chars) / sizeof(uint);

	if (words_cap > 0) {
		if (strbuf_init(&lex->words, words_cap, words_cap * 8, alloc) == NULL) {
			return NULL;
		}
	} else {
		mem_set(&lex->words, 0, sizeof(strbuf_t));
	}

	if (arr_init(&lex->toks, toks_cap, sizeof(tok_t), alloc) == NULL) {
		return NULL;
	}

	return lex;
}

void lex_free(lex_t *lex)
{
	if (lex == NULL) {
		return;
	}

	arr_free(&lex->toks);
	strbuf_free(&lex->words);
}

void lex_reset(lex_t *lex)
{
	if (lex == NULL) {
		return;
	}

	arr_reset(&lex->toks, 0);
	strbuf_reset(&lex->words, 0);
}

strv_t lex_get_tok_val(const lex_t *lex, tok_t tok)
{
	if (lex == NULL || lex->src.data == NULL) {
		return STRV_NULL;
	}

	return STRVN(&lex->src.data[tok.start], tok.len);
}

tok_loc_t lex_get_tok_loc(const lex_t *lex, uint index)
{
	if (lex == NULL || lex->src.data == NULL) {
		return (tok_loc_t){0};
	}

	tok_loc_t loc = {0};

	for (uint i = 0; i < index && i < (uint)lex->src.len; i++) {
		if (lex->src.data[i] == '\n') {
			loc.line_off = i + 1;
			loc.line_nr++;
			loc.col = 0;
		} else {
			loc.col++;
		}
	}

	for (size_t i = loc.line_off; i < lex->src.len && lex->src.data[i] != '\n'; i++) {
		loc.line_len++;
	}

	return loc;
}

void lex_set_src(lex_t *lex, strv_t src, strv_t file, uint line_off)
{
	if (lex == NULL) {
		return;
	}

	lex->src      = src;
	lex->file     = file;
	lex->line_off = line_off;
	lex->toks.cnt = 0;
}

int lex_tokenize(lex_t *lex, strv_t src, strv_t file, uint line_off)
{
	if (lex == NULL) {
		return 1;
	}

	lex_set_src(lex, src, file, line_off);

	strv_t word;

	for (size_t i = 0; i < lex->src.len;) {
		tok_t *tok = arr_add(&lex->toks, NULL);
		if (tok == NULL) {
			return 1;
		}

		uint j	  = 0;
		int found = 0;
		strbuf_foreach(&lex->words, j, word)
		{
			if (i + word.len > lex->src.len) {
				continue;
			}

			if (!strv_eq(STRVN(&lex->src.data[i], word.len), word)) {
				continue;
			}

			*tok = (tok_t){
				.type  = 1 << TOK_WORD,
				.start = i,
				.len   = (uint)word.len,
			};
			i += word.len;
			found = 1;
			break;
		}

		if (found) {
			continue;
		}

		uint c = (uint)lex->src.data[i];
		*tok   = (tok_t){
			  .type	 = c < lex->chars_len ? lex->chars[c] : TOK_UNKNOWN,
			  .start = i,
			  .len	 = 1,
		  };

		i++;
	}

	return 0;
}

size_t lex_print_tok(const lex_t *lex, tok_t tok, dst_t dst)
{
	if (lex == NULL) {
		return 0;
	}

	size_t off = dst.off;

	dst.off += tok_type_print(tok.type, dst);
	strv_t val   = lex_get_tok_val(lex, tok);
	char buf[32] = {0};
	size_t len   = strv_print(val, DST_BUF(buf));
	dst.off += dputs(dst, STRV("("));
	dst.off += dputs(dst, STRVN(buf, len));
	dst.off += dputs(dst, STRV(")"));

	return dst.off - off;
}

size_t lex_print(const lex_t *lex, dst_t dst)
{
	if (lex == NULL) {
		return 0;
	}

	size_t off = dst.off;

	tok_t *tok;
	uint i = 0;
	arr_foreach(&lex->toks, i, tok)
	{
		dst.off += lex_print_tok(lex, *tok, dst);
		dst.off += dputs(dst, STRV("\n"));
	}

	return dst.off - off;
}

size_t lex_tok_loc_print_loc(const lex_t *lex, tok_loc_t loc, dst_t dst)
{
	if (lex == NULL || lex->file.data == NULL) {
		return 0;
	}

	size_t off = dst.off;

	dst.off += dputf(dst, "%.*s:%d:%d: ", lex->file.len, lex->file.data, lex->line_off + loc.line_nr, loc.col);

	return dst.off - off;
}

size_t lex_tok_loc_print_src(const lex_t *lex, tok_loc_t loc, dst_t dst)
{
	if (lex == NULL || lex->src.data == NULL) {
		return 0;
	}

	size_t off = dst.off;

	dst.off += dputs(dst, STRVN(&lex->src.data[loc.line_off], loc.line_len));
	dst.off += dputf(dst, "\n%*s^\n", loc.col, "");

	return dst.off - off;
}
