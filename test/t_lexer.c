#include "lexer.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(lex_init_free)
{
	START;

	lex_t lex = {0};

	EXPECT_EQ(lex_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(lex_init(&lex, 1, 0, ALLOC_STD), NULL);
	EXPECT_EQ(lex_init(&lex, 0, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(lex_init(&lex, 0, 0, ALLOC_STD), &lex);
	log_set_quiet(0, 0);

	lex_free(NULL);
	lex_free(&lex);

	END;
}

TEST(lex_add_word)
{
	START;

	lex_t lex = {0};
	log_set_quiet(0, 1);
	lex_init(&lex, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(lex_add_word(&lex, STRV("Aa"), NULL), 0);

	lex_free(&lex);

	END;
}

TEST(lex_add_token)
{
	START;

	lex_t lex = {0};
	log_set_quiet(0, 1);
	str_t src = strz(0);
	lex_init(&lex, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	lex_set_src(&lex, &src, STR(__FILE__), __LINE__);

	EXPECT_EQ(lex_add_token(NULL, 0, STRV_NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(lex_add_token(&lex, 0, STRV("A"), NULL), 1);
	mem_oom(0);
	str_free(&src);
	src	= strz(2);
	lex.src = &src;
	mem_oom(1);
	EXPECT_EQ(lex_add_token(&lex, 0, STRV("A"), NULL), 1);
	mem_oom(0);
	EXPECT_EQ(lex_add_token(&lex, 0, STRV("A"), NULL), 0);
	uint index;
	EXPECT_EQ(lex_add_token(&lex, 0, STRV("A"), &index), 0);
	EXPECT_EQ(index, 1);

	EXPECT_STRN(src.data, "AA", src.len);

	str_free(&src);
	lex_free(&lex);

	END;
}

TEST(lex_get_token)
{
	START;

	lex_t lex = {0};
	str_t src = STR("Aa");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

	token_t token = lex_get_token(&lex, 0);
	EXPECT_EQ(token.start, 0);
	EXPECT_EQ(token.len, 1);

	lex_free(&lex);

	END;
}

TEST(lex_get_token_val)
{
	START;

	lex_t lex = {0};
	str_t src = STR("src");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

	token_t token = lex_get_token(&lex, 0);

	EXPECT_EQ(lex_get_token_val(NULL, token).len, 0);
	strv_t val = lex_get_token_val(&lex, token);

	EXPECT_STRN(val.data, "s", val.len);

	lex_free(&lex);

	END;
}

TEST(lex_get_token_loc)
{
	START;

	lex_t lex = {0};
	str_t src = STR("src");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

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
	str_t src = STR("line1\n");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

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
	str_t src = STR("line1\n");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

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
	str_t src = STR("line1\nline2");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

	token_loc_t loc = lex_get_token_loc(&lex, 6);

	EXPECT_EQ(loc.line_off, 6);
	EXPECT_EQ(loc.line_len, 5);
	EXPECT_EQ(loc.line_nr, 1);
	EXPECT_EQ(loc.col, 0);

	lex_free(&lex);

	END;
}

TEST(lex_set_src)
{
	START;

	lex_t lex = {0};
	lex_set_src(NULL, NULL, STR_NULL, 0);
	lex_set_src(&lex, NULL, STR_NULL, 0);

	END;
}

TEST(lex_tokenize)
{
	START;

	lex_t lex = {0};
	str_t src = STR("Aabc");
	lex_init(&lex, 1, 1, ALLOC_STD);

	lex_add_word(&lex, STRV("abc"), NULL);

	EXPECT_EQ(lex_tokenize(NULL, NULL, STR_NULL, 0), 1);
	mem_oom(1);
	EXPECT_EQ(lex_tokenize(&lex, &src, STR(__FILE__), __LINE__), 1);
	mem_oom(0);
	EXPECT_EQ(lex_tokenize(&lex, &src, STR(__FILE__), __LINE__), 0);

	EXPECT_EQ(lex.tokens.cnt, 2);

	{
		token_t token = lex_get_token(&lex, 0);
		EXPECT_EQ(token.type, (1 << TOKEN_UPPER) | (1 << TOKEN_ALPHA));
		EXPECT_EQ(token.start, 0);
		EXPECT_EQ(token.len, 1);
	}

	{
		token_t token = lex_get_token(&lex, 1);
		EXPECT_EQ(token.type, (1 << TOKEN_WORD));
		EXPECT_EQ(token.start, 1);
		EXPECT_EQ(token.len, 3);
	}

	lex_free(&lex);

	END;
}

TEST(lex_print_token)
{
	START;

	lex_t lex = {0};
	str_t src = STR("Aa");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

	char buf[256] = {0};

	EXPECT_EQ(lex_print_token(NULL, (token_t){0}, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(lex_print_token(&lex, lex_get_token(&lex, 0), PRINT_DST_BUF(buf, sizeof(buf), 0)), 14);

	EXPECT_STR(buf, "ALPHA|UPPER(A)");

	lex_free(&lex);

	END;
}

TEST(lex_print)
{
	START;

	lex_t lex = {0};
	str_t src = STR("Aa");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

	char buf[256] = {0};

	EXPECT_EQ(lex_print(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(lex_print(&lex, PRINT_DST_BUF(buf, sizeof(buf), 0)), 30);

	EXPECT_STR(buf,
		   "ALPHA|UPPER(A)\n"
		   "ALPHA|LOWER(a)\n");

	lex_free(&lex);

	END;
}

TEST(lex_token_loc_print_loc)
{
	START;

	lex_t lex = {0};
	str_t src = STR("line1\n"
			"line2");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 3);

	token_loc_t loc = lex_get_token_loc(&lex, 7);

	char buf[2048] = {0};
	EXPECT_EQ(lex_token_loc_print_loc(NULL, loc, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	lex_token_loc_print_loc(&lex, loc, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, __FILE__ ":300:1: ");

	lex_free(&lex);

	END;
}

TEST(lex_token_loc_print_src)
{
	START;

	lex_t lex = {0};
	str_t src = STR("line1\nline2");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__);

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
	RUN(lex_add_word);
	RUN(lex_add_token);
	RUN(lex_get_token);
	RUN(lex_get_token_val);
	RUN(lex_get_token_loc);
	RUN(lex_get_token_loc_nl);
	RUN(lex_get_token_loc_el);
	RUN(lex_get_token_loc_sl);
	RUN(lex_set_src);
	RUN(lex_tokenize);
	RUN(lex_print_token);
	RUN(lex_print);
	RUN(lex_token_loc_print_loc);
	RUN(lex_token_loc_print_src);

	SEND;
}
