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
	EXPECT_EQ(cfg_prs_init(&prs, ALLOC_STD), &prs);

	cfg_prs_free(&prs);
	cfg_prs_free(NULL);

	END;
}

TEST(cfg_prs_null)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("");

	EXPECT_EQ(cfg_prs_parse(NULL, STRV_NULL, NULL, ALLOC_STD, NULL, DST_NONE()), 1);
	EXPECT_EQ(cfg_prs_parse(&prs, STRV_NULL, NULL, ALLOC_STD, NULL, DST_NONE()), 1);
	EXPECT_EQ(cfg_prs_parse(&prs, str, NULL, ALLOC_STD, NULL, DST_NONE()), 1);
	mem_oom(1);
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, NULL, DST_NONE()), 1);
	mem_oom(0);
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, NULL, DST_NONE()), 0);

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_prs_fail)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("int0 = 1\n");

	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, NULL, DST_NONE()), 1);

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_prs_root)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("int = 1\n"
			  "str = \"str\"\n"
			  "lit = lit_LIT\n"
			  "arr = [\"str\", 1]\n"
			  "obj = {str = \"str\", int = 1}\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, &root, DST_NONE()), 0);
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
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_prs_lit)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("abcABC123:_\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf, "abcABC123:_\n");

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_prs_str)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("\"abcABC123 +*'\"\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf, "\"abcABC123 +*'\"\n");

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_prs_tbl)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV(":tbl\n"
			  "int = 1\n"
			  "str = \"str\"\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, &root, DST_STD()), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));
	EXPECT_STR(buf,
		   ":tbl\n"
		   "int = 1\n"
		   "str = \"str\"\n");

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_prs_test)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("\"str\"\n"
			  "int = 1\n"
			  "str = \"str\"\n"
			  "lit = lit_LIT\n"
			  "arr = [\"str\", 1]\n"
			  "obj = {str = \"str\", int = 1}\n"
			  ":tbl\n"
			  "int = 1\n"
			  "str = \"str\"\n"
			  "\n"
			  ":tbll\n");

	cfg_var_t root;
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, &root, DST_STD()), 0);
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
		   ":tbl\n"
		   "int = 1\n"
		   "str = \"str\"\n"
		   "\n"
		   ":tbll\n");

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

STEST(cfg_prs)
{
	SSTART;

	RUN(cfg_prs_init_free);
	RUN(cfg_prs_null);
	RUN(cfg_prs_fail);
	RUN(cfg_prs_root);
	RUN(cfg_prs_lit);
	RUN(cfg_prs_str);
	RUN(cfg_prs_tbl);
	RUN(cfg_prs_test);

	SEND;
}
