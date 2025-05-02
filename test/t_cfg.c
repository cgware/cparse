#include "file/cfg.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(cfg_init_free)
{
	START;

	cfg_t cfg = {0};

	EXPECT_EQ(cfg_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(cfg_init(&cfg, 1, 0, ALLOC_STD), NULL);
	EXPECT_EQ(cfg_init(&cfg, 0, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_init(&cfg, 0, 0, ALLOC_STD), &cfg);
	log_set_quiet(0, 0);

	cfg_free(&cfg);
	cfg_free(NULL);

	END;
}

TEST(cfg_var_init)
{
	START;

	cfg_t cfg = {0};

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(cfg_var_init(NULL, STRV_NULL, CFG_NULL), CFG_VAR_END);
	mem_oom(1);
	EXPECT_EQ(cfg_var_init(&cfg, STRV_NULL, CFG_NULL), CFG_VAR_END);
	EXPECT_EQ(cfg_var_init(&cfg, STRV(""), CFG_NULL), CFG_VAR_END);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_var_init(&cfg, STRV_NULL, (cfg_val_t){.type = -1}), CFG_VAR_END);
	log_set_quiet(0, 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_var_init_lit)
{
	START;

	cfg_t cfg = {0};

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(CFG_LIT(&cfg, STRV_NULL, STRV_NULL), CFG_VAR_END);
	mem_oom(0);
	EXPECT_EQ(CFG_LIT(&cfg, STRV_NULL, STRV_NULL), 0);
	cfg_free(&cfg);

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(CFG_LIT(&cfg, STRV_NULL, STRV("")), CFG_VAR_END);
	mem_oom(0);
	cfg_free(&cfg);

	END;
}

TEST(cfg_var_init_str)
{
	START;

	cfg_t cfg = {0};

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(CFG_STR(&cfg, STRV_NULL, STRV_NULL), CFG_VAR_END);
	mem_oom(0);
	EXPECT_EQ(CFG_STR(&cfg, STRV_NULL, STRV_NULL), 0);
	cfg_free(&cfg);

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(CFG_STR(&cfg, STRV_NULL, STRV("")), CFG_VAR_END);
	mem_oom(0);
	cfg_free(&cfg);

	END;
}

TEST(cfg_var_init_int)
{
	START;

	cfg_t cfg = {0};

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(CFG_INT(&cfg, STRV_NULL, 0), CFG_VAR_END);
	mem_oom(0);
	EXPECT_EQ(CFG_INT(&cfg, STRV_NULL, 0), 0);
	cfg_free(&cfg);

	END;
}

TEST(cfg_var_init_arr)
{
	START;

	cfg_t cfg = {0};

	log_set_quiet(0, 1);
	cfg_init(&cfg, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(CFG_ARR(&cfg, STRV_NULL), CFG_VAR_END);
	mem_oom(0);
	EXPECT_EQ(CFG_ARR(&cfg, STRV_NULL), 0);
	cfg_free(&cfg);

	END;
}

TEST(cfg_var_init_tbl)
{
	START;

	cfg_t cfg = {0};

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(CFG_TBL(&cfg, STRV_NULL), CFG_VAR_END);
	mem_oom(0);
	EXPECT_EQ(CFG_TBL(&cfg, STRV_NULL), 0);
	cfg_free(&cfg);

	END;
}

TEST(cfg_add_var)
{
	START;

	cfg_t cfg = {0};

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);
	EXPECT_EQ(cfg_add_var(NULL, CFG_VAR_END, CFG_VAR_END), CFG_VAR_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_add_var(&cfg, CFG_VAR_END, CFG_VAR_END), CFG_VAR_END);
	log_set_quiet(0, 0);
	cfg_var_t i = CFG_INT(&cfg, STRV_NULL, 0);
	EXPECT_EQ(cfg_add_var(&cfg, i, CFG_VAR_END), CFG_VAR_END);
	cfg_var_t root = CFG_ROOT(&cfg);
	EXPECT_NE(cfg_add_var(&cfg, root, CFG_INT(&cfg, STRV_NULL, 0)), CFG_VAR_END);
	cfg_free(&cfg);

	END;
}

TEST(cfg_get_var)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_var_t i = cfg_add_var(&cfg, root, CFG_INT(&cfg, STRV("a"), 2));

	cfg_var_t var;

	EXPECT_EQ(cfg_get_var(NULL, root, STRV_NULL, NULL), 1);
	EXPECT_EQ(cfg_get_var(&cfg, root, STRV_NULL, NULL), 1);
	EXPECT_EQ(cfg_get_var(&cfg, root, STRV("a"), &var), 0);
	EXPECT_EQ(var, i);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_var_not_found)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_add_var(&cfg, root, CFG_STR(&cfg, STRV("a"), STRV("int")));

	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_var(&cfg, root, STRV("int"), NULL), 1);
	log_set_quiet(0, 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_var_wrong_parent)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_INT(&cfg, STRV("int"), 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_var(&cfg, root, STRV("int"), NULL), 1);
	log_set_quiet(0, 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_val_wrong_type)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_var_t var = cfg_add_var(&cfg, root, CFG_STR(&cfg, STRV("int"), STRV("a")));

	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_int(&cfg, var, NULL), 1);
	log_set_quiet(0, 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_lit)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t var = CFG_LIT(&cfg, STRV("str"), STRV("val"));

	strv_t val;

	EXPECT_EQ(cfg_get_lit(NULL, CFG_VAR_END, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_lit(&cfg, CFG_VAR_END, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(cfg_get_lit(&cfg, var, &val), 0);
	EXPECT_STRN(val.data, "val", val.len);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_str)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t var = CFG_STR(&cfg, STRV("str"), STRV("val"));

	strv_t val;

	EXPECT_EQ(cfg_get_str(NULL, CFG_VAR_END, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_str(&cfg, CFG_VAR_END, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(cfg_get_str(&cfg, var, &val), 0);
	EXPECT_STRN(val.data, "val", val.len);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_int)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t var = CFG_INT(&cfg, STRV("int"), 1);

	int val;

	EXPECT_EQ(cfg_get_int(NULL, CFG_VAR_END, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_int(&cfg, CFG_VAR_END, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(cfg_get_int(&cfg, var, &val), 0);
	EXPECT_EQ(val, 1);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_arr)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t arr = CFG_ARR(&cfg, STRV("arr"));

	EXPECT_EQ(cfg_get_arr(NULL, CFG_VAR_END, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_arr(&cfg, CFG_VAR_END, NULL), 1);
	log_set_quiet(0, 0);
	cfg_var_t var = CFG_VAR_END;
	EXPECT_EQ(cfg_get_arr(&cfg, arr, &var), 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_arr_str)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t arr = CFG_ARR(&cfg, STRV("arr"));

	cfg_var_t vars[] = {
		cfg_add_var(&cfg, arr, CFG_STR(&cfg, STRV("str"), STRV("val"))),
		cfg_add_var(&cfg, arr, CFG_STR(&cfg, STRV("str"), STRV("val"))),
	};

	cfg_var_t var = CFG_VAR_END;
	int cnt	      = 0;
	while (cfg_get_arr(&cfg, arr, &var) == 0) {
		EXPECT_EQ(var, vars[cnt]);
		cnt++;
	};
	EXPECT_EQ(cnt, 2);

	cfg_free(&cfg);

	END;
}

typedef struct var_data_s {
	uint key;
	cfg_val_type_t type;
} var_data_t;

TEST(cfg_print)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_add_var(&cfg, root, CFG_LIT(&cfg, STRV("lit"), STRV("val")));
	cfg_add_var(&cfg, root, CFG_STR(&cfg, STRV("str"), STRV("str")));
	cfg_add_var(&cfg, root, CFG_INT(&cfg, STRV("int"), 1));
	cfg_add_var(&cfg, root, CFG_ARR(&cfg, STRV("arr")));
	cfg_add_var(&cfg, root, CFG_OBJ(&cfg, STRV("obj")));
	cfg_add_var(&cfg, root, CFG_TBL(&cfg, STRV("tbl")));

	EXPECT_EQ(cfg_print(NULL, root, DST_NONE()), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_print(&cfg, CFG_VAR_END, DST_NONE()), 0);
	log_set_quiet(0, 0);

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));

	EXPECT_STR(buf,
		   "lit = val\n"
		   "str = \"str\"\n"
		   "int = 1\n"
		   "arr = []\n"
		   "obj = {}\n"
		   "\n"
		   "[tbl]\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_none)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);
	cfg_var_t root	 = CFG_ROOT(&cfg);
	cfg_var_t none	 = cfg_add_var(&cfg, root, CFG_INT(&cfg, STRV("none"), 0));
	var_data_t *data = list_get(&cfg.vars, none);

	data->type = 0;

	char buf[1024] = {0};
	log_set_quiet(0, 1);
	cfg_print(&cfg, root, DST_BUF(buf));
	log_set_quiet(0, 0);

	EXPECT_STR(buf, "\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_lit)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_add_var(&cfg, root, CFG_LIT(&cfg, STRV("str"), STRV("val")));

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));

	EXPECT_STR(buf, "str = val\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_str)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_add_var(&cfg, root, CFG_STR(&cfg, STRV("str"), STRV("val")));

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));

	EXPECT_STR(buf, "str = \"val\"\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_arr)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_var_t arr = cfg_add_var(&cfg, root, CFG_ARR(&cfg, STRV("arr")));
	cfg_add_var(&cfg, arr, CFG_STR(&cfg, STRV_NULL, STRV("str")));
	cfg_add_var(&cfg, arr, CFG_INT(&cfg, STRV_NULL, 1));
	cfg_add_var(&cfg, arr, CFG_ARR(&cfg, STRV_NULL));
	cfg_add_var(&cfg, arr, CFG_OBJ(&cfg, STRV_NULL));

	cfg_var_t obj = cfg_add_var(&cfg, arr, CFG_OBJ(&cfg, STRV_NULL));
	cfg_add_var(&cfg, obj, CFG_STR(&cfg, STRV("str"), STRV("str")));
	cfg_add_var(&cfg, obj, CFG_INT(&cfg, STRV("int"), 1));

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));

	EXPECT_STR(buf, "arr = [\"str\", 1, [], {}, {str = \"str\", int = 1}]\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_obj)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_var_t obj = cfg_add_var(&cfg, root, CFG_OBJ(&cfg, STRV("obj")));
	cfg_add_var(&cfg, obj, CFG_STR(&cfg, STRV("str"), STRV("str")));
	cfg_add_var(&cfg, obj, CFG_INT(&cfg, STRV("int"), 1));
	cfg_var_t obj_obj = cfg_add_var(&cfg, obj, CFG_OBJ(&cfg, STRV("obj_obj")));
	cfg_add_var(&cfg, obj_obj, CFG_INT(&cfg, STRV("int"), 1));

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));

	EXPECT_STR(buf, "obj = {str = \"str\", int = 1, obj_obj = {int = 1}}\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_tbl)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_var_t tbl = cfg_add_var(&cfg, root, CFG_TBL(&cfg, STRV("tbl")));
	cfg_add_var(&cfg, tbl, CFG_STR(&cfg, STRV("str"), STRV("str")));
	cfg_add_var(&cfg, tbl, CFG_INT(&cfg, STRV("int"), 1));
	cfg_add_var(&cfg, tbl, CFG_ARR(&cfg, STRV("arr")));
	cfg_add_var(&cfg, tbl, CFG_OBJ(&cfg, STRV("obj")));

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));

	EXPECT_STR(buf,
		   "[tbl]\n"
		   "str = \"str\"\n"
		   "int = 1\n"
		   "arr = []\n"
		   "obj = {}\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_tbl1)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);
	cfg_add_var(&cfg, root, CFG_INT(&cfg, STRV("int"), 1));

	cfg_add_var(&cfg, root, CFG_TBL(&cfg, STRV("tbl")));

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));

	EXPECT_STR(buf,
		   "int = 1\n"
		   "\n"
		   "[tbl]\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_tbl2)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root = CFG_ROOT(&cfg);

	cfg_var_t tbl = cfg_add_var(&cfg, root, CFG_TBL(&cfg, STRV("tbl")));
	cfg_add_var(&cfg, tbl, CFG_STR(&cfg, STRV("str"), STRV("str")));
	cfg_add_var(&cfg, tbl, CFG_INT(&cfg, STRV("int"), 1));
	cfg_add_var(&cfg, tbl, CFG_ARR(&cfg, STRV("arr")));
	cfg_add_var(&cfg, tbl, CFG_OBJ(&cfg, STRV("obj")));

	cfg_add_var(&cfg, root, CFG_TBL(&cfg, STRV("tbll")));

	char buf[1024] = {0};
	cfg_print(&cfg, root, DST_BUF(buf));

	EXPECT_STR(buf,
		   "[tbl]\n"
		   "str = \"str\"\n"
		   "int = 1\n"
		   "arr = []\n"
		   "obj = {}\n"
		   "\n"
		   "[tbll]\n");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_root_str)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t str = CFG_STR(&cfg, STRV("str"), STRV("val"));

	char buf[1024] = {0};
	cfg_print(&cfg, str, DST_BUF(buf));

	EXPECT_STR(buf, "\"val\"");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_root_arr)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t arr = CFG_ARR(&cfg, STRV("arr"));

	cfg_add_var(&cfg, arr, CFG_STR(&cfg, STRV_NULL, STRV("str")));
	cfg_add_var(&cfg, arr, CFG_INT(&cfg, STRV_NULL, 1));
	cfg_add_var(&cfg, arr, CFG_ARR(&cfg, STRV_NULL));
	cfg_add_var(&cfg, arr, CFG_OBJ(&cfg, STRV_NULL));

	cfg_var_t obj = cfg_add_var(&cfg, arr, CFG_OBJ(&cfg, STRV_NULL));
	cfg_add_var(&cfg, obj, CFG_STR(&cfg, STRV("str"), STRV("str")));
	cfg_add_var(&cfg, obj, CFG_INT(&cfg, STRV("int"), 1));

	char buf[1024] = {0};
	cfg_print(&cfg, arr, DST_BUF(buf));

	EXPECT_STR(buf, "[\"str\", 1, [], {}, {str = \"str\", int = 1}]");

	cfg_free(&cfg);
	END;
}

TEST(cfg_print_root_tbl)
{
	START;

	cfg_t cfg = {0};

	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t tbl = CFG_TBL(&cfg, STRV("tbl"));

	cfg_add_var(&cfg, tbl, CFG_STR(&cfg, STRV("str"), STRV("str")));
	cfg_add_var(&cfg, tbl, CFG_INT(&cfg, STRV("int"), 1));
	cfg_add_var(&cfg, tbl, CFG_ARR(&cfg, STRV("arr")));
	cfg_add_var(&cfg, tbl, CFG_OBJ(&cfg, STRV("obj")));

	char buf[1024] = {0};
	cfg_print(&cfg, tbl, DST_BUF(buf));

	EXPECT_STR(buf,
		   "str = \"str\"\n"
		   "int = 1\n"
		   "arr = []\n"
		   "obj = {}\n");

	cfg_free(&cfg);
	END;
}

STEST(cfg)
{
	SSTART;

	RUN(cfg_init_free);
	RUN(cfg_var_init);
	RUN(cfg_var_init_lit);
	RUN(cfg_var_init_str);
	RUN(cfg_var_init_int);
	RUN(cfg_var_init_arr);
	RUN(cfg_var_init_tbl);
	RUN(cfg_add_var);
	RUN(cfg_get_var);
	RUN(cfg_get_var_not_found);
	RUN(cfg_get_var_wrong_parent);
	RUN(cfg_get_val_wrong_type);
	RUN(cfg_get_lit);
	RUN(cfg_get_str);
	RUN(cfg_get_int);
	RUN(cfg_get_arr);
	RUN(cfg_get_arr_str);
	RUN(cfg_print);
	RUN(cfg_print_none);
	RUN(cfg_print_lit);
	RUN(cfg_print_str);
	RUN(cfg_print_arr);
	RUN(cfg_print_obj);
	RUN(cfg_print_tbl);
	RUN(cfg_print_tbl1);
	RUN(cfg_print_tbl2);
	RUN(cfg_print_root_str);
	RUN(cfg_print_root_arr);
	RUN(cfg_print_root_tbl);

	SEND;
}
