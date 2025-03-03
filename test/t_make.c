#include "file/make.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(make_init_free)
{
	START;

	make_t make = {0};

	EXPECT_EQ(make_init(NULL, 0, 0, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(make_init(&make, 1, 0, 0, 0, ALLOC_STD), NULL);
	EXPECT_EQ(make_init(&make, 0, 1, 0, 0, ALLOC_STD), NULL);
	EXPECT_EQ(make_init(&make, 0, 0, 1, 0, ALLOC_STD), NULL);
	EXPECT_EQ(make_init(&make, 0, 0, 0, 1, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(make_init(&make, 1, 1, 1, 1, ALLOC_STD), &make);

	EXPECT_NE(make.acts.data, NULL);
	EXPECT_NE(make.strs.data, NULL);
	EXPECT_EQ(make.root, MAKE_END);

	make_free(&make);
	make_free(NULL);

	EXPECT_EQ(make.acts.data, NULL);
	EXPECT_EQ(make.strs.data, NULL);
	EXPECT_EQ(make.root, MAKE_END);

	END;
}

TEST(make_create_empty)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_create_empty(NULL), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_empty(&make), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_empty(&make), 0);

	make_free(&make);
	END;
}

TEST(make_create_var)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	uint id = ARR_END;

	EXPECT_EQ(make_create_var(NULL, STRV(""), -1, NULL), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_var(&make, STRV(""), -1, NULL), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_var(&make, STRV(""), -1, NULL), 0);
	EXPECT_EQ(make_create_var(&make, STRV(""), MAKE_VAR_INST, NULL), 1);
	EXPECT_EQ(make_create_var(&make, STRV(""), MAKE_VAR_APP, &id), 2);

	EXPECT_EQ(id, 2);

	make_free(&make);

	END;
}

TEST(make_create_var_oom)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 1, 1, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_var(&make, STRV(""), -1, NULL), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create_var_ext)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	uint id = ARR_END;

	EXPECT_EQ(make_create_var_ext(NULL, STRV(""), NULL), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_var_ext(&make, STRV(""), NULL), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_var_ext(&make, STRV(""), NULL), 0);
	EXPECT_EQ(make_create_var_ext(&make, STRV(""), NULL), 1);
	EXPECT_EQ(make_create_var_ext(&make, STRV(""), &id), 2);

	EXPECT_EQ(id, 2);

	make_free(&make);

	END;
}

TEST(make_create_rule)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_create_rule(NULL, MRULE(MVAR(MAKE_END)), 0), MAKE_END);
	make_str_data_t str = (make_str_data_t){.type = -1};
	EXPECT_EQ(make_create_rule(&make, MRULE(str), 0), 0);
	EXPECT_EQ(make_create_rule(&make, MRULE(MVAR(MAKE_END)), 0), 1);
	mem_oom(1);
	EXPECT_EQ(make_create_rule(&make, MRULE(MSTR(STRH(""))), 0), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_rule(&make, MRULE(MSTR(STRH(""))), 0), 2);

	make_free(&make);

	END;
}

TEST(make_create_phony)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_create_phony(NULL), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_phony(&make), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_phony(&make), 0);

	make_free(&make);

	END;
}

TEST(make_create_cmd)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_create_cmd(NULL, MCMD(STR(""))), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_cmd(&make, MCMD(STR(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_cmd(&make, MCMD(STRH(""))), 0);

	make_free(&make);

	END;
}

TEST(make_create_if)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_create_if(NULL, MVAR(MAKE_END), MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_create_if(&make, MVAR(MAKE_END), MVAR(MAKE_END)), 0);
	mem_oom(1);
	EXPECT_EQ(make_create_if(&make, MSTR(STR("")), MSTR(STR(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_if(&make, MSTR(STRH("")), MSTR(STRH(""))), 1);

	make_free(&make);

	END;
}

TEST(make_create_if_oom_l)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 1, 1, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_if(&make, MSTR(STR("")), MSTR(STR(""))), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create_if_oom_r)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_if(&make, MSTR(STR("")), MSTR(STR(""))), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create)
{
	SSTART;
	RUN(make_create_empty);
	RUN(make_create_var);
	RUN(make_create_var_oom);
	RUN(make_create_var_ext);
	RUN(make_create_rule);
	RUN(make_create_phony);
	RUN(make_create_cmd);
	RUN(make_create_if);
	RUN(make_create_if_oom_l);
	RUN(make_create_if_oom_r);
	SEND;
}

TEST(make_add_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	EXPECT_EQ(make_add_act(NULL, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_add_act(&make, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_add_act(&make, make_create_empty(&make)), 0);
	EXPECT_EQ(make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH(""))), 0)), 1);

	make_free(&make);

	END;
}

TEST(make_var_add_val)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	const make_empty_t empty = make_create_empty(&make);
	const make_rule_t var	 = make_create_var(&make, STRV(""), MAKE_VAR_INST, NULL);

	EXPECT_EQ(make_var_add_val(NULL, MAKE_END, MVAR(MAKE_END)), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_var_add_val(&make, MAKE_END, MVAR(MAKE_END)), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_var_add_val(&make, empty, MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_var_add_val(&make, make_create_var_ext(&make, STRV(""), NULL), MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_var_add_val(&make, var, MVAR(MAKE_END)), 1);
	mem_oom(1);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRH(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRH(""))), 1);

	make_free(&make);

	END;
}

TEST(make_rule_add_depend)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	const make_empty_t empty = make_create_empty(&make);
	const make_rule_t rule	 = make_create_rule(&make, MRULE(MSTR(STRH(""))), 0);

	EXPECT_EQ(make_rule_add_depend(NULL, MAKE_END, MRULE(MVAR(MAKE_END))), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_rule_add_depend(&make, MAKE_END, MRULE(MVAR(MAKE_END))), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_rule_add_depend(&make, empty, MRULE(MVAR(MAKE_END))), MAKE_END);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MVAR(MAKE_END))), 0);
	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("")))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("")))), 1);

	make_free(&make);

	END;
}

TEST(make_rule_add_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	const make_empty_t empty = make_create_empty(&make);
	const make_rule_t rule	 = make_create_rule(&make, MRULE(MSTR(STRH(""))), 0);

	EXPECT_EQ(make_rule_add_act(NULL, MAKE_END, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_rule_add_act(&make, MAKE_END, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_rule_add_act(&make, empty, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_rule_add_act(&make, rule, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_rule_add_act(&make, rule, empty), 0);

	make_free(&make);

	END;
}

TEST(make_if_add_true_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	const make_empty_t empty = make_create_empty(&make);
	const make_if_t mif	 = make_create_if(&make, MSTR(STRH("")), MSTR(STRH("")));

	EXPECT_EQ(make_if_add_true_act(NULL, MAKE_END, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_if_add_true_act(&make, MAKE_END, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_if_add_true_act(&make, empty, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_if_add_true_act(&make, mif, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_if_add_true_act(&make, mif, empty), 0);

	make_free(&make);

	END;
}

TEST(make_if_add_false_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	const make_empty_t empty = make_create_empty(&make);
	const make_if_t mif	 = make_create_if(&make, MSTR(STRH("")), MSTR(STRH("")));

	EXPECT_EQ(make_if_add_false_act(NULL, MAKE_END, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_if_add_false_act(&make, MAKE_END, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_if_add_false_act(&make, empty, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_if_add_false_act(&make, mif, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_if_add_false_act(&make, mif, empty), 0);

	make_free(&make);

	END;
}

TEST(make_add)
{
	SSTART;
	RUN(make_add_act);
	RUN(make_var_add_val);
	RUN(make_rule_add_depend);
	RUN(make_rule_add_act);
	RUN(make_if_add_true_act);
	RUN(make_if_add_false_act);
	SEND;
}

TEST(make_rule_get_target)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_var_t var = make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL);

	make_add_act(&make, make_create_rule(&make, MRULE(MVAR(var)), 0));
	make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("a"))), 0));
	make_add_act(&make, make_create_rule(&make, MRULEACT(MSTR(STRH("a")), STRH("/action")), 0));

	EXPECT_EQ(make_rule_get_target(NULL, MRULE(MSTR(STR("")))), MAKE_END);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MSTR(STR("")))), MAKE_END);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MSTR(STR("a")))), 3);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MVAR(var))), 1);

	make_free(&make);

	END;
}

TEST(make_vars_init_free)
{
	START;

	make_t make	 = {0};
	make_vars_t vars = {0};

	make_vars_init(NULL, NULL, ALLOC_STD);
	make_vars_init(&make, NULL, ALLOC_STD);
	log_set_quiet(0, 1);
	make_vars_init(&make, &vars, ALLOC_STD);
	log_set_quiet(0, 0);

	make.vars.off.cnt = 1;

	mem_oom(1);
	make_vars_init(&make, &vars, ALLOC_STD);
	mem_oom(0);

	make_vars_free(NULL);
	make_vars_free(&vars);

	END;
}

TEST(make_ext_set_val)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	uint ext = ARR_END;
	make_create_var_ext(&make, STRV("EXT"), &ext);

	EXPECT_EQ(make_ext_set_val(NULL, -1, MVAR(MAKE_END)), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_ext_set_val(&make, -1, MVAR(MAKE_END)), MAKE_END);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(make_ext_set_val(&make, ext, MVAR(MAKE_END)), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_ext_set_val(&make, ext, MVAR(MAKE_END)), 0);
	EXPECT_EQ(make_ext_set_val(&make, ext, MSTR(STR(""))), 0);

	make_free(&make);

	END;
}

TEST(make_vars_eval)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_vars_t vars = {0};

	EXPECT_EQ(make_vars_eval(NULL, NULL), 1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	make_free(&make);

	END;
}

TEST(make_vars_replace)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("$(ABC")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("$(ABC)")));
	make_var_add_val(&make,
			 make_add_act(&make, make_create_var(&make, STRV("A"), MAKE_VAR_INST, NULL)),
			 MSTR(STR("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")));
	make_var_add_val(&make,
			 make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)),
			 MSTR(STR("$(A) $(A) $(A) $(A) $(A) $(A) $(A) $(A)")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	log_set_quiet(0, 0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_eval_oom)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make,
			 make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)),
			 MSTR(STR("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")));
	make_var_add_val(&make,
			 make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_APP, &var)),
			 MSTR(STR("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	mem_oom(1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	mem_oom(0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}


TEST(make_vars_eval_var_type)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make,
			 make_add_act(&make, make_create_var(&make, STRV("VAR"), -1, &var)),
			 MSTR(STR("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 1);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_get_expanded)
{
	START;

	make_vars_t vars = {0};

	EXPECT_EQ(make_vars_get_expanded(NULL, 0).data, NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_get_expanded(&vars, 0).data, NULL);
	log_set_quiet(0, 0);

	END;
}

TEST(make_vars_get_resolved)
{
	START;

	make_t make	 = {0};
	make_vars_t vars = {0};
	EXPECT_EQ(make_vars_get_resolved(NULL, NULL, 0).data, NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_get_resolved(&make, &vars, 0).data, NULL);
	log_set_quiet(0, 0);

	END;
}

TEST(make_print)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[8] = {0};
	EXPECT_EQ(make_print(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	make_add_act(&make, make_create_empty(&make));

	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	make_add_act(&make, make_create_var(&make, STRV(""), -1, NULL));
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	make_free(&make);

	END;
}

TEST(make_dbg)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[8] = {0};
	EXPECT_EQ(make_dbg(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	make_add_act(&make, make_create_empty(&make));

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);

	make_free(&make);

	END;
}

TEST(make_eval_print_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_add_act(&make, make_create_empty(&make));

	make_vars_t vars = {0};
	log_set_quiet(0, 1);
	make_vars_init(&make, &vars, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	char buf[8] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
	EXPECT_STR(buf, "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(var_exp.data, "", var_exp.len);

	char buf[128] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);
	EXPECT_STR(buf, "VAR :=\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 37);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STRH("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(var_exp.data, "VAL", var_exp.len);

	char buf[16] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 11);
	EXPECT_STR(buf, "VAR := VAL\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst2)
{
	START;

	make_t make = {0};
	make_init(&make, 3, 3, 1, 1, ALLOC_STD);

	uint id	       = ARR_END;
	make_var_t var = make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &id));
	make_var_add_val(&make, var, MSTR(STRH("VAL1")));
	make_var_add_val(&make, var, MSTR(STRH("VAL2")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(var_exp.data, "VAL1 VAL2", var_exp.len);

	char buf[32] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 17);
	EXPECT_STR(buf, "VAR := VAL1 VAL2\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_ref)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_REF, &var)), MSTR(STRH("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(var_exp.data, "VAL", var_exp.len);

	char buf[16] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 10);
	EXPECT_STR(buf, "VAR = VAL\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_app)
{
	START;

	make_t make = {0};
	make_init(&make, 2, 2, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STRH("VAL1")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_APP, &var)), MSTR(STRH("VAL2")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(var_exp.data, "VAL1 VAL2", var_exp.len);

	char buf[256] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 24);
	EXPECT_STR(buf,
		   "VAR := VAL1\n"
		   "VAR += VAL2\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 100);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_ext_inst)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var_ext(&make, STRV("VAR"), &var)), MSTR(STR("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(var_exp.data, "VAL", var_exp.len);

	char buf[8] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_STR(buf, "");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_ext)
{
	START;

	make_t make = {0};
	make_init(&make, 8, 8, 1, 1, ALLOC_STD);

	uint ext_id1	= ARR_END;
	uint ext_id2	= ARR_END;
	uint ext_id3	= ARR_END;
	uint ext_id4	= ARR_END;
	make_var_t ext1 = make_add_act(&make, make_create_var_ext(&make, STRV("EXT1"), &ext_id1));
	make_var_t ext2 = make_add_act(&make, make_create_var_ext(&make, STRV("EXT2"), &ext_id2));
	make_var_t ext3 = make_add_act(&make, make_create_var_ext(&make, STRV("EXT3"), &ext_id3));
	make_var_t ext4 = make_add_act(&make, make_create_var_ext(&make, STRV("EXT4"), &ext_id4));

	uint var_id	   = ARR_END;
	make_var_t var	   = make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_APP, &var_id));
	make_var_t var_app = make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_APP, &var_id));

	uint out_id    = ARR_END;
	make_var_t out = make_add_act(&make, make_create_var(&make, STRV("OUT"), MAKE_VAR_INST, &out_id));

	make_var_add_val(&make, var, MVAR(ext1));
	make_var_add_val(&make, var, MVAR(ext2));
	make_var_add_val(&make, var_app, MVAR(ext3));
	make_var_add_val(&make, var_app, MVAR(ext4));

	make_var_add_val(&make, out, MVAR(var));

	make_ext_set_val(&make, ext_id1, MSTR(STR("VAL1")));
	make_ext_set_val(&make, ext_id2, MSTR(STR("VAL2")));
	make_ext_set_val(&make, ext_id3, MSTR(STR("VAL3")));
	make_ext_set_val(&make, ext_id4, MSTR(STR("VAL4")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	strv_t out_exp = make_vars_get_expanded(&vars, out_id);
	EXPECT_STRN(out_exp.data, "$(VAR)", out_exp.len);
	strv_t out_res = make_vars_get_resolved(&make, &vars, out_id);
	EXPECT_STRN(out_res.data, "VAL1 VAL2 VAL3 VAL4", out_res.len);

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 60);
	EXPECT_STR(buf,
		   "VAR += $(EXT1) $(EXT2)\n"
		   "VAR += $(EXT3) $(EXT4)\n"
		   "OUT := $(VAR)\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_if_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_add_act(&make, make_create_if(&make, MSTR(str_null()), MSTR(str_null())));

	make_vars_t vars = {0};
	log_set_quiet(0, 1);
	make_vars_init(&make, &vars, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 15);
	EXPECT_STR(buf,
		   "ifeq (,)\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 24);
	EXPECT_STR(buf,
		   "IF\n"
		   "    L: ''\n"
		   "    R: ''\n"
		   "\n")

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_if_lr)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_add_act(&make, make_create_if(&make, MSTR(STRH("L")), MSTR(STRH("R"))));

	make_vars_t vars = {0};
	log_set_quiet(0, 1);
	make_vars_init(&make, &vars, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 17);
	EXPECT_STR(buf,
		   "ifeq (L,R)\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 26);
	EXPECT_STR(buf,
		   "IF\n"
		   "    L: 'L'\n"
		   "    R: 'R'\n"
		   "\n")

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_if_true)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 1, ALLOC_STD);

	uint cond_id		= ARR_END;
	const make_var_t cond	= make_add_act(&make, make_create_var_ext(&make, STRV("COND"), &cond_id));
	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(cond), MSTR(STRH("A"))));
	uint var_id		= ARR_END;
	make_if_add_true_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id), MSTR(STRH("VAL"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_ext_set_val(&make, cond_id, MSTR(STR("A")));
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp_a = make_vars_get_expanded(&vars, var_id);
	EXPECT_STRN(var_exp_a.data, "VAL", var_exp_a.len);

	make_ext_set_val(&make, cond_id, MSTR(STR("B")));
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp_b = make_vars_get_expanded(&vars, var_id);
	(void)var_exp_b;
	EXPECT_STRN(var_exp_b.data, "", var_exp_b.len);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 34);
	EXPECT_STR(buf,
		   "ifeq ($(COND),A)\n"
		   "VAR := VAL\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 134);
	EXPECT_STR(buf,
		   "VAR\n"
		   "    NAME    : COND (ext)\n"
		   "    VALUES  :\n"
		   "        B\n"
		   "IF\n"
		   "    L: '$(COND)'\n"
		   "    R: 'A'\n"
		   "\n"
		   "VAR\n"
		   "    NAME    : VAR \n"
		   "    VALUES  :\n"
		   "        VAL\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_if_false)
{
	START;

	make_t make = {0};
	make_init(&make, 6, 6, 1, 1, ALLOC_STD);

	uint cond_id		= ARR_END;
	const make_var_t cond	= make_add_act(&make, make_create_var_ext(&make, STRV("COND"), &cond_id));
	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(cond), MSTR(STRH("A"))));
	uint var_id		= ARR_END;
	make_if_add_true_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id), MSTR(STRH("VAL1"))));
	make_if_add_false_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id), MSTR(STRH("VAL2"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_ext_set_val(&make, cond_id, MSTR(STR("A")));
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp_a = make_vars_get_expanded(&vars, var_id);
	EXPECT_STRN(var_exp_a.data, "VAL1", var_exp_a.len);

	make_ext_set_val(&make, cond_id, MSTR(STR("B")));
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strv_t var_exp_b = make_vars_get_expanded(&vars, var_id);
	EXPECT_STRN(var_exp_b.data, "VAL2", var_exp_b.len);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 52);
	EXPECT_STR(buf,
		   "ifeq ($(COND),A)\n"
		   "VAR := VAL1\n"
		   "else\n"
		   "VAR := VAL2\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 185);
	EXPECT_STR(buf,
		   "VAR\n"
		   "    NAME    : COND (ext)\n"
		   "    VALUES  :\n"
		   "        B\n"
		   "IF\n"
		   "    L: '$(COND)'\n"
		   "    R: 'A'\n"
		   "\n"
		   "VAR\n"
		   "    NAME    : VAR \n"
		   "    VALUES  :\n"
		   "        VAL1\n"
		   "VAR\n"
		   "    NAME    : VAR \n"
		   "    VALUES  :\n"
		   "        VAL2\n")

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_not_found)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MVAR(MAKE_END));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	char buf[256] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 8);
	log_set_quiet(0, 0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_ext_override)
{
	START;

	make_t make = {0};
	make_init(&make, 3, 3, 1, 1, ALLOC_STD);

	uint ext = ARR_END;
	make_add_act(&make, make_create_var_ext(&make, STRV("EXT"), &ext));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("EXT"), MAKE_VAR_INST, &ext)), MSTR(STR("VAL")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("EXT"), MAKE_VAR_APP, &ext)), MSTR(STR("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	{
		make_vars_eval(&make, &vars);
		strv_t exp = make_vars_get_expanded(&vars, ext);
		EXPECT_EQ(exp.len, 7);
		EXPECT_STRN(exp.data, "VAL VAL", exp.len);
		strv_t res = make_vars_get_resolved(&make, &vars, ext);
		EXPECT_EQ(res.len, 7);
		EXPECT_STRN(res.data, "VAL VAL", res.len);
	}
	{
		make_ext_set_val(&make, ext, MSTR(STR("EXT")));
		make_vars_eval(&make, &vars);
		strv_t exp = make_vars_get_expanded(&vars, ext);
		EXPECT_EQ(exp.len, 3);
		EXPECT_STRN(exp.data, "EXT", exp.len);
		strv_t res = make_vars_get_expanded(&vars, ext);
		EXPECT_EQ(res.len, 3);
		EXPECT_STRN(res.data, "EXT", res.len)
	}

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_inst_app)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("A")));
	uint t = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_INST, &t)), MVAR(var));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_APP, &t)), MVAR(var));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("B")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);
	strv_t exp = make_vars_get_expanded(&vars, t);
	EXPECT_EQ(exp.len, 13);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR)", exp.len);
	strv_t res = make_vars_get_resolved(&make, &vars, t);
	EXPECT_EQ(res.len, 3);
	EXPECT_STRN(res.data, "A A", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_ref_app)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("A")));
	uint t = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_REF, &t)), MVAR(var));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_APP, &t)), MVAR(var));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("B")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);
	strv_t exp = make_vars_get_expanded(&vars, t);
	EXPECT_EQ(exp.len, 13);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR)", exp.len);
	strv_t res = make_vars_get_resolved(&make, &vars, t);
	EXPECT_EQ(res.len, 3);
	EXPECT_STRN(res.data, "B B", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_inst_if)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("A")));
	uint t = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_INST, &t)), MVAR(var));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("B")));

	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(t), MSTR(STRH("A"))));
	uint r			= ARR_END;
	make_if_add_true_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("R"), MAKE_VAR_INST, &r), MSTR(STRH("C"))));
	make_if_add_false_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("R"), MAKE_VAR_INST, &r), MSTR(STRH("D"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);
	strv_t exp = make_vars_get_expanded(&vars, r);
	EXPECT_EQ(exp.len, 1);
	EXPECT_STRN(exp.data, "C", exp.len);
	strv_t res = make_vars_get_resolved(&make, &vars, r);
	EXPECT_EQ(res.len, 1);
	EXPECT_STRN(res.data, "C", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_ref_if)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("A")));
	uint t = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_REF, &t)), MVAR(var));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("B")));

	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(t), MSTR(STRH("A"))));
	uint r			= ARR_END;
	make_if_add_true_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("R"), MAKE_VAR_INST, &r), MSTR(STRH("C"))));
	make_if_add_false_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("R"), MAKE_VAR_INST, &r), MSTR(STRH("D"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);
	strv_t exp = make_vars_get_expanded(&vars, r);
	EXPECT_EQ(exp.len, 1);
	EXPECT_STRN(exp.data, "D", exp.len);
	strv_t res = make_vars_get_resolved(&make, &vars, r);
	EXPECT_EQ(res.len, 1);
	EXPECT_STRN(res.data, "D", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_print)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STR("A")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);

	char buf[256] = {0};
	make_vars_print(&make, &vars, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "VAR              A                                                                A\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_print_rule_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);
	EXPECT_STR(buf,
		   "rule:\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 35);

	make_free(&make);

	END;
}

TEST(make_print_rule_empty_var)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_add_act(&make, make_create_rule(&make, MRULE(MVAR(MAKE_END)), 1));

	char buf[64] = {0};
	log_set_quiet(0, 1);
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 3);
	EXPECT_STR(buf,
		   ":\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 31);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_print_rule_empty_action)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_add_act(&make, make_create_rule(&make, MRULEACT(MSTR(STRH("rule")), STRH("/action")), 1));

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 14);
	EXPECT_STR(buf,
		   "rule/action:\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 42);

	make_free(&make);

	END;
}

TEST(make_print_rule_depend)
{
	START;

	make_t make = {0};
	make_init(&make, 2, 2, 1, 1, ALLOC_STD);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("depend"))));

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 14);
	EXPECT_STR(buf,
		   "rule: depend\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 50);

	make_free(&make);

	END;
}

TEST(make_print_rule_depends)
{
	START;

	make_t make = {0};
	make_init(&make, 3, 3, 1, 1, ALLOC_STD);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("depend1"))));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("depend2"))));

	char buf[128] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 23);
	EXPECT_STR(buf,
		   "rule: depend1 depend2\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 67);

	make_free(&make);

	END;
}

TEST(make_print_rule_acts)
{
	START;

	make_t make = {0};
	make_init(&make, 6, 6, 1, 1, ALLOC_STD);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRH("cmd1"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRH("cmd2"))));

	make_rule_t if_rule = make_rule_add_act(&make, rule, make_create_if(&make, MSTR(STRH("L")), MSTR(STRH("R"))));
	make_if_add_true_act(&make, if_rule, make_create_cmd(&make, MCMD(STRH("cmd3"))));
	make_if_add_false_act(&make, if_rule, make_create_cmd(&make, MCMD(STRH("cmd4"))));

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 53);
	EXPECT_STR(buf,
		   "rule:\n"
		   "\tcmd1\n"
		   "\tcmd2\n"
		   "ifeq (L,R)\n"
		   "\tcmd3\n"
		   "else\n"
		   "\tcmd4\n"
		   "endif\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 229);
	EXPECT_STR(buf,
		   "RULE\n"
		   "    TARGET: rule\n"
		   "    DEPENDS:\n"
		   "CMD\n"
		   "    ARG1: cmd1\n"
		   "    ARG2: \n"
		   "    TYPE: 0\n"
		   "CMD\n"
		   "    ARG1: cmd2\n"
		   "    ARG2: \n"
		   "    TYPE: 0\n"
		   "IF\n"
		   "    L: 'L'\n"
		   "    R: 'R'\n"
		   "\n"
		   "CMD\n"
		   "    ARG1: cmd3\n"
		   "    ARG2: \n"
		   "    TYPE: 0\n"
		   "CMD\n"
		   "    ARG1: cmd4\n"
		   "    ARG2: \n"
		   "    TYPE: 0\n")
	make_free(&make);

	END;
}

TEST(make_print_cmd)
{
	START;

	make_t make = {0};
	make_init(&make, 5, 5, 1, 1, ALLOC_STD);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRH("cmd"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDCHILD(STRH("dir"), str_null())));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDCHILD(STRH("dir"), STRH("action"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDERR(STRH("msg"))));

	char buf[256] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 67);
	EXPECT_STR(buf,
		   "rule:\n"
		   "\tcmd\n"
		   "\t@$(MAKE) -C dir\n"
		   "\t@$(MAKE) -C dir action\n"
		   "\t$(error msg)\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 205);

	make_free(&make);

	END;
}

TEST(make_eval_print)
{
	SSTART;
	RUN(make_ext_set_val);
	RUN(make_vars_init_free);
	RUN(make_vars_eval);
	RUN(make_vars_replace);
	RUN(make_vars_eval_oom);
	RUN(make_vars_eval_var_type);
	RUN(make_vars_get_expanded);
	RUN(make_vars_get_resolved);
	RUN(make_print);
	RUN(make_dbg);
	RUN(make_eval_print_empty);
	RUN(make_eval_print_var_inst_empty);
	RUN(make_eval_print_var_inst);
	RUN(make_eval_print_var_inst2);
	RUN(make_eval_print_var_ref);
	RUN(make_eval_print_var_app);
	RUN(make_eval_print_var_ext_inst);
	RUN(make_eval_print_var_ext);
	RUN(make_eval_print_if_empty);
	RUN(make_eval_print_if_lr);
	RUN(make_eval_print_var_if_true);
	RUN(make_eval_print_var_if_false);
	RUN(make_eval_print_var_not_found);
	RUN(make_eval_ext_override);
	RUN(make_eval_inst_app);
	RUN(make_eval_ref_app);
	RUN(make_eval_inst_if);
	RUN(make_eval_ref_if);
	RUN(make_vars_print);
	RUN(make_print_rule_empty);
	RUN(make_print_rule_empty_var);
	RUN(make_print_rule_empty_action);
	RUN(make_print_rule_depend);
	RUN(make_print_rule_depends);
	RUN(make_print_rule_acts);
	RUN(make_print_cmd);
	SEND;
}

STEST(make)
{
	SSTART;

	RUN(make_init_free);
	RUN(make_create);
	RUN(make_add);
	RUN(make_rule_get_target);
	RUN(make_eval_print);

	SEND;
}
