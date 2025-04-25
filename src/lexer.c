#include "lexer.h"

#include "token.h"

#include <memory.h>

typedef struct word_s {
	uint start;
	uint len;
} word_t;

static const uint s_chars[128] = {
	['\0'] = (1 << TOKEN_NULL),
	['\t'] = (1 << TOKEN_WS) | (1 << TOKEN_TAB),
	['\n'] = (1 << TOKEN_WS) | (1 << TOKEN_NL),
	['\r'] = (1 << TOKEN_WS) | (1 << TOKEN_CR),
	[' ']  = (1 << TOKEN_WS),
	['!']  = (1 << TOKEN_SYMBOL),
	['"']  = (1 << TOKEN_QUOTE),
	['#']  = (1 << TOKEN_SYMBOL),
	['$']  = (1 << TOKEN_SYMBOL),
	['%']  = (1 << TOKEN_SYMBOL),
	['&']  = (1 << TOKEN_SYMBOL),
	['\''] = (1 << TOKEN_QUOTE),
	['(']  = (1 << TOKEN_SYMBOL) | (1 << TOKEN_PARENT),
	[')']  = (1 << TOKEN_SYMBOL) | (1 << TOKEN_PARENT),
	['*']  = (1 << TOKEN_SYMBOL),
	['+']  = (1 << TOKEN_SYMBOL),
	[',']  = (1 << TOKEN_SYMBOL),
	['-']  = (1 << TOKEN_SYMBOL),
	['.']  = (1 << TOKEN_SYMBOL),
	['/']  = (1 << TOKEN_SYMBOL),
	['0']  = (1 << TOKEN_DIGIT),
	['1']  = (1 << TOKEN_DIGIT),
	['2']  = (1 << TOKEN_DIGIT),
	['3']  = (1 << TOKEN_DIGIT),
	['4']  = (1 << TOKEN_DIGIT),
	['5']  = (1 << TOKEN_DIGIT),
	['6']  = (1 << TOKEN_DIGIT),
	['7']  = (1 << TOKEN_DIGIT),
	['8']  = (1 << TOKEN_DIGIT),
	['9']  = (1 << TOKEN_DIGIT),
	[':']  = (1 << TOKEN_SYMBOL),
	[';']  = (1 << TOKEN_SYMBOL),
	['<']  = (1 << TOKEN_SYMBOL),
	['=']  = (1 << TOKEN_SYMBOL),
	['>']  = (1 << TOKEN_SYMBOL),
	['?']  = (1 << TOKEN_SYMBOL),
	['@']  = (1 << TOKEN_SYMBOL),
	['A']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['B']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['C']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['D']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['E']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['F']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['G']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['H']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['I']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['J']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['K']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['L']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['M']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['N']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['O']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['P']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['Q']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['R']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['S']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['T']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['U']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['V']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['W']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['X']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['Y']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['Z']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER),
	['[']  = (1 << TOKEN_SYMBOL) | (1 << TOKEN_PARENT),
	['\\'] = (1 << TOKEN_SYMBOL),
	[']']  = (1 << TOKEN_SYMBOL) | (1 << TOKEN_PARENT),
	['^']  = (1 << TOKEN_SYMBOL),
	['_']  = (1 << TOKEN_SYMBOL),
	['`']  = (1 << TOKEN_SYMBOL),
	['a']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['b']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['c']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['d']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['e']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['f']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['g']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['h']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['i']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['j']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['k']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['l']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['m']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['n']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['o']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['p']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['q']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['r']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['s']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['t']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['u']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['v']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['w']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['x']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['y']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['z']  = (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER),
	['{']  = (1 << TOKEN_SYMBOL) | (1 << TOKEN_PARENT),
	['|']  = (1 << TOKEN_SYMBOL),
	['}']  = (1 << TOKEN_SYMBOL) | (1 << TOKEN_PARENT),
	['~']  = (1 << TOKEN_SYMBOL),
};

lex_t *lex_init(lex_t *lex, uint words_cap, uint tokens_cap, alloc_t alloc)
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
		memset(&lex->words, 0, sizeof(strbuf_t));
	}

	if (arr_init(&lex->tokens, tokens_cap, sizeof(token_t), alloc) == NULL) {
		return NULL;
	}

	return lex;
}

void lex_free(lex_t *lex)
{
	if (lex == NULL) {
		return;
	}

	arr_free(&lex->tokens);
	strbuf_free(&lex->words);
}

strv_t lex_get_token_val(const lex_t *lex, token_t token)
{
	if (lex == NULL || lex->src.data == NULL) {
		return STRV_NULL;
	}

	return STRVN(&lex->src.data[token.start], token.len);
}

token_loc_t lex_get_token_loc(const lex_t *lex, uint index)
{
	if (lex == NULL || lex->src.data == NULL) {
		return (token_loc_t){0};
	}

	token_loc_t loc = {0};

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

	lex->src	= src;
	lex->file	= file;
	lex->line_off	= line_off;
	lex->tokens.cnt = 0;
}

int lex_tokenize(lex_t *lex, strv_t src, strv_t file, uint line_off)
{
	if (lex == NULL) {
		return 1;
	}

	lex_set_src(lex, src, file, line_off);

	strv_t word;

	for (size_t i = 0; i < lex->src.len;) {
		token_t *token = arr_add(&lex->tokens);
		if (token == NULL) {
			return 1;
		}

		uint j  = 0;
		int found = 0;
		strbuf_foreach(&lex->words, j, word)
		{
			if (i + word.len > lex->src.len) {
				continue;
			}

			if (memcmp(&lex->src.data[i], word.data, word.len) != 0) {
				continue;
			}

			*token = (token_t){
				.type  = 1 << TOKEN_WORD,
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
		*token = (token_t){
			.type  = c < lex->chars_len ? lex->chars[c] : TOKEN_UNKNOWN,
			.start = i,
			.len   = 1,
		};

		i++;
	}

	return 0;
}

int lex_print_token(const lex_t *lex, token_t token, print_dst_t dst)
{
	if (lex == NULL) {
		return 0;
	}

	int off = dst.off;

	dst.off += token_type_print(token.type, dst);
	strv_t val   = lex_get_token_val(lex, token);
	char buf[32] = {0};
	int len	     = strv_print(val, PRINT_DST_BUF(buf, sizeof(buf), 0));
	dst.off += c_dprintf(dst, "(%.*s)", len, buf);

	return dst.off - off;
}

int lex_print(const lex_t *lex, print_dst_t dst)
{
	if (lex == NULL) {
		return 0;
	}

	int off = dst.off;

	token_t *token;
	arr_foreach(&lex->tokens, token)
	{
		dst.off += lex_print_token(lex, *token, dst);
		dst.off += c_dprintf(dst, "\n");
	}

	return dst.off - off;
}

int lex_token_loc_print_loc(const lex_t *lex, token_loc_t loc, print_dst_t dst)
{
	if (lex == NULL || lex->file.data == NULL) {
		return 0;
	}

	int off = dst.off;

	dst.off += c_dprintf(dst, "%.*s:%d:%d: ", lex->file.len, lex->file.data, lex->line_off + loc.line_nr, loc.col);

	return dst.off - off;
}

int lex_token_loc_print_src(const lex_t *lex, token_loc_t loc, print_dst_t dst)
{
	if (lex == NULL || lex->src.data == NULL) {
		return 0;
	}

	int off = dst.off;

	dst.off += c_dprintf(dst, "%.*s\n", loc.line_len, &lex->src.data[loc.line_off]);
	dst.off += c_dprintf(dst, "%*s^\n", loc.col, "");

	return dst.off - off;
}
