#include "lex.h"

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

TEST(lex_get_tok)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("Aa");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	tok_t tok = lex_get_tok(&lex, 0);
	EXPECT_EQ(tok.start, 0);
	EXPECT_EQ(tok.len, 1);

	lex_free(&lex);

	END;
}

TEST(lex_get_tok_val)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("src");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	tok_t tok = lex_get_tok(&lex, 0);

	EXPECT_EQ(lex_get_tok_val(NULL, tok).len, 0);
	strv_t val = lex_get_tok_val(&lex, tok);

	EXPECT_STRN(val.data, "s", val.len);

	lex_free(&lex);

	END;
}

TEST(lex_get_tok_loc)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("src");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	lex_get_tok_loc(NULL, 0);
	tok_loc_t loc = lex_get_tok_loc(&lex, 0);

	EXPECT_EQ(loc.line_off, 0);
	EXPECT_EQ(loc.line_len, 3);
	EXPECT_EQ(loc.line_nr, 0);
	EXPECT_EQ(loc.col, 0);

	lex_free(&lex);

	END;
}

TEST(lex_get_tok_loc_nl)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("line1\n");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	tok_loc_t loc = lex_get_tok_loc(&lex, 1);

	EXPECT_EQ(loc.line_off, 0);
	EXPECT_EQ(loc.line_len, 5);
	EXPECT_EQ(loc.line_nr, 0);
	EXPECT_EQ(loc.col, 1);

	lex_free(&lex);

	END;
}

TEST(lex_get_tok_loc_el)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("line1\n");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	tok_loc_t loc = lex_get_tok_loc(&lex, 5);

	EXPECT_EQ(loc.line_off, 0);
	EXPECT_EQ(loc.line_len, 5);
	EXPECT_EQ(loc.line_nr, 0);
	EXPECT_EQ(loc.col, 5);

	lex_free(&lex);

	END;
}

TEST(lex_get_tok_loc_sl)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("line1\nline2");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	tok_loc_t loc = lex_get_tok_loc(&lex, 6);

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
	lex_set_src(NULL, STRV_NULL, STRV_NULL, 0);
	lex_set_src(&lex, STRV_NULL, STRV_NULL, 0);

	END;
}

TEST(lex_tokenize)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("Aabc");
	lex_init(&lex, 1, 1, ALLOC_STD);

	lex_add_word(&lex, STRV("abc"), NULL);

	EXPECT_EQ(lex_tokenize(NULL, STRV_NULL, STRV_NULL, 0), 1);
	mem_oom(1);
	EXPECT_EQ(lex_tokenize(&lex, src, STRV(__FILE__), __LINE__), 1);
	mem_oom(0);
	EXPECT_EQ(lex_tokenize(&lex, src, STRV(__FILE__), __LINE__), 0);

	EXPECT_EQ(lex.toks.cnt, 2);

	{
		tok_t tok = lex_get_tok(&lex, 0);
		EXPECT_EQ(tok.type, (1 << TOK_UPPER) | (1 << TOK_ALPHA));
		EXPECT_EQ(tok.start, 0);
		EXPECT_EQ(tok.len, 1);
	}

	{
		tok_t tok = lex_get_tok(&lex, 1);
		EXPECT_EQ(tok.type, (1 << TOK_WORD));
		EXPECT_EQ(tok.start, 1);
		EXPECT_EQ(tok.len, 3);
	}

	lex_free(&lex);

	END;
}

TEST(lex_print_tok)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("Aa");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	char buf[256] = {0};

	EXPECT_EQ(lex_print_tok(NULL, (tok_t){0}, DST_BUF(buf)), 0);
	EXPECT_EQ(lex_print_tok(&lex, lex_get_tok(&lex, 0), DST_BUF(buf)), 14);

	EXPECT_STR(buf, "ALPHA|UPPER(A)");

	lex_free(&lex);

	END;
}

TEST(lex_print)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("Aa");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	char buf[256] = {0};

	EXPECT_EQ(lex_print(NULL, DST_BUF(buf)), 0);
	EXPECT_EQ(lex_print(&lex, DST_BUF(buf)), 30);

	EXPECT_STR(buf,
		   "ALPHA|UPPER(A)\n"
		   "ALPHA|LOWER(a)\n");

	lex_free(&lex);

	END;
}

TEST(lex_tok_loc_print_loc)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("line1\n"
			  "line2");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 3);

	tok_loc_t loc = lex_get_tok_loc(&lex, 7);

	char buf[2048] = {0};
	EXPECT_EQ(lex_tok_loc_print_loc(NULL, loc, DST_BUF(buf)), 0);
	lex_tok_loc_print_loc(&lex, loc, DST_BUF(buf));

	EXPECT_STR(buf, __FILE__ ":266:1: ");

	lex_free(&lex);

	END;
}

TEST(lex_tok_loc_print_src)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("line1\nline2");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__);

	tok_loc_t loc = lex_get_tok_loc(&lex, 7);

	char buf[2048] = {0};
	EXPECT_EQ(lex_tok_loc_print_src(NULL, loc, DST_BUF(buf)), 0);
	lex_tok_loc_print_src(&lex, loc, DST_BUF(buf));

	EXPECT_STR(buf,
		   "line2\n"
		   " ^\n");

	lex_free(&lex);

	END;
}

STEST(lex)
{
	SSTART;

	RUN(lex_init_free);
	RUN(lex_add_word);
	RUN(lex_get_tok);
	RUN(lex_get_tok_val);
	RUN(lex_get_tok_loc);
	RUN(lex_get_tok_loc_nl);
	RUN(lex_get_tok_loc_el);
	RUN(lex_get_tok_loc_sl);
	RUN(lex_set_src);
	RUN(lex_tokenize);
	RUN(lex_print_tok);
	RUN(lex_print);
	RUN(lex_tok_loc_print_loc);
	RUN(lex_tok_loc_print_src);

	SEND;
}
