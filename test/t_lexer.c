#include "lexer.h"

#include "cstr.h"
#include "log.h"
#include "mem.h"
#include "test.h"

TEST(lex_init_free)
{
	START;

	lex_t lex = {0};

	EXPECT_EQ(lex_init(NULL, str_null(), str_null(), 0), NULL);
	EXPECT_EQ(lex_init(&lex, str_null(), str_null(), 0), &lex);
	EXPECT_EQ(lex_init(&lex, STR(__FILE__), STR("src"), __LINE__), &lex);

	EXPECT_STR(lex.file.data, __FILE__);
	EXPECT_STR(lex.src.data, "src");

	lex_free(NULL);
	lex_free(&lex);

	END;
}

TEST(lex_get_token)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), STR("Aa0'\", \t\r\n\0{"), __LINE__);

	token_t token;
	token = lex_get_token(NULL, 0);
	EXPECT_EQ(token.type, TOKEN_UNKNOWN);
	token = lex_get_token(&lex, 0);
	EXPECT_EQ(token.start, 0);
	EXPECT_EQ(token.len, 1);

	token = lex_get_token(&lex, 0);
	EXPECT_EQ(token.type, (1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER));
	token = lex_get_token(&lex, 1);
	EXPECT_EQ(token.type, (1 << TOKEN_ALPHA) | (1 << TOKEN_LOWER));
	token = lex_get_token(&lex, 2);
	EXPECT_EQ(token.type, (1 << TOKEN_DIGIT));
	token = lex_get_token(&lex, 3);
	EXPECT_EQ(token.type, (1 << TOKEN_SQUOTE));
	token = lex_get_token(&lex, 4);
	EXPECT_EQ(token.type, (1 << TOKEN_DQUOTE));
	token = lex_get_token(&lex, 5);
	EXPECT_EQ(token.type, (1 << TOKEN_COMMA));
	token = lex_get_token(&lex, 6);
	EXPECT_EQ(token.type, (1 << TOKEN_WS) | (1 << TOKEN_SPACE));
	token = lex_get_token(&lex, 7);
	EXPECT_EQ(token.type, (1 << TOKEN_WS) | (1 << TOKEN_TAB));
	token = lex_get_token(&lex, 8);
	EXPECT_EQ(token.type, (1 << TOKEN_WS) | (1 << TOKEN_CR));
	token = lex_get_token(&lex, 9);
	EXPECT_EQ(token.type, (1 << TOKEN_WS) | (1 << TOKEN_NL));
	token = lex_get_token(&lex, 10);
	EXPECT_EQ(token.type, (1 << TOKEN_NULL));
	token = lex_get_token(&lex, 11);
	EXPECT_EQ(token.type, (1 << TOKEN_SYMBOL));

	lex_free(&lex);

	END;
}

TEST(lex_get_token_val)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), STR("src"), __LINE__);

	token_t token = lex_get_token(&lex, 0);

	lex_get_token_val(NULL, token);
	str_t val = lex_get_token_val(&lex, token);

	EXPECT_STRN(val.data, "s", val.len);

	lex_free(&lex);

	END;
}

TEST(lex_get_token_loc)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), STR("src"), __LINE__);

	lex_get_token_loc(NULL, 0);
	token_loc_t loc = lex_get_token_loc(&lex, 0);

	EXPECT_EQ(loc.line_off, 0);
	EXPECT_EQ(loc.line_len, 3);
	EXPECT_EQ(loc.line_nr, 0);
	EXPECT_EQ(loc.col, 0);

	lex_free(&lex);

	END;
}

TEST(lex_get_token_loc_nl)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), STR("line1\n"), __LINE__);

	token_loc_t loc = lex_get_token_loc(&lex, 1);

	EXPECT_EQ(loc.line_off, 0);
	EXPECT_EQ(loc.line_len, 5);
	EXPECT_EQ(loc.line_nr, 0);
	EXPECT_EQ(loc.col, 1);

	lex_free(&lex);

	END;
}

TEST(lex_get_token_loc_el)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), STR("line1\n"), __LINE__);

	token_loc_t loc = lex_get_token_loc(&lex, 5);

	EXPECT_EQ(loc.line_off, 0);
	EXPECT_EQ(loc.line_len, 5);
	EXPECT_EQ(loc.line_nr, 0);
	EXPECT_EQ(loc.col, 5);

	lex_free(&lex);

	END;
}

TEST(lex_get_token_loc_sl)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), STR("line1\nline2"), __LINE__);

	token_loc_t loc = lex_get_token_loc(&lex, 6);

	EXPECT_EQ(loc.line_off, 6);
	EXPECT_EQ(loc.line_len, 5);
	EXPECT_EQ(loc.line_nr, 1);
	EXPECT_EQ(loc.col, 0);

	lex_free(&lex);

	END;
}

TEST(lex_token_loc_print_loc)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), STR("line1\nline2"), __LINE__);

	token_loc_t loc = lex_get_token_loc(&lex, 7);

	char buf[2048] = {0};
	EXPECT_EQ(lex_token_loc_print_loc(NULL, loc, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	lex_token_loc_print_loc(&lex, loc, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, __FILE__ ":173:1: ");

	lex_free(&lex);

	END;
}

TEST(lex_token_loc_print_src)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), STR("line1\nline2"), __LINE__);

	token_loc_t loc = lex_get_token_loc(&lex, 7);

	char buf[2048] = {0};
	EXPECT_EQ(lex_token_loc_print_src(NULL, loc, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	lex_token_loc_print_src(&lex, loc, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf,
		   "line2\n"
		   " ^\n");

	lex_free(&lex);

	END;
}

STEST(lexer)
{
	SSTART;

	RUN(lex_init_free);
	RUN(lex_get_token);
	RUN(lex_get_token_val);
	RUN(lex_get_token_loc);
	RUN(lex_get_token_loc_nl);
	RUN(lex_get_token_loc_el);
	RUN(lex_get_token_loc_sl);
	RUN(lex_token_loc_print_loc);
	RUN(lex_token_loc_print_src);

	SEND;
}
