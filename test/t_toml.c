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

TEST(toml_val_init)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(toml_val_init(NULL, STRV_NULL, TOML_NULL), TOML_VAL_END);
	mem_oom(1);
	EXPECT_EQ(toml_val_init(&toml, STRV_NULL, TOML_NULL), TOML_VAL_END);
	EXPECT_EQ(toml_val_init(&toml, STRV(""), TOML_NULL), TOML_VAL_END);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(toml_val_init(&toml, STRV_NULL, (toml_add_val_t){.type = -1}), TOML_VAL_END);
	log_set_quiet(0, 0);

	toml_free(&toml);

	END;
}

TEST(toml_val_init_strl)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(TOML_STRL(&toml, STRV_NULL, STRV_NULL), TOML_VAL_END);
	mem_oom(0);
	EXPECT_EQ(TOML_STRL(&toml, STRV_NULL, STRV_NULL), 0);
	toml_free(&toml);

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(TOML_STRL(&toml, STRV_NULL, STRV("")), TOML_VAL_END);
	mem_oom(0);
	toml_free(&toml);

	END;
}

TEST(toml_val_init_int)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(TOML_INT(&toml, STRV_NULL, 0), TOML_VAL_END);
	mem_oom(0);
	EXPECT_EQ(TOML_INT(&toml, STRV_NULL, 0), 0);
	toml_free(&toml);

	END;
}

TEST(toml_val_init_arr)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(TOML_ARR(&toml, STRV_NULL), TOML_VAL_END);
	mem_oom(0);
	EXPECT_EQ(TOML_ARR(&toml, STRV_NULL), 0);
	toml_free(&toml);

	END;
}

TEST(toml_val_init_tbl)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(TOML_TBL(&toml, STRV_NULL), TOML_VAL_END);
	mem_oom(0);
	EXPECT_EQ(TOML_TBL(&toml, STRV_NULL), 0);
	toml_free(&toml);

	END;
}

TEST(toml_add_val)
{
	START;

	toml_t toml = {0};

	log_set_quiet(0, 1);
	toml_init(&toml, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	EXPECT_EQ(toml_add_val(NULL, TOML_VAL_END, TOML_VAL_END), TOML_VAL_END);
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
	toml_val_t root = toml_add_val(&toml, TOML_VAL_END, TOML_VAL_END);
	toml_add_val(&toml, root, TOML_STRL(&toml, STRV("strl"), STRV("str")));
	toml_add_val(&toml, root, TOML_INT(&toml, STRV("int"), 1));
	toml_add_val(&toml, root, TOML_ARR(&toml, STRV("arr")));
	toml_add_val(&toml, root, TOML_INL(&toml, STRV("inl")));
	toml_add_val(&toml, root, TOML_TBL(&toml, STRV("tbl")));
	toml_add_val(&toml, root, TOML_TBL_ARR(&toml, STRV("tbl_arr")));

	EXPECT_EQ(toml_print(NULL, TOML_VAL_END, PRINT_DST_NONE()), 0);

	char buf[1024] = {0};
	toml_print(&toml, root, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf,
		   "strl = 'str'\n"
		   "int = 1\n"
		   "arr = []\n"
		   "inl = {}\n"
		   "\n"
		   "[tbl]\n"
		   "\n"
		   "[[tbl_arr]]\n");

	toml_free(&toml);
	END;
}

TEST(toml_print_none)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);
	toml_val_t root = toml_add_val(&toml, TOML_VAL_END, TOML_VAL_END);

	toml_val_t none	 = toml_add_val(&toml, root, TOML_INT(&toml, STRV("none"), 0));
	val_data_t *data = tree_get_data(&toml.vals, none);

	data->type = 0;

	char buf[1024] = {0};
	log_set_quiet(0, 1);
	toml_print(&toml, root, PRINT_DST_BUF(buf, sizeof(buf), 0));
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
	toml_val_t root = toml_add_val(&toml, TOML_VAL_END, TOML_VAL_END);

	toml_val_t arr = toml_add_val(&toml, root, TOML_ARR(&toml, STRV("arr")));
	toml_add_val(&toml, arr, TOML_STRL(&toml, STRV_NULL, STRV("str")));
	toml_add_val(&toml, arr, TOML_INT(&toml, STRV_NULL, 1));
	toml_add_val(&toml, arr, TOML_ARR(&toml, STRV_NULL));
	toml_add_val(&toml, arr, TOML_TBL(&toml, STRV_NULL));
	toml_add_val(&toml, arr, TOML_INL(&toml, STRV_NULL));

	toml_val_t tbl = toml_add_val(&toml, arr, TOML_TBL(&toml, STRV_NULL));
	toml_add_val(&toml, tbl, TOML_STRL(&toml, STRV("strl"), STRV("str")));
	toml_add_val(&toml, tbl, TOML_INT(&toml, STRV("int"), 1));

	char buf[1024] = {0};
	toml_print(&toml, root, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "arr = ['str', 1, [], {}, {}, {strl = 'str', int = 1}]\n");

	toml_free(&toml);
	END;
}

TEST(toml_print_inl)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);
	toml_val_t root = toml_add_val(&toml, TOML_VAL_END, TOML_VAL_END);

	toml_val_t inl = toml_add_val(&toml, root, TOML_INL(&toml, STRV("inl")));
	toml_add_val(&toml, inl, TOML_STRL(&toml, STRV("strl"), STRV("str")));
	toml_add_val(&toml, inl, TOML_INT(&toml, STRV("int"), 1));
	toml_val_t inl_inl = toml_add_val(&toml, inl, TOML_INL(&toml, STRV("inl_inl")));
	toml_add_val(&toml, inl_inl, TOML_INT(&toml, STRV("int"), 1));

	char buf[1024] = {0};
	toml_print(&toml, root, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf, "inl = {strl = 'str', int = 1, inl_inl = {int = 1}}\n");

	toml_free(&toml);
	END;
}

TEST(toml_print_tbl)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);
	toml_val_t root = toml_add_val(&toml, TOML_VAL_END, TOML_VAL_END);

	toml_val_t tbl = toml_add_val(&toml, root, TOML_TBL(&toml, STRV("tbl")));
	toml_add_val(&toml, tbl, TOML_STRL(&toml, STRV("strl"), STRV("str")));
	toml_add_val(&toml, tbl, TOML_INT(&toml, STRV("int"), 1));
	toml_add_val(&toml, tbl, TOML_ARR(&toml, STRV("arr")));
	toml_add_val(&toml, tbl, TOML_INL(&toml, STRV("inl")));
	toml_add_val(&toml, tbl, TOML_TBL(&toml, STRV("tbl.tbl")));

	char buf[1024] = {0};
	toml_print(&toml, root, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf,
		   "[tbl]\n"
		   "strl = 'str'\n"
		   "int = 1\n"
		   "arr = []\n"
		   "inl = {}\n"
		   "[tbl.tbl]\n");

	toml_free(&toml);
	END;
}

TEST(toml_print_tbl_arr)
{
	START;

	toml_t toml = {0};

	toml_init(&toml, 1, 1, ALLOC_STD);
	toml_val_t root = toml_add_val(&toml, TOML_VAL_END, TOML_VAL_END);

	toml_val_t tbl_arr = toml_add_val(&toml, root, TOML_TBL_ARR(&toml, STRV("tblarr")));
	toml_add_val(&toml, tbl_arr, TOML_STRL(&toml, STRV("strl"), STRV("str")));
	toml_add_val(&toml, tbl_arr, TOML_INT(&toml, STRV("int"), 1));

	toml_val_t tbl	= toml_add_val(&toml, tbl_arr, TOML_TBL(&toml, STRV("tblarr.tbl")));
	toml_val_t inl	= toml_add_val(&toml, tbl, TOML_INL(&toml, STRV("inl")));
	toml_val_t ita	= toml_add_val(&toml, inl, TOML_TBL_ARR(&toml, STRV("inl_tbl_arr")));
	toml_val_t tbl0 = toml_add_val(&toml, ita, TOML_TBL(&toml, STRV_NULL));
	toml_add_val(&toml, tbl0, TOML_INT(&toml, STRV("int"), 1));

	char buf[1024] = {0};
	toml_print(&toml, root, PRINT_DST_BUF(buf, sizeof(buf), 0));

	EXPECT_STR(buf,
		   "[[tblarr]]\n"
		   "strl = 'str'\n"
		   "int = 1\n"
		   "[tblarr.tbl]\n"
		   "inl = {inl_tbl_arr = [{int = 1}]}\n");

	toml_free(&toml);
	END;
}

STEST(toml)
{
	SSTART;

	RUN(toml_init_free);
	RUN(toml_val_init);
	RUN(toml_val_init_strl);
	RUN(toml_val_init_int);
	RUN(toml_val_init_arr);
	RUN(toml_val_init_tbl);
	RUN(toml_add_val);
	RUN(toml_print);
	RUN(toml_print_none);
	RUN(toml_print_arr);
	RUN(toml_print_inl);
	RUN(toml_print_tbl);
	RUN(toml_print_tbl_arr);

	SEND;
}
