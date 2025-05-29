#include "file/cfg.h"
#include "file/cfg_priv.h"

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

TEST(cfg_root)
{
	START;

	cfg_t cfg = {0};
	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(cfg_root(NULL, NULL), 1);
	EXPECT_EQ(cfg_root(&cfg, NULL), 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_lit)
{
	START;

	cfg_t cfg = {0};
	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	cfg_var_t lit;

	EXPECT_EQ(cfg_lit(NULL, STRV_NULL, STRV_NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(cfg_lit(&cfg, STRV_NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(cfg_lit(&cfg, STRV(""), STRV_NULL, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(cfg_lit(&cfg, STRV_NULL, STRV_NULL, &lit), 0);
	EXPECT_EQ(lit, 0);
	cfg_free(&cfg);

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(cfg_lit(&cfg, STRV_NULL, STRV(""), NULL), 1);
	mem_oom(0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_str)
{
	START;

	cfg_t cfg = {0};
	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	cfg_var_t str;

	EXPECT_EQ(cfg_str(NULL, STRV_NULL, STRV_NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(cfg_str(&cfg, STRV_NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(cfg_str(&cfg, STRV(""), STRV_NULL, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(cfg_str(&cfg, STRV_NULL, STRV_NULL, &str), 0);
	EXPECT_EQ(str, 0);
	cfg_free(&cfg);

	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(cfg_str(&cfg, STRV_NULL, STRV(""), NULL), 1);
	mem_oom(0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_int)
{
	START;

	cfg_t cfg = {0};
	log_set_quiet(0, 1);
	cfg_init(&cfg, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	cfg_var_t val;

	mem_oom(1);
	EXPECT_EQ(cfg_int(&cfg, STRV_NULL, 0, NULL), 1);
	EXPECT_EQ(cfg_int(&cfg, STRV(""), 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(cfg_int(&cfg, STRV_NULL, 0, &val), 0);
	EXPECT_EQ(val, 0);
	cfg_free(&cfg);

	END;
}

TEST(cfg_arr)
{
	START;

	cfg_t cfg = {0};
	log_set_quiet(0, 1);
	cfg_init(&cfg, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	cfg_var_t arr;

	mem_oom(1);
	EXPECT_EQ(cfg_arr(&cfg, STRV_NULL, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(cfg_arr(&cfg, STRV_NULL, &arr), 0);
	EXPECT_EQ(arr, 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_obj)
{
	START;

	cfg_t cfg = {0};
	log_set_quiet(0, 1);
	cfg_init(&cfg, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	cfg_var_t obj;

	mem_oom(1);
	EXPECT_EQ(cfg_obj(&cfg, STRV_NULL, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(cfg_obj(&cfg, STRV_NULL, &obj), 0);
	EXPECT_EQ(obj, 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_tbl)
{
	START;

	cfg_t cfg = {0};
	log_set_quiet(0, 1);
	cfg_init(&cfg, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	cfg_var_t tbl;

	mem_oom(1);
	EXPECT_EQ(cfg_tbl(&cfg, STRV_NULL, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(cfg_tbl(&cfg, STRV_NULL, &tbl), 0);
	EXPECT_EQ(tbl, 0);

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

	cfg_var_t i, root;

	EXPECT_EQ(cfg_add_var(NULL, cfg.vars.cnt, cfg.vars.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_add_var(&cfg, cfg.vars.cnt, cfg.vars.cnt), 1);
	log_set_quiet(0, 0);

	cfg_int(&cfg, STRV_NULL, 0, &i);
	EXPECT_EQ(cfg_add_var(&cfg, i, cfg.vars.cnt), 1);
	cfg_root(&cfg, &root);
	EXPECT_EQ(cfg_add_var(&cfg, root, i), 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_has_var)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root, i, var;

	cfg_root(&cfg, &root);
	cfg_int(&cfg, STRV("a"), 2, &i);
	
	EXPECT_EQ(cfg_has_var(NULL, cfg.vars.cnt, STRV_NULL, NULL), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_has_var(&cfg, cfg.vars.cnt, STRV_NULL, NULL), 0);
	EXPECT_EQ(cfg_has_var(&cfg, root, STRV_NULL, NULL), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(cfg_has_var(&cfg, root, STRV("a"), &var), 0);
	cfg_add_var(&cfg, root, i);
	EXPECT_EQ(cfg_has_var(&cfg, root, STRV("a"), &var), 1);
	EXPECT_EQ(var, i);

	cfg_free(&cfg);

	END;
}

TEST(cfg_has_var_not_found)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root, str;

	cfg_root(&cfg, &root);
	cfg_str(&cfg, STRV("a"), STRV("int"), &str);
	cfg_add_var(&cfg, root, str);

	EXPECT_EQ(cfg_has_var(&cfg, root, STRV("int"), NULL), 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_has_var_wrong_parent)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root;

	cfg_int(&cfg, STRV("int"), 0, &root);

	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_has_var(&cfg, root, STRV("int"), NULL), 0);
	log_set_quiet(0, 0);

	cfg_free(&cfg);

	END;
}

TEST(cfg_get_val_wrong_type)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root, var;

	cfg_root(&cfg, &root);

	cfg_str(&cfg, STRV("int"), STRV("a"), &var);
	cfg_add_var(&cfg, root, var);

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

	cfg_var_t var;
	strv_t val;

	cfg_lit(&cfg, STRV("str"), STRV("val"), &var);

	EXPECT_EQ(cfg_get_lit(NULL, cfg.vars.cnt, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_lit(&cfg, cfg.vars.cnt, NULL), 1);
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

	cfg_var_t var;
	strv_t val;

	cfg_str(&cfg, STRV("str"), STRV("val"), &var);

	EXPECT_EQ(cfg_get_str(NULL, cfg.vars.cnt, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_str(&cfg, cfg.vars.cnt, NULL), 1);
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

	cfg_var_t var;
	int val;

	cfg_int(&cfg, STRV("int"), 1, &var);

	EXPECT_EQ(cfg_get_int(NULL, cfg.vars.cnt, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_get_int(&cfg, cfg.vars.cnt, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(cfg_get_int(&cfg, var, &val), 0);
	EXPECT_EQ(val, 1);

	cfg_free(&cfg);

	END;
}

TEST(cfg_it_begin)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 2, 2, ALLOC_STD);

	cfg_var_t arr, var, it;

	cfg_arr(&cfg, STRV("arr"), &arr);
	cfg_int(&cfg, STRV("str"), 0, &var);

	EXPECT_EQ(cfg_it_begin(NULL, cfg.vars.cnt, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_it_begin(&cfg, cfg.vars.cnt, NULL), NULL);
	EXPECT_EQ(cfg_it_begin(&cfg, var, NULL), NULL);
	log_set_quiet(0, 0);
	EXPECT_EQ(cfg_it_begin(&cfg, arr, NULL), NULL);
	cfg_add_var(&cfg, arr, var);
	EXPECT_NE(cfg_it_begin(&cfg, arr, &it), NULL);
	EXPECT_EQ(it, var);

	cfg_free(&cfg);

	END;
}

TEST(cfg_it_next)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t arr, var0, var1;

	cfg_arr(&cfg, STRV("arr"), &arr);
	cfg_int(&cfg, STRV("str"), 0, &var0);
	cfg_int(&cfg, STRV("str"), 1, &var1);

	cfg_var_t it = cfg.vars.cnt;

	EXPECT_EQ(cfg_it_next(NULL, NULL), NULL);
	EXPECT_EQ(cfg_it_next(&cfg, NULL), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_it_next(&cfg, &it), NULL);
	log_set_quiet(0, 0);
	cfg_add_var(&cfg, arr, var0);
	it = var0;
	EXPECT_EQ(cfg_it_next(&cfg, &it), NULL);
	cfg_add_var(&cfg, arr, var1);
	it = var0;
	EXPECT_NE(cfg_it_next(&cfg, &it), NULL);
	EXPECT_EQ(it, var1);
	EXPECT_EQ(cfg_it_next(&cfg, &it), NULL);

	cfg_free(&cfg);

	END;
}

TEST(cfg_it)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 3, 3, ALLOC_STD);

	cfg_var_t arr, it, vars[2];

	cfg_arr(&cfg, STRV("arr"), &arr);
	cfg_int(&cfg, STRV("str"), 0, &vars[0]);
	cfg_int(&cfg, STRV("str"), 1, &vars[1]);
	cfg_add_var(&cfg, arr, vars[0]);
	cfg_add_var(&cfg, arr, vars[1]);

	int cnt = 0;
	void *data;
	cfg_foreach(&cfg, arr, data, &it)
	{
		EXPECT_EQ(it, vars[cnt]);
		cnt++;
	}
	EXPECT_EQ(cnt, 2);

	cfg_free(&cfg);

	END;
}

TEST(cfg_print)
{
	START;

	cfg_t cfg = {0};
	cfg_init(&cfg, 1, 1, ALLOC_STD);

	cfg_var_t root, var;

	cfg_root(&cfg, &root);

	char buf[64] = {0};
	EXPECT_EQ(cfg_print(&cfg, root, DST_BUF(buf)), 0);

	cfg_lit(&cfg, STRV("lit"), STRV("val"), &var);
	cfg_add_var(&cfg, root, var);
	cfg_str(&cfg, STRV("str"), STRV("str"), &var);
	cfg_add_var(&cfg, root, var);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, root, var);
	cfg_arr(&cfg, STRV("arr"), &var);
	cfg_add_var(&cfg, root, var);
	cfg_obj(&cfg, STRV("obj"), &var);
	cfg_add_var(&cfg, root, var);
	cfg_tbl(&cfg, STRV("tbl"), &var);
	cfg_add_var(&cfg, root, var);

	EXPECT_EQ(cfg_print(NULL, root, DST_NONE()), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(cfg_print(&cfg, cfg.vars.cnt, DST_NONE()), 0);
	log_set_quiet(0, 0);

	EXPECT_EQ(cfg_print(&cfg, root, DST_BUF(buf)), 55);
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

	cfg_var_t root, none;

	cfg_root(&cfg, &root);
	cfg_int(&cfg, STRV("none"), 0, &none);
	cfg_add_var(&cfg, root, none);

	cfg_var_data_t *data = list_get(&cfg.vars, none);

	data->type = 0;

	char buf[16] = {0};
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

	cfg_var_t root, var;

	cfg_root(&cfg, &root);
	cfg_lit(&cfg, STRV("str"), STRV("val"), &var);
	cfg_add_var(&cfg, root, var);

	char buf[16] = {0};
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

	cfg_var_t root, var;

	cfg_root(&cfg, &root);
	cfg_str(&cfg, STRV("str"), STRV("val"), &var);
	cfg_add_var(&cfg, root, var);

	char buf[16] = {0};
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

	cfg_var_t root, arr, var, obj;

	cfg_root(&cfg, &root);
	cfg_arr(&cfg, STRV("arr"), &arr);
	cfg_add_var(&cfg, root, arr);
	cfg_str(&cfg, STRV_NULL, STRV("str"), &var);
	cfg_add_var(&cfg, arr, var);
	cfg_int(&cfg, STRV_NULL, 1, &var);
	cfg_add_var(&cfg, arr, var);
	cfg_arr(&cfg, STRV_NULL, &var);
	cfg_add_var(&cfg, arr, var);
	cfg_obj(&cfg, STRV_NULL, &var);
	cfg_add_var(&cfg, arr, var);

	cfg_obj(&cfg, STRV_NULL, &obj);
	cfg_add_var(&cfg, arr, obj);
	cfg_str(&cfg, STRV("str"), STRV("str"), &var);
	cfg_add_var(&cfg, obj, var);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, obj, var);

	char buf[64] = {0};
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

	cfg_var_t root, obj, var, obj_obj;

	cfg_root(&cfg, &root);
	cfg_obj(&cfg, STRV("obj"), &obj);
	cfg_add_var(&cfg, root, obj);
	cfg_str(&cfg, STRV("str"), STRV("str"), &var);
	cfg_add_var(&cfg, obj, var);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, obj, var);
	cfg_obj(&cfg, STRV("obj_obj"), &obj_obj);
	cfg_add_var(&cfg, obj, obj_obj);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, obj_obj, var);

	char buf[64] = {0};
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

	cfg_var_t root, tbl, var;

	cfg_root(&cfg, &root);
	cfg_tbl(&cfg, STRV("tbl"), &tbl);
	cfg_add_var(&cfg, root, tbl);
	cfg_str(&cfg, STRV("str"), STRV("str"), &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_arr(&cfg, STRV("arr"), &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_obj(&cfg, STRV("obj"), &var);
	cfg_add_var(&cfg, tbl, var);

	char buf[64] = {0};
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

	cfg_var_t root, var;

	cfg_root(&cfg, &root);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, root, var);
	cfg_tbl(&cfg, STRV("tbl"), &var);
	cfg_add_var(&cfg, root, var);

	char buf[32] = {0};
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

	cfg_var_t root, tbl, var;

	cfg_root(&cfg, &root);
	cfg_tbl(&cfg, STRV("tbl"), &tbl);
	cfg_add_var(&cfg, root, tbl);
	cfg_str(&cfg, STRV("str"), STRV("str"), &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_arr(&cfg, STRV("arr"), &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_obj(&cfg, STRV("obj"), &var);
	cfg_add_var(&cfg, tbl, var);

	cfg_tbl(&cfg, STRV("tbll"), &var);
	cfg_add_var(&cfg, root, var);

	char buf[64] = {0};
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

	cfg_var_t str;

	cfg_str(&cfg, STRV("str"), STRV("val"), &str);

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

	cfg_var_t arr, var, obj;

	cfg_arr(&cfg, STRV("arr"), &arr);
	cfg_str(&cfg, STRV_NULL, STRV("str"), &var);
	cfg_add_var(&cfg, arr, var);
	cfg_int(&cfg, STRV_NULL, 1, &var);
	cfg_add_var(&cfg, arr, var);
	cfg_arr(&cfg, STRV_NULL, &var);
	cfg_add_var(&cfg, arr, var);
	cfg_obj(&cfg, STRV_NULL, &var);
	cfg_add_var(&cfg, arr, var);

	cfg_obj(&cfg, STRV_NULL, &obj);
	cfg_add_var(&cfg, arr, obj);
	cfg_str(&cfg, STRV("str"), STRV("str"), &var);
	cfg_add_var(&cfg, obj, var);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, obj, var);

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

	cfg_var_t tbl, var;

	cfg_tbl(&cfg, STRV("tbl"), &tbl);
	cfg_str(&cfg, STRV("str"), STRV("str"), &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_int(&cfg, STRV("int"), 1, &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_arr(&cfg, STRV("arr"), &var);
	cfg_add_var(&cfg, tbl, var);
	cfg_obj(&cfg, STRV("obj"), &var);
	cfg_add_var(&cfg, tbl, var);

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
	RUN(cfg_root);
	RUN(cfg_lit);
	RUN(cfg_str);
	RUN(cfg_int);
	RUN(cfg_arr);
	RUN(cfg_obj);
	RUN(cfg_tbl);
	RUN(cfg_add_var);
	RUN(cfg_has_var);
	RUN(cfg_has_var_not_found);
	RUN(cfg_has_var_wrong_parent);
	RUN(cfg_get_val_wrong_type);
	RUN(cfg_get_lit);
	RUN(cfg_get_str);
	RUN(cfg_get_int);
	RUN(cfg_it_begin);
	RUN(cfg_it_next);
	RUN(cfg_it);
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
