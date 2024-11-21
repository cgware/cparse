#include "lexer.h"

#include "log.h"

lex_t *lex_init(lex_t *lex, str_t file, str_t src, uint line_off)
{
	if (lex == NULL) {
		return NULL;
	}

	lex->file     = file;
	lex->src      = src;
	lex->line_off = line_off;

	return lex;
}

void lex_free(lex_t *lex)
{
	if (lex == NULL) {
		return;
	}
}

token_t lex_get_token(const lex_t *lex, uint index)
{
	if (lex == NULL) {
		return (token_t){.type = TOKEN_UNKNOWN};
	}

	if (index >= lex->src.len) {
		return (token_t){.type = 1 << TOKEN_EOF};
	}

	char c = lex->src.data[index];

	token_type_t type = TOKEN_UNKNOWN;

	if (c >= 'A' && c <= 'Z') {
		type |= 1 << TOKEN_ALPHA;
		type |= 1 << TOKEN_UPPER;
	} else if (c >= 'a' && c <= 'z') {
		type |= 1 << TOKEN_ALPHA;
		type |= 1 << TOKEN_LOWER;
	} else if (c >= '0' && c <= '9') {
		type |= 1 << TOKEN_DIGIT;
	} else if (c == '\'') {
		type |= 1 << TOKEN_SQUOTE;
	} else if (c == '"') {
		type |= 1 << TOKEN_DQUOTE;
	} else if (c == ',') {
		type |= 1 << TOKEN_COMMA;
	} else if (c == ' ') {
		type |= 1 << TOKEN_WS;
		type |= 1 << TOKEN_SPACE;
	} else if (c == '\t') {
		type |= 1 << TOKEN_WS;
		type |= 1 << TOKEN_TAB;
	} else if (c == '\r') {
		type |= 1 << TOKEN_WS;
		type |= 1 << TOKEN_CR;
	} else if (c == '\n') {
		type |= 1 << TOKEN_WS;
		type |= 1 << TOKEN_NL;
	} else if (c == '\0') {
		type |= 1 << TOKEN_NULL;
	} else {
		type |= 1 << TOKEN_SYMBOL;
	}

	return (token_t){
		.start = index,
		.len   = 1,
		.type  = type,
	};
}

str_t lex_get_token_val(const lex_t *lex, token_t token)
{
	if (lex == NULL) {
		return STR_NULL;
	}

	return strc(&lex->src.data[token.start], token.len);
}

token_loc_t lex_get_token_loc(const lex_t *lex, uint index)
{
	if (lex == NULL) {
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

int lex_token_loc_print_loc(const lex_t *lex, token_loc_t loc, print_dst_t dst)
{
	if (lex == NULL) {
		return 0;
	}

	int off = dst.off;

	dst.off += c_dprintf(dst, "%.*s:%d:%d: ", lex->file.len, lex->file.data, lex->line_off + loc.line_nr, loc.col);

	return dst.off - off;
}

int lex_token_loc_print_src(const lex_t *lex, token_loc_t loc, print_dst_t dst)
{
	if (lex == NULL) {
		return 0;
	}

	int off = dst.off;

	dst.off += c_dprintf(dst, "%.*s\n", loc.line_len, &lex->src.data[loc.line_off]);
	dst.off += c_dprintf(dst, "%*s^\n", loc.col, "");

	return dst.off - off;
}
