#include "file/cfg_prs.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(cfg_prs_init_free)
{
	START;

	cfg_prs_t prs = {0};

	EXPECT_EQ(cfg_prs_init(NULL, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(cfg_prs_init(&prs, ALLOC_STD), NULL);
	mem_oom(0);

	cfg_prs_free(NULL);

	END;
}

TESTP(cfg_prs_null, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str  = STRV("");
	strv_t str2 = STRV("int0 = 1\n");

	uint cap = prs->lex.toks.cap;

	EXPECT_EQ(cfg_prs_parse(NULL, STRV_NULL, NULL, NULL, DST_NONE()), 1);
	EXPECT_EQ(cfg_prs_parse(prs, STRV_NULL, NULL, NULL, DST_NONE()), 1);
	EXPECT_EQ(cfg_prs_parse(prs, str, NULL, NULL, DST_NONE()), 1);
	mem_oom(1);
	prs->lex.toks.cap = 0;
	EXPECT_EQ(cfg_prs_parse(prs, str2, &cfg, NULL, DST_NONE()), 1);
	prs->lex.toks.cap = cap;
	mem_oom(0);
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, NULL, DST_NONE()), 0);

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_fail, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("int0 = 1\n");

	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, NULL, DST_NONE()), 1);

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_root, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("int = 1\n"
			  "str = \"str\"\n"
			  "lit = lit_LIT\n"
			  "arr = [\"str\", 1]\n"
			  "obj = {str = \"str\", int = 1}\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, &root, DST_NONE()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf,
		   "int = 1\n"
		   "str = \"str\"\n"
		   "lit = lit_LIT\n"
		   "arr = [\"str\", 1]\n"
		   "obj = {str = \"str\", int = 1}\n");

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_lit, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("abcABC123:_\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf, "abcABC123:_\n");

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_str, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("\"abcABC123 +*'\"\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf, "\"abcABC123 +*'\"\n");

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_mode, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("a += 1\n"
			  "a -= 1\n"
			  "a ?= 1\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf,
		   "a += 1\n"
		   "a -= 1\n"
		   "a ?= 1\n");

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_arr_one, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("arr:\n"
			  "val1\n"
			  "\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf,
		   "arr:\n"
		   "val1\n"
		   "\n");

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_arr_two, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("arr:\n"
			  "val1\n"
			  "val2\n"
			  "\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf,
		   "arr:\n"
		   "val1\n"
		   "val2\n"
		   "\n");

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_tbl, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("[+tbl:name=tbl1]\n"
			  "int = 1\n"
			  "str = \"str\"\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf,
		   "[+tbl:name=tbl1]\n"
		   "int = 1\n"
		   "str = \"str\"\n");

	cfg_free(&cfg);

	END;
}

TESTP(cfg_prs_test, cfg_prs_t *prs)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("\"str\"\n"
			  "int = 1\n"
			  "str = \"str\"\n"
			  "lit = lit_LIT\n"
			  "arr = [\"str\", 1]\n"
			  "obj = {str = \"str\", int = 1}\n"
			  "\n"
			  "[tbl]\n"
			  "int = 1\n"
			  "arr:\n"
			  "\"val1\"\n"
			  "\"val2\"\n"
			  "\n"
			  "str = \"str\"\n"
			  "\n"
			  "[tbll]\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(prs, str, &cfg, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf,
		   "\"str\"\n"
		   "int = 1\n"
		   "str = \"str\"\n"
		   "lit = lit_LIT\n"
		   "arr = [\"str\", 1]\n"
		   "obj = {str = \"str\", int = 1}\n"
		   "\n"
		   "[tbl]\n"
		   "int = 1\n"
		   "arr:\n"
		   "\"val1\"\n"
		   "\"val2\"\n"
		   "\n"
		   "str = \"str\"\n"
		   "\n"
		   "[tbll]\n");

	cfg_free(&cfg);

	END;
}

STEST(cfg_prs)
{
	SSTART;

	RUN(cfg_prs_init_free);
	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);
	RUNP(cfg_prs_null, &prs);
	RUNP(cfg_prs_fail, &prs);
	RUNP(cfg_prs_root, &prs);
	RUNP(cfg_prs_lit, &prs);
	RUNP(cfg_prs_str, &prs);
	RUNP(cfg_prs_mode, &prs);
	RUNP(cfg_prs_arr_one, &prs);
	RUNP(cfg_prs_arr_two, &prs);
	RUNP(cfg_prs_tbl, &prs);
	RUNP(cfg_prs_test, &prs);
	cfg_prs_free(&prs);
	SEND;
}
