#include "file/cfg_parse.h"

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

TEST(cfg_parse_null)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("");

	EXPECT_EQ(cfg_prs_parse(NULL, STRV_NULL, NULL, ALLOC_STD, PRINT_DST_NONE()), CFG_VAR_END);
	EXPECT_EQ(cfg_prs_parse(&prs, STRV_NULL, NULL, ALLOC_STD, PRINT_DST_NONE()), CFG_VAR_END);
	EXPECT_EQ(cfg_prs_parse(&prs, str, NULL, ALLOC_STD, PRINT_DST_NONE()), CFG_VAR_END);
	mem_oom(1);
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, PRINT_DST_NONE()), CFG_VAR_END);
	mem_oom(0);
	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, PRINT_DST_NONE()), 0);

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_parse_fail)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("int0 = 1\n");

	EXPECT_EQ(cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, PRINT_DST_NONE()), CFG_VAR_END);

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_parse_root)
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

	cfg_var_t root = cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, PRINT_DST_NONE());
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, PRINT_DST_BUF(buf, sizeof(buf), 0));
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

TEST(cfg_parse_tbl)
{
	START;

	cfg_prs_t prs = {0};
	cfg_prs_init(&prs, ALLOC_STD);

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	strv_t str = STRV("[tbl]\n"
			  "int = 1\n"
			  "str = \"str\"\n");

	cfg_var_t root = cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, PRINT_DST_STD());
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf,
		   "[tbl]\n"
		   "int = 1\n"
		   "str = \"str\"\n");

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

TEST(cfg_parse_test)
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
			  "obj = {str = \"str\", int = 1}\n"
			  "[tbl]\n"
			  "int = 1\n"
			  "str = \"str\"\n"
			  "\n"
			  "[tbll]\n");

	cfg_var_t root = cfg_prs_parse(&prs, str, &cfg, ALLOC_STD, PRINT_DST_STD());
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf,
		   "int = 1\n"
		   "str = \"str\"\n"
		   "lit = lit_LIT\n"
		   "arr = [\"str\", 1]\n"
		   "obj = {str = \"str\", int = 1}\n"
		   "\n"
		   "[tbl]\n"
		   "int = 1\n"
		   "str = \"str\"\n"
		   "\n"
		   "[tbll]\n");

	cfg_free(&cfg);
	cfg_prs_free(&prs);

	END;
}

STEST(cfg_parse)
{
	SSTART;

	RUN(cfg_prs_init_free);
	RUN(cfg_parse_null);
	RUN(cfg_parse_fail);
	RUN(cfg_parse_root);
	RUN(cfg_parse_tbl);
	RUN(cfg_parse_test);

	SEND;
}
