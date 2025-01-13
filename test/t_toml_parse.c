#include "file/toml_parse.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(toml_prs_init_free)
{
	START;

	toml_prs_t prs = {0};

	EXPECT_EQ(toml_prs_init(NULL, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(toml_prs_init(&prs, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(toml_prs_init(&prs, ALLOC_STD), &prs);

	toml_prs_free(&prs);
	toml_prs_free(NULL);

	END;
}

TEST(toml_parse_null)
{
	START;

	toml_prs_t prs = {0};
	toml_prs_init(&prs, ALLOC_STD);

	toml_t toml = {0};
	toml_init(&toml, 1, 1, ALLOC_STD);

	strv_t str = STRV("");

	EXPECT_EQ(toml_prs_parse(NULL, STRV_NULL, NULL, ALLOC_STD, PRINT_DST_NONE()), TOML_VAL_END);
	EXPECT_EQ(toml_prs_parse(&prs, STRV_NULL, NULL, ALLOC_STD, PRINT_DST_NONE()), TOML_VAL_END);
	EXPECT_EQ(toml_prs_parse(&prs, str, NULL, ALLOC_STD, PRINT_DST_NONE()), TOML_VAL_END);
	mem_oom(1);
	EXPECT_EQ(toml_prs_parse(&prs, str, &toml, ALLOC_STD, PRINT_DST_NONE()), TOML_VAL_END);
	mem_oom(0);
	EXPECT_EQ(toml_prs_parse(&prs, str, &toml, ALLOC_STD, PRINT_DST_NONE()), 0);

	toml_free(&toml);
	toml_prs_free(&prs);

	END;
}

TEST(toml_parse_fail)
{
	START;

	toml_prs_t prs = {0};
	toml_prs_init(&prs, ALLOC_STD);

	toml_t toml = {0};
	toml_init(&toml, 1, 1, ALLOC_STD);

	strv_t str = STRV("int0 = 1\n");

	EXPECT_EQ(toml_prs_parse(&prs, str, &toml, ALLOC_STD, PRINT_DST_NONE()), TOML_VAL_END);

	toml_free(&toml);
	toml_prs_free(&prs);

	END;
}

TEST(toml_parse_test)
{
	START;

	toml_prs_t prs = {0};
	toml_prs_init(&prs, ALLOC_STD);

	toml_t toml = {0};
	toml_init(&toml, 1, 1, ALLOC_STD);

	strv_t str = STRV("int = 1\n"
			  "strl = 'str'\n"
			  "arr = ['str', 1]\n"
			  "inl = {strl = 'str', int = 1}\n"
			  "[tbl]\n"
			  "int = 1\n"
			  "strl = 'str'\n"
			  "[[tblarr]]\n"
			  "int = 1\n"
			  "strl = 'str'\n"
			  "\n"
			  "[tbll]\n");

	toml_val_t root = toml_prs_parse(&prs, str, &toml, ALLOC_STD, PRINT_DST_NONE());
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	toml_print(&toml, root, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf,
		   "int = 1\n"
		   "strl = 'str'\n"
		   "arr = ['str', 1]\n"
		   "inl = {strl = 'str', int = 1}\n"
		   "\n"
		   "[tbl]\n"
		   "int = 1\n"
		   "strl = 'str'\n"
		   "[[tblarr]]\n"
		   "int = 1\n"
		   "strl = 'str'\n"
		   "\n"
		   "[tbll]\n");

	toml_free(&toml);
	toml_prs_free(&prs);

	END;
}

STEST(toml_parse)
{
	SSTART;

	RUN(toml_prs_init_free);
	RUN(toml_parse_null);
	RUN(toml_parse_fail);
	RUN(toml_parse_test);

	SEND;
}
