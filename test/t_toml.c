#include "file/toml.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(toml_init_free)
{
	START;

	toml_t toml = {0};

	EXPECT_EQ(toml_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(toml_init(&toml, 1, 0, ALLOC_STD), NULL);
	EXPECT_EQ(toml_init(&toml, 0, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(toml_init(&toml, 0, 0, ALLOC_STD), &toml);
	log_set_quiet(0, 0);

	toml_free(&toml);
	toml_free(NULL);

	END;
}

TEST(toml_add_val)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(toml_add_val(NULL, STRV_NULL, TOML_NONE(), NULL), 1);
	mem_oom(1);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_NONE(), NULL), 1);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, (toml_add_val_t){.type = -1}, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_NONE(), NULL), 0);

	toml_free(&toml);

	END;
}

TEST(toml_add_str)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	toml_val_t val;
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_STR(STRV_NULL), &val), 0);
	EXPECT_EQ(val, 1);

	toml_free(&toml);

	log_set_quiet(0, 1);
	toml_init(&toml, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_STR(STRV_NULL), NULL), 1);
	mem_oom(0);
	toml_free(&toml);

	log_set_quiet(0, 1);
	toml_init(&toml, 2, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_STR(STRV_NULL), NULL), 1);
	mem_oom(0);
	toml_free(&toml);

	log_set_quiet(0, 1);
	toml_init(&toml, 2, 1, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_STR(STRV_NULL), NULL), 0);
	mem_oom(0);
	toml_free(&toml);

	END;
}

TEST(toml_add_int)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	toml_val_t val;
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_INT(0), &val), 0);
	EXPECT_EQ(val, 1);

	toml_free(&toml);

	log_set_quiet(0, 1);
	toml_init(&toml, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_INT(0), NULL), 1);
	mem_oom(0);
	toml_free(&toml);

	END;
}

TEST(toml_add_arr)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	toml_val_t val;
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_ARR(), &val), 0);
	EXPECT_EQ(val, 1);

	toml_free(&toml);

	log_set_quiet(0, 1);
	toml_init(&toml, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_ARR(), NULL), 1);
	mem_oom(0);
	toml_free(&toml);

	END;
}

TEST(toml_arr_add_val)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	toml_val_t arr;
	toml_add_val(&toml, STRV_NULL, TOML_ARR(), &arr);

	EXPECT_EQ(toml_arr_add_val(NULL, arr, TOML_INT(0), NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(toml_arr_add_val(&toml, ARR_END, TOML_INT(0), NULL), 1);
	EXPECT_EQ(toml_arr_add_val(&toml, arr, (toml_add_val_t){.type = -1}, NULL), 1);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(toml_arr_add_val(&toml, arr, TOML_INT(0), NULL), 1);
	mem_oom(0);

	toml_val_t val;
	EXPECT_EQ(toml_arr_add_val(&toml, arr, TOML_INT(0), &val), 0);
	EXPECT_EQ(val, 2);
	EXPECT_EQ(toml_arr_add_val(&toml, arr, TOML_NONE(), NULL), 0);

	toml_free(&toml);

	END;
}

TEST(toml_add_tbl)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	toml_val_t val;
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_TBL(), &val), 0);
	EXPECT_EQ(val, 1);

	toml_free(&toml);

	log_set_quiet(0, 1);
	toml_init(&toml, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(toml_add_val(&toml, STRV_NULL, TOML_TBL(), NULL), 1);
	mem_oom(0);
	toml_free(&toml);

	END;
}

TEST(toml_tbl_add_val)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 3, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	toml_val_t tbl;
	toml_add_val(&toml, STRV_NULL, TOML_TBL(), &tbl);

	EXPECT_EQ(toml_tbl_add_val(NULL, tbl, STRV_NULL, TOML_INT(0), NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(toml_tbl_add_val(&toml, ARR_END, STRV_NULL, TOML_INT(0), NULL), 1);
	EXPECT_EQ(toml_tbl_add_val(&toml, tbl, STRV_NULL, (toml_add_val_t){.type = -1}, NULL), 1);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(toml_tbl_add_val(&toml, tbl, STRV_NULL, TOML_INT(0), NULL), 1);
	mem_oom(0);

	toml_val_t val;
	EXPECT_EQ(toml_tbl_add_val(&toml, tbl, STRV_NULL, TOML_INT(0), &val), 0);
	EXPECT_EQ(val, 2);
	EXPECT_EQ(toml_tbl_add_val(&toml, tbl, STRV_NULL, TOML_NONE(), NULL), 0);

	toml_free(&toml);

	END;
}

typedef struct val_data_s {
	uint key;
	toml_val_type_t type;
} val_data_t;

TEST(toml_print)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);
	toml_add_val(&toml, STRVC("string"), TOML_STR(STRVC("str")), NULL);
	toml_add_val(&toml, STRVC("int"), TOML_INT(1), NULL);
	toml_add_val(&toml, STRVC("arr"), TOML_ARR(), NULL);
	toml_add_val(&toml, STRVC("inl"), TOML_INL(), NULL);
	toml_add_val(&toml, STRVC("tbl"), TOML_TBL(), NULL);
	toml_add_val(&toml, STRVC("tbl_arr"), TOML_TBL_ARR(), NULL);

	EXPECT_EQ(toml_print(NULL, PRINT_DST_NONE()), 0);

	char buf[1024] = {0};
	toml_print(&toml, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf,
		   "string = \"str\"\n"
		   "int = 1\n"
		   "arr = []\n"
		   "inl = {}\n"
		   "[tbl]\n"
		   "\n"
		   "[[tbl_arr]]\n"
		   "\n");

	toml_free(&toml);
	END;
}

TEST(toml_print_none)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);
	toml_val_t none;
	toml_add_val(&toml, STRVC("none"), TOML_INT(0), &none);
	val_data_t *data = tree_get_data(&toml.vals, none);

	data->type = 0;

	char buf[1024] = {0};
	log_set_quiet(0, 1);
	toml_print(&toml, PRINT_DST_BUF(buf, sizeof(buf), 0));
	log_set_quiet(0, 0);

	EXPECT_STR(buf, "");

	toml_free(&toml);
	END;
}

TEST(toml_print_arr)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);

	toml_val_t arr;
	toml_add_val(&toml, STRVC("arr"), TOML_ARR(), &arr);
	toml_arr_add_val(&toml, arr, TOML_STR(STRVC("str")), NULL);
	toml_arr_add_val(&toml, arr, TOML_INT(1), NULL);
	toml_arr_add_val(&toml, arr, TOML_ARR(), NULL);
	toml_arr_add_val(&toml, arr, TOML_TBL(), NULL);
	toml_arr_add_val(&toml, arr, TOML_INL(), NULL);

	toml_val_t tbl;
	toml_arr_add_val(&toml, arr, TOML_TBL(), &tbl);
	toml_tbl_add_val(&toml, tbl, STRVC("string"), TOML_STR(STRVC("str")), NULL);
	toml_tbl_add_val(&toml, tbl, STRVC("int"), TOML_INT(1), NULL);

	EXPECT_EQ(toml_print(NULL, PRINT_DST_NONE()), 0);

	char buf[1024] = {0};
	toml_print(&toml, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "arr = [\"str\", 1, [], {}, {}, {string = \"str\", int = 1}]\n");

	toml_free(&toml);
	END;
}

TEST(toml_print_inl)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);

	toml_val_t inl;
	toml_add_val(&toml, STRVC("inl"), TOML_INL(), &inl);
	toml_tbl_add_val(&toml, inl, STRVC("string"), TOML_STR(STRVC("str")), NULL);
	toml_tbl_add_val(&toml, inl, STRVC("int"), TOML_INT(1), NULL);
	toml_val_t inl_inl;
	toml_tbl_add_val(&toml, inl, STRVC("inl_inl"), TOML_INL(), &inl_inl);
	toml_tbl_add_val(&toml, inl_inl, STRVC("int"), TOML_INT(1), NULL);

	char buf[1024] = {0};
	toml_print(&toml, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "inl = {string = \"str\", int = 1, inl_inl = {int = 1}}\n");

	toml_free(&toml);
	END;
}

TEST(toml_print_tbl)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);

	toml_val_t tbl;
	toml_add_val(&toml, STRVC("tbl"), TOML_TBL(), &tbl);
	toml_tbl_add_val(&toml, tbl, STRVC("string"), TOML_STR(STRVC("str")), NULL);
	toml_tbl_add_val(&toml, tbl, STRVC("int"), TOML_INT(1), NULL);
	toml_tbl_add_val(&toml, tbl, STRVC("arr"), TOML_ARR(), NULL);
	toml_tbl_add_val(&toml, tbl, STRVC("inl"), TOML_INL(), NULL);
	toml_tbl_add_val(&toml, tbl, STRVC("tbl.tbl"), TOML_TBL(), NULL);

	char buf[1024] = {0};
	toml_print(&toml, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf,
		   "[tbl]\n"
		   "string = \"str\"\n"
		   "int = 1\n"
		   "arr = []\n"
		   "inl = {}\n"
		   "[tbl.tbl]\n"
		   "\n");

	toml_free(&toml);
	END;
}

TEST(toml_print_tbl_arr)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);

	toml_val_t tbl_arr;
	toml_add_val(&toml, STRVC("tblarr"), TOML_TBL_ARR(), &tbl_arr);
	toml_tbl_add_val(&toml, tbl_arr, STRVC("string"), TOML_STR(STRVC("str")), NULL);
	toml_tbl_add_val(&toml, tbl_arr, STRVC("int"), TOML_INT(1), NULL);

	toml_val_t tbl;
	toml_tbl_add_val(&toml, tbl_arr, STRVC("tblarr.tbl"), TOML_TBL(), &tbl);

	toml_val_t inl;
	toml_tbl_add_val(&toml, tbl, STRVC("inl"), TOML_INL(), &inl);

	toml_val_t inl_tbl_arr;
	toml_tbl_add_val(&toml, inl, STRVC("inl_tbl_arr"), TOML_TBL_ARR(), &inl_tbl_arr);

	toml_val_t tbl0;
	toml_arr_add_val(&toml, inl_tbl_arr, TOML_TBL(), &tbl0);
	toml_tbl_add_val(&toml, tbl0, STRVC("int"), TOML_INT(1), NULL);

	char buf[1024] = {0};
	toml_print(&toml, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf,
		   "[[tblarr]]\n"
		   "string = \"str\"\n"
		   "int = 1\n"
		   "[tblarr.tbl]\n"
		   "inl = {inl_tbl_arr = [{int = 1}]}\n"
		   "\n");

	toml_free(&toml);
	END;
}

STEST(toml)
{
	SSTART;

	RUN(toml_init_free);
	RUN(toml_add_val);
	RUN(toml_add_str);
	RUN(toml_add_int);
	RUN(toml_add_arr);
	RUN(toml_arr_add_val);
	RUN(toml_add_tbl);
	RUN(toml_tbl_add_val);
	RUN(toml_print);
	RUN(toml_print_none);
	RUN(toml_print_arr);
	RUN(toml_print_inl);
	RUN(toml_print_tbl);
	RUN(toml_print_tbl_arr);

	SEND;
}
