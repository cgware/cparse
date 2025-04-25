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

	EXPECT_NE(make.arrs.data, NULL);
	EXPECT_NE(make.acts.data, NULL);
	EXPECT_EQ(make.root, MAKE_END);

	make_free(&make);
	make_free(NULL);

	EXPECT_EQ(make.arrs.data, NULL);
	EXPECT_EQ(make.acts.data, NULL);
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
	make_create_str_t str = (make_create_str_t){.type = -1};
	EXPECT_EQ(make_create_rule(&make, MRULE(str), 0), 0);
	EXPECT_EQ(make_create_rule(&make, MRULE(MVAR(MAKE_END)), 0), 1);
	mem_oom(1);
	EXPECT_EQ(make_create_rule(&make, MRULE(MSTR(STRV(""))), 0), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_rule(&make, MRULE(MSTR(STRV(""))), 0), 2);

	make_free(&make);

	END;
}

TEST(make_create_rule_oom_target)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_rule(&make, MRULE(MSTR(STRV(""))), 0), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create_rule_oom_action)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_rule(&make, MRULEACT(MSTR(STRV("")), STRV("")), 0), MAKE_END);
	mem_oom(0);

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

	EXPECT_EQ(make_create_cmd(NULL, MCMD(STRV(""))), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_cmd(&make, MCMD(STRV(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_cmd(&make, MCMD(STRV(""))), 0);

	make_free(&make);

	END;
}

TEST(make_create_cmd_oom_arg1)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_cmd(&make, MCMD(STRV(""))), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create_cmd_oom_arg2)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_cmd(&make, MCMDCHILD(STRV(""), STRV(""))), MAKE_END);
	mem_oom(0);

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
	EXPECT_EQ(make_create_if(&make, MSTR(STRV("")), MSTR(STRV(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_if(&make, MSTR(STRV("")), MSTR(STRV(""))), 1);

	make_free(&make);

	END;
}

TEST(make_create_if_oom_l)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_if(&make, MSTR(STRV("")), MSTR(STRV(""))), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create_if_oom_r)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_if(&make, MSTR(STRV("")), MSTR(STRV(""))), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create_def)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_create_def(NULL, STRV_NULL), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_def(&make, STRV("")), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_def(&make, STRV("")), 0);

	make_free(&make);

	END;
}

TEST(make_create_def_oom_name)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_def(&make, STRV("")), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create_eval_def)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_def_t def = make_add_act(&make, make_create_def(&make, STRV("def")));

	EXPECT_EQ(make_create_eval_def(NULL, MAKE_END), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_eval_def(&make, def), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_eval_def(&make, def), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_create_eval_def(&make, -1), MAKE_END);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_create_inc)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_create_inc(NULL, STRV_NULL), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_inc(&make, STRV("")), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_inc(&make, STRV("")), 0);

	make_free(&make);

	END;
}

TEST(make_create_inc_oom_name)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_create_inc(&make, STRV("")), MAKE_END);
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
	RUN(make_create_rule_oom_target);
	RUN(make_create_rule_oom_action);
	RUN(make_create_phony);
	RUN(make_create_cmd);
	RUN(make_create_cmd_oom_arg1);
	RUN(make_create_cmd_oom_arg2);
	RUN(make_create_if);
	RUN(make_create_if_oom_l);
	RUN(make_create_if_oom_r);
	RUN(make_create_def);
	RUN(make_create_def_oom_name);
	RUN(make_create_eval_def);
	RUN(make_create_inc);
	RUN(make_create_inc_oom_name);
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
	EXPECT_EQ(make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRV(""))), 0)), 1);

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
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRV(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRV(""))), 1);

	make_free(&make);

	END;
}

TEST(make_var_add_val_oom_val)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 1, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	const make_rule_t var = make_create_var(&make, STRV(""), MAKE_VAR_INST, NULL);

	mem_oom(1);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRV(""))), MAKE_END);
	mem_oom(0);

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
	const make_rule_t rule	 = make_create_rule(&make, MRULE(MSTR(STRV(""))), 0);

	EXPECT_EQ(make_rule_add_depend(NULL, MAKE_END, MRULE(MVAR(MAKE_END))), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_rule_add_depend(&make, MAKE_END, MRULE(MVAR(MAKE_END))), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_rule_add_depend(&make, empty, MRULE(MVAR(MAKE_END))), MAKE_END);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MVAR(MAKE_END))), 0);
	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("")))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("")))), 1);

	make_free(&make);

	END;
}

TEST(make_rule_add_depend_oom_action)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 2, ALLOC_STD);
	log_set_quiet(0, 0);

	const make_rule_t rule = make_create_rule(&make, MRULE(MSTR(STRV(""))), 0);

	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULEACT(MSTR(STRV("")), STRV(""))), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_rule_add_depend_oom_depends)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 2, ALLOC_STD);
	log_set_quiet(0, 0);

	const make_rule_t rule = make_create_rule(&make, MRULE(MSTR(STRV(""))), 0);

	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("")))), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_rule_add_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	const make_empty_t empty = make_create_empty(&make);
	const make_rule_t rule	 = make_create_rule(&make, MRULE(MSTR(STRV(""))), 0);

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
	const make_if_t mif	 = make_create_if(&make, MSTR(STRV("")), MSTR(STRV("")));

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
	const make_if_t mif	 = make_create_if(&make, MSTR(STRV("")), MSTR(STRV("")));

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

TEST(make_def_add_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	const make_empty_t empty = make_create_empty(&make);
	const make_def_t def	 = make_create_def(&make, STRV(""));

	EXPECT_EQ(make_def_add_act(NULL, MAKE_END, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_def_add_act(&make, MAKE_END, MAKE_END), MAKE_END);
	EXPECT_EQ(make_def_add_act(&make, empty, MAKE_END), MAKE_END);
	EXPECT_EQ(make_def_add_act(&make, def, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_def_add_act(&make, def, empty), 0);

	make_free(&make);

	END;
}

TEST(make_eval_def_add_arg)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	const make_def_t def	   = make_create_def(&make, STRV("def"));
	const make_eval_def_t eval = make_create_eval_def(&make, def);

	EXPECT_EQ(make_eval_def_add_arg(NULL, MAKE_END, MSTR(STRV_NULL)), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval_def_add_arg(&make, MAKE_END, MSTR(STRV_NULL)), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_eval_def_add_arg(&make, eval, MSTR(STRV(""))), 1);
	mem_oom(1);
	EXPECT_EQ(make_eval_def_add_arg(&make, eval, MSTR(STRV(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_eval_def_add_arg(&make, eval, MSTR(STRV(""))), 1);

	make_free(&make);

	END;
}

TEST(make_eval_def_add_arg_oom_arg)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 1, 2, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	const make_def_t def	   = make_create_def(&make, STRV("def"));
	const make_eval_def_t eval = make_create_eval_def(&make, def);

	mem_oom(1);
	EXPECT_EQ(make_eval_def_add_arg(&make, eval, MSTR(STRV(""))), MAKE_END);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_inc_add_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	const make_empty_t empty = make_create_empty(&make);
	const make_def_t def	 = make_create_inc(&make, STRV(""));

	EXPECT_EQ(make_inc_add_act(NULL, MAKE_END, MAKE_END), MAKE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_inc_add_act(&make, MAKE_END, MAKE_END), MAKE_END);
	EXPECT_EQ(make_inc_add_act(&make, empty, MAKE_END), MAKE_END);
	EXPECT_EQ(make_inc_add_act(&make, def, MAKE_END), MAKE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_inc_add_act(&make, def, empty), 0);

	make_free(&make);

	END;
}

TEST(make_add)
{
	SSTART;
	RUN(make_add_act);
	RUN(make_var_add_val);
	RUN(make_var_add_val_oom_val);
	RUN(make_rule_add_depend);
	RUN(make_rule_add_depend_oom_action);
	RUN(make_rule_add_depend_oom_depends);
	RUN(make_rule_add_act);
	RUN(make_if_add_true_act);
	RUN(make_if_add_false_act);
	RUN(make_def_add_act);
	RUN(make_eval_def_add_arg);
	RUN(make_eval_def_add_arg_oom_arg);
	RUN(make_inc_add_act);
	SEND;
}

TEST(make_rule_get_target)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 8, ALLOC_STD);

	make_var_t var = make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL);

	make_add_act(&make, make_create_rule(&make, MRULE(MVAR(var)), 0));
	make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRV("a"))), 0));
	make_add_act(&make, make_create_rule(&make, MRULEACT(MSTR(STRV("a")), STRV("/action")), 0));

	EXPECT_EQ(make_rule_get_target(NULL, MRULE(MSTR(STRV("")))), MAKE_END);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MSTR(STRV("")))), MAKE_END);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MSTR(STRV("a")))), 3);
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

	make.strs.off.cnt = 1;

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
	EXPECT_EQ(make_ext_set_val(&make, ext, MSTR(STRV(""))), 0);

	make_free(&make);

	END;
}

TEST(make_ext_set_val_oom_val)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 1, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	uint ext = ARR_END;
	make_create_var_ext(&make, STRV("EXT"), &ext);

	mem_oom(1);
	EXPECT_EQ(make_ext_set_val(&make, ext, MSTR(STRV(""))), MAKE_END);
	mem_oom(0);

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
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STRV("$(ABC")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STRV("$(ABC)")));
	make_var_add_val(&make,
			 make_add_act(&make, make_create_var(&make, STRV("A"), MAKE_VAR_INST, NULL)),
			 MSTR(STRV("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")));
	make_var_add_val(&make,
			 make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)),
			 MSTR(STRV("$(A) $(A) $(A) $(A) $(A) $(A) $(A) $(A)")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	log_set_quiet(0, 0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_eval_oom_add_var)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_vars_t vars = {0};
	log_set_quiet(0, 1);
	make_vars_init(&make, &vars, ALLOC_STD);
	log_set_quiet(0, 0);

	make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL));

	mem_oom(1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	mem_oom(0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_eval_oom_var_inst)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make.strs.off.cnt = 1;

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make.strs.off.cnt = 0;

	make_var_add_val(
		&make, make_add_act(&make, make_create_var(&make, STRV("A"), MAKE_VAR_INST, NULL)), MSTR(STRV("AAAAAAAAAAAAAAAAA")));

	mem_oom(1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	mem_oom(0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_eval_oom_var_append)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make.strs.off.cnt = 1;

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make.strs.off.cnt = 0;

	uint var = ARR_END;
	make_var_add_val(
		&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_APP, &var)), MSTR(STRV("AAAAAAAAAAAAAAAAA")));

	mem_oom(1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	mem_oom(0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_eval_oom_if)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_vars_t vars = {0};
	log_set_quiet(0, 1);
	make_vars_init(&make, &vars, ALLOC_STD);
	log_set_quiet(0, 0);

	make_add_act(&make, make_create_if(&make, MSTR(STRV("L")), MSTR(STRV("R"))));

	mem_oom(1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	mem_oom(0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_eval_invalid_eval_def)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_vars_t vars = {0};
	log_set_quiet(0, 1);
	make_vars_init(&make, &vars, ALLOC_STD);
	make_create_eval_def(&make, MAKE_END);
	log_set_quiet(0, 0);

	make_add_act(&make, 0);

	char buf[64] = {0};

	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	log_set_quiet(0, 0);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_eval_var_type)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), -1, NULL)), MSTR(STRV("A")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 1);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_eval_ext_not_set)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_add_act(&make, make_create_var_ext(&make, STRV("EXT"), NULL));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	uint id;
	strv_t exp, res;

	strbuf_find(&vars.names, STRV("EXT"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "", res.len);

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

	make_vars_t vars = {0};
	EXPECT_EQ(make_vars_get_resolved(NULL, 0).data, NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_get_resolved(&vars, 0).data, NULL);
	log_set_quiet(0, 0);

	END;
}

TEST(make_inc_print)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[8] = {0};
	EXPECT_EQ(make_inc_print(NULL, MAKE_END, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_inc_print(&make, MAKE_END, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	log_set_quiet(0, 0);

	make_inc_t inc	   = make_add_act(&make, make_create_inc(&make, STRV("")));
	make_empty_t empty = make_create_empty(&make);
	make_inc_add_act(&make, inc, empty);

	EXPECT_EQ(make_inc_print(&make, empty, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(make_inc_print(&make, inc, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	make_free(&make);

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

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 6);

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

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 6);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint var;
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &var);
	strv_t var_exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(var_exp.data, "", var_exp.len);

	char buf[128] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);
	EXPECT_STR(buf, "VAR :=\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 36);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL)), MSTR(STRV("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint var;
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &var);
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
	make_init(&make, 3, 3, 1, 4, ALLOC_STD);

	make_var_t var = make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL));
	make_var_add_val(&make, var, MSTR(STRV("VAL1")));
	make_var_add_val(&make, var, MSTR(STRV("VAL2")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint id;
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &id);
	strv_t var_exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(var_exp.data, "VAL1 VAL2", var_exp.len);

	char buf[32] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 17);
	EXPECT_STR(buf, "VAR := VAL1 VAL2\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst_out)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("A"), MAKE_VAR_INST, NULL)), MSTR(STRV("V")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("B"), MAKE_VAR_INST, NULL)), MSTR(STRV("$$(A)")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint var;
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("B"), &var);
	strv_t var_exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(var_exp.data, "$$(A)", var_exp.len);

	strv_t var_res = make_vars_get_resolved(&vars, var);
	EXPECT_STRN(var_res.data, "$(A)", var_res.len);

	char buf[32] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 18);
	EXPECT_STR(buf,
		   "A := V\n"
		   "B := $$(A)\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_ref)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_REF, NULL)), MSTR(STRV("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint var = ARR_END;
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &var);
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
	make_init(&make, 2, 2, 1, 4, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STRV("VAL1")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_APP, &var)), MSTR(STRV("VAL2")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint id;
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &id);
	strv_t var_exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(var_exp.data, "VAL1 VAL2", var_exp.len);

	char buf[256] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 24);
	EXPECT_STR(buf,
		   "VAR := VAL1\n"
		   "VAR += VAL2\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 98);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_ext_inst)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	uint var = ARR_END;
	make_add_act(&make, make_create_var_ext(&make, STRV("VAR"), &var));

	make_ext_set_val(&make, var, MSTR(STRV("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint id;
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &id);
	strv_t var_exp = make_vars_get_expanded(&vars, id);
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
	make_init(&make, 8, 8, 1, 16, ALLOC_STD);

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

	make_var_t out = make_add_act(&make, make_create_var(&make, STRV("OUT"), MAKE_VAR_INST, NULL));

	make_var_add_val(&make, var, MVAR(ext1));
	make_var_add_val(&make, var, MVAR(ext2));
	make_var_add_val(&make, var_app, MVAR(ext3));
	make_var_add_val(&make, var_app, MVAR(ext4));

	make_var_add_val(&make, out, MVAR(var));

	make_ext_set_val(&make, ext_id1, MSTR(STRV("VAL1")));
	make_ext_set_val(&make, ext_id2, MSTR(STRV("VAL2")));
	make_ext_set_val(&make, ext_id3, MSTR(STRV("VAL3")));
	make_ext_set_val(&make, ext_id4, MSTR(STRV("VAL4")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	uint out_id = ARR_END;
	strbuf_find(&vars.names, STRV("OUT"), &out_id);

	strv_t out_exp = make_vars_get_expanded(&vars, out_id);
	EXPECT_STRN(out_exp.data, "$(VAR)", out_exp.len);
	strv_t out_res = make_vars_get_resolved(&vars, out_id);
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

	make_add_act(&make, make_create_if(&make, MSTR(STRV_NULL), MSTR(STRV_NULL)));

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

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 23);
	EXPECT_STR(buf,
		   "IF\n"
		   "    L: ''\n"
		   "    R: ''\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_if_lr)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_add_act(&make, make_create_if(&make, MSTR(STRV("L")), MSTR(STRV("R"))));

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

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 25);
	EXPECT_STR(buf,
		   "IF\n"
		   "    L: 'L'\n"
		   "    R: 'R'\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_if_true)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	uint cond_id		= ARR_END;
	const make_var_t cond	= make_add_act(&make, make_create_var_ext(&make, STRV("COND"), &cond_id));
	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(cond), MSTR(STRV("A"))));
	make_if_add_true_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL), MSTR(STRV("VAL"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint var_id;

	make_ext_set_val(&make, cond_id, MSTR(STRV("A")));
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &var_id);
	strv_t var_exp_a = make_vars_get_expanded(&vars, var_id);
	EXPECT_STRN(var_exp_a.data, "VAL", var_exp_a.len);

	make_ext_set_val(&make, cond_id, MSTR(STRV("B")));
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	EXPECT_EQ(strbuf_find(&vars.names, STRV("VAR"), &var_id), 1);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 34);
	EXPECT_STR(buf,
		   "ifeq ($(COND),A)\n"
		   "VAR := VAL\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 132);
	EXPECT_STR(buf,
		   "VAR\n"
		   "    NAME    : COND (ext)\n"
		   "    VALUES  :\n"
		   "        B\n"
		   "IF\n"
		   "    L: '$(COND)'\n"
		   "    R: 'A'\n"
		   "VAR\n"
		   "    NAME    : VAR\n"
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
	make_init(&make, 6, 6, 1, 8, ALLOC_STD);

	uint cond_id		= ARR_END;
	const make_var_t cond	= make_add_act(&make, make_create_var_ext(&make, STRV("COND"), &cond_id));
	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(cond), MSTR(STRV("A"))));
	uint var_id		= ARR_END;
	make_if_add_true_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id), MSTR(STRV("VAL1"))));
	make_if_add_false_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id), MSTR(STRV("VAL2"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint varr = ARR_END;

	make_ext_set_val(&make, cond_id, MSTR(STRV("A")));
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &varr);
	strv_t var_exp_a = make_vars_get_expanded(&vars, varr);
	EXPECT_STRN(var_exp_a.data, "VAL1", var_exp_a.len);

	make_ext_set_val(&make, cond_id, MSTR(STRV("B")));
	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	strbuf_find(&vars.names, STRV("VAR"), &varr);
	strv_t var_exp_b = make_vars_get_expanded(&vars, varr);
	EXPECT_STRN(var_exp_b.data, "VAL2", var_exp_b.len);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 52);
	EXPECT_STR(buf,
		   "ifeq ($(COND),A)\n"
		   "VAR := VAL1\n"
		   "else\n"
		   "VAR := VAL2\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 182);
	EXPECT_STR(buf,
		   "VAR\n"
		   "    NAME    : COND (ext)\n"
		   "    VALUES  :\n"
		   "        B\n"
		   "IF\n"
		   "    L: '$(COND)'\n"
		   "    R: 'A'\n"
		   "VAR\n"
		   "    NAME    : VAR\n"
		   "    VALUES  :\n"
		   "        VAL1\n"
		   "VAR\n"
		   "    NAME    : VAR\n"
		   "    VALUES  :\n"
		   "        VAL2\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_def_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_add_act(&make, make_create_def(&make, STRV_NULL));

	make_vars_t vars = {0};
	log_set_quiet(0, 1);
	make_vars_init(&make, &vars, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 14);
	EXPECT_STR(buf,
		   "define \n"
		   "endef\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 17);
	EXPECT_STR(buf,
		   "DEF\n"
		   "    NAME: ''\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_def)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	const make_def_t def = make_add_act(&make, make_create_def(&make, STRV("def")));
	make_def_add_act(&make, def, make_var_add_val(&make, make_create_var(&make, STRV("$(1)"), MAKE_VAR_INST, NULL), MSTR(STRV("VAL"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);
	EXPECT_EQ(vars.names.off.cnt, 0);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 29);
	EXPECT_STR(buf,
		   "define def\n"
		   "$(1) := VAL\n"
		   "endef\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 69);
	EXPECT_STR(buf,
		   "DEF\n"
		   "    NAME: 'def'\n"
		   "VAR\n"
		   "    NAME    : $(1)\n"
		   "    VALUES  :\n"
		   "        VAL\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_def_no_arg)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	const make_def_t def = make_add_act(&make, make_create_def(&make, STRV("def")));
	make_def_add_act(&make, def, make_var_add_val(&make, make_create_var(&make, STRV("$(1)"), MAKE_VAR_INST, NULL), MSTR(STRV("VAL"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint var_id = ARR_END;

	make_add_act(&make, make_create_eval_def(&make, def));

	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	log_set_quiet(0, 0);
	strbuf_find(&vars.names, STRV(""), &var_id);

	strv_t var_exp_a = make_vars_get_expanded(&vars, var_id);
	EXPECT_STRN(var_exp_a.data, "VAL", var_exp_a.len);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 49);
	EXPECT_STR(buf,
		   "define def\n"
		   "$(1) := VAL\n"
		   "endef\n"
		   "$(eval $(call def))\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 115);
	EXPECT_STR(buf,
		   "DEF\n"
		   "    NAME: 'def'\n"
		   "VAR\n"
		   "    NAME    : $(1)\n"
		   "    VALUES  :\n"
		   "        VAL\n"
		   "EVAL DEF\n"
		   "    DEF: 'def'\n"
		   "    ARGS:\n"
		   "        def\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_def_args)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	const make_def_t def = make_add_act(&make, make_create_def(&make, STRV("def")));
	make_var_t var	     = make_def_add_act(&make, def, make_create_var(&make, STRV("$(1)"), MAKE_VAR_INST, NULL));

	make_var_add_val(&make, var, MSTR(STRV("$(0)")));
	make_var_add_val(&make, var, MSTR(STRV("$(1)")));
	make_var_add_val(&make, var, MSTR(STRV("$(2)")));
	make_var_add_val(&make, var, MSTR(STRV("$(3)")));
	make_var_add_val(&make, var, MSTR(STRV("$(4)")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint var_id = ARR_END;

	make_eval_def_t eval = make_add_act(&make, make_create_eval_def(&make, def));
	make_eval_def_add_arg(&make, eval, MSTR(STRV("VAR")));

	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	log_set_quiet(0, 0);
	strbuf_find(&vars.names, STRV("VAR"), &var_id);
	strv_t var_exp_a = make_vars_get_expanded(&vars, var_id);
	EXPECT_STRN(var_exp_a.data, "def VAR   ", var_exp_a.len);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 74);
	EXPECT_STR(buf,
		   "define def\n"
		   "$(1) := $(0) $(1) $(2) $(3) $(4)\n"
		   "endef\n"
		   "$(eval $(call def,VAR))\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 180);
	EXPECT_STR(buf,
		   "DEF\n"
		   "    NAME: 'def'\n"
		   "VAR\n"
		   "    NAME    : $(1)\n"
		   "    VALUES  :\n"
		   "        $(0)\n"
		   "        $(1)\n"
		   "        $(2)\n"
		   "        $(3)\n"
		   "        $(4)\n"
		   "EVAL DEF\n"
		   "    DEF: 'def'\n"
		   "    ARGS:\n"
		   "        def\n"
		   "        VAR\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_def_var_invalid)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	const make_def_t def = make_add_act(&make, make_create_def(&make, STRV("def")));

	make_var_add_val(&make, make_def_add_act(&make, def, make_create_var(&make, STRV("A"), MAKE_VAR_INST, NULL)), MSTR(STRV("V")));
	make_var_add_val(&make, make_def_add_act(&make, def, make_create_var(&make, STRV("B"), MAKE_VAR_INST, NULL)), MSTR(STRV("$(A)")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	uint b_id = ARR_END;

	make_add_act(&make, make_create_eval_def(&make, def));

	log_set_quiet(0, 1);
	EXPECT_EQ(make_vars_eval(&make, &vars), 1);
	log_set_quiet(0, 0);

	strbuf_find(&vars.names, STRV("B"), &b_id);
	strv_t var_exp_a = make_vars_get_expanded(&vars, b_id);
	EXPECT_STRN(var_exp_a.data, "", var_exp_a.len);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 54);
	EXPECT_STR(buf,
		   "define def\n"
		   "A := V\n"
		   "B := $(A)\n"
		   "endef\n"
		   "$(eval $(call def))\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 157);
	EXPECT_STR(buf,
		   "DEF\n"
		   "    NAME: 'def'\n"
		   "VAR\n"
		   "    NAME    : A\n"
		   "    VALUES  :\n"
		   "        V\n"
		   "VAR\n"
		   "    NAME    : B\n"
		   "    VALUES  :\n"
		   "        $(A)\n"
		   "EVAL DEF\n"
		   "    DEF: 'def'\n"
		   "    ARGS:\n"
		   "        def\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_def_var_imm)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("A"), MAKE_VAR_INST, NULL)), MSTR(STRV("V")));

	const make_def_t def = make_add_act(&make, make_create_def(&make, STRV("def")));
	make_var_add_val(&make, make_def_add_act(&make, def, make_create_var(&make, STRV("B"), MAKE_VAR_INST, NULL)), MSTR(STRV("$(A)")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_add_act(&make, make_create_eval_def(&make, def));

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	uint id = ARR_END;
	strv_t exp, res;

	strbuf_find(&vars.names, STRV("B"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "V", exp.len);
	res = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(res.data, "V", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_def_var)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	const make_def_t def = make_add_act(&make, make_create_def(&make, STRV("def")));

	make_var_t a = make_var_add_val(
		&make, make_def_add_act(&make, def, make_create_var(&make, STRV("A"), MAKE_VAR_INST, NULL)), MSTR(STRV("V")));

	make_var_add_val(&make, make_def_add_act(&make, def, make_create_var(&make, STRV("B"), MAKE_VAR_INST, NULL)), MVAR(a));
	make_var_add_val(&make, make_def_add_act(&make, def, make_create_var(&make, STRV("C"), MAKE_VAR_INST, NULL)), MSTR(STRV("$$(A)")));

	make_def_add_act(&make, def, make_create_rule(&make, MRULE(MVAR(a)), 1));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_add_act(&make, make_create_eval_def(&make, def));

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	uint id = ARR_END;
	strv_t val;

	strbuf_find(&vars.names, STRV("B"), &id);
	val = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(val.data, "$(A)", val.len);
	val = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(val.data, "V", val.len);

	strbuf_find(&vars.names, STRV("B"), &id);
	val = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(val.data, "$(A)", val.len);
	val = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(val.data, "V", val.len);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 74);
	EXPECT_STR(buf,
		   "define def\n"
		   "A := V\n"
		   "B := $$(A)\n"
		   "C := $$(A)\n"
		   "$$(A):\n"
		   "\n"
		   "endef\n"
		   "$(eval $(call def))\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 242);
	EXPECT_STR(buf,
		   "DEF\n"
		   "    NAME: 'def'\n"
		   "VAR\n"
		   "    NAME    : A\n"
		   "    VALUES  :\n"
		   "        V\n"
		   "VAR\n"
		   "    NAME    : B\n"
		   "    VALUES  :\n"
		   "        $$(A)\n"
		   "VAR\n"
		   "    NAME    : C\n"
		   "    VALUES  :\n"
		   "        $$(A)\n"
		   "RULE\n"
		   "    TARGET: $$(A)\n"
		   "    DEPENDS:\n"
		   "EVAL DEF\n"
		   "    DEF: 'def'\n"
		   "    ARGS:\n"
		   "        def\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_var_not_found)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL)), MVAR(MAKE_END));

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

TEST(make_eval_print_inc)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_inc_t inc = make_add_act(&make, make_create_inc(&make, STRV("inc.mk")));
	make_var_add_val(&make, make_inc_add_act(&make, inc, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL)), MSTR(STRV("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	uint id = ARR_END;
	strv_t val;

	strbuf_find(&vars.names, STRV("VAR"), &id);
	val = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(val.data, "VAL", val.len);
	val = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(val.data, "VAL", val.len);

	char buf[128] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 15);
	EXPECT_STR(buf, "include inc.mk\n");

	EXPECT_EQ(make_inc_print(&make, inc, PRINT_DST_BUF(buf, sizeof(buf), 0)), 11);
	EXPECT_STR(buf, "VAR := VAL\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 75);
	EXPECT_STR(buf,
		   "INCLUDE\n"
		   "    PATH: 'inc.mk'\n"
		   "VAR\n"
		   "    NAME    : VAR\n"
		   "    VALUES  :\n"
		   "        VAL\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_print_inc_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_add_act(&make, make_create_inc(&make, STRV("inc.mk")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	char buf[128] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 15);
	EXPECT_STR(buf, "include inc.mk\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 27);
	EXPECT_STR(buf,
		   "INCLUDE\n"
		   "    PATH: 'inc.mk'\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_ext_override)
{
	START;

	make_t make = {0};
	make_init(&make, 3, 3, 1, 8, ALLOC_STD);

	uint ext = ARR_END;
	make_add_act(&make, make_create_var_ext(&make, STRV("EXT"), &ext));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("EXT"), MAKE_VAR_INST, &ext)), MSTR(STRV("VAL")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("EXT"), MAKE_VAR_APP, &ext)), MSTR(STRV("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	{
		make_vars_eval(&make, &vars);
		uint extr;
		strbuf_find(&vars.names, STRV("EXT"), &extr);
		strv_t exp = make_vars_get_expanded(&vars, extr);
		EXPECT_STRN(exp.data, "VAL VAL", exp.len);
		strv_t res = make_vars_get_resolved(&vars, extr);
		EXPECT_STRN(res.data, "VAL VAL", res.len);
	}
	{
		make_ext_set_val(&make, ext, MSTR(STRV("EXT")));
		make_vars_eval(&make, &vars);
		uint extr;
		strbuf_find(&vars.names, STRV("EXT"), &extr);
		strv_t exp = make_vars_get_expanded(&vars, extr);
		EXPECT_STRN(exp.data, "EXT", exp.len);
		strv_t res = make_vars_get_resolved(&vars, extr);
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
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_var_t var =
		make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL)), MSTR(STRV("A")));
	uint t = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_INST, &t)), MVAR(var));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_APP, &t)), MVAR(var));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STRV("B")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);

	uint tr;
	strbuf_find(&vars.names, STRV("T"), &tr);

	strv_t exp = make_vars_get_expanded(&vars, tr);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR)", exp.len);
	strv_t res = make_vars_get_resolved(&vars, tr);
	EXPECT_STRN(res.data, "A A", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_ref_app)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	uint var_id = ARR_END;
	make_var_t var =
		make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("A")));
	uint t = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_REF, &t)), MVAR(var));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_APP, &t)), MVAR(var));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("B")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);

	uint tr;
	strbuf_find(&vars.names, STRV("T"), &tr);

	strv_t exp = make_vars_get_expanded(&vars, tr);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR)", exp.len);
	strv_t res = make_vars_get_resolved(&vars, tr);
	EXPECT_STRN(res.data, "B B", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_inst_if)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	uint var_id = ARR_END;
	make_var_t var =
		make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("A")));
	make_var_t t = make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_INST, NULL)), MVAR(var));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("B")));

	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(t), MSTR(STRV("A"))));
	uint r			= ARR_END;
	make_if_add_true_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("R"), MAKE_VAR_INST, &r), MSTR(STRV("C"))));
	make_if_add_false_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("R"), MAKE_VAR_INST, &r), MSTR(STRV("D"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);

	uint rr;
	strbuf_find(&vars.names, STRV("R"), &rr);

	strv_t exp = make_vars_get_expanded(&vars, rr);
	EXPECT_STRN(exp.data, "C", exp.len);
	strv_t res = make_vars_get_resolved(&vars, rr);
	EXPECT_STRN(res.data, "C", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_ref_if)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	uint var_id = ARR_END;
	make_var_t var =
		make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("A")));
	make_var_t t = make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("T"), MAKE_VAR_REF, NULL)), MVAR(var));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("B")));

	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(t), MSTR(STRV("A"))));
	uint r			= ARR_END;
	make_if_add_true_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("R"), MAKE_VAR_INST, &r), MSTR(STRV("C"))));
	make_if_add_false_act(
		&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRV("R"), MAKE_VAR_INST, &r), MSTR(STRV("D"))));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);

	uint rr;
	strbuf_find(&vars.names, STRV("R"), &rr);

	strv_t exp = make_vars_get_expanded(&vars, rr);
	EXPECT_STRN(exp.data, "D", exp.len);
	strv_t res = make_vars_get_resolved(&vars, rr);
	EXPECT_STRN(res.data, "D", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_name)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL)), MSTR(STRV("A")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("$(VAR)_VAR"), MAKE_VAR_INST, NULL)), MSTR(STRV("VAL")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);

	uint var;
	strbuf_find(&vars.names, STRV("A_VAR"), &var);

	strv_t exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(exp.data, "VAL", exp.len);
	strv_t res = make_vars_get_resolved(&vars, var);
	EXPECT_STRN(res.data, "VAL", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_eval_def_name)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, NULL)), MSTR(STRV("A")));

	make_def_t def = make_add_act(&make, make_create_def(&make, STRV("def")));

	make_var_add_val(
		&make, make_def_add_act(&make, def, make_create_var(&make, STRV("$(VAR)_VAR"), MAKE_VAR_INST, NULL)), MSTR(STRV("VAL")));

	make_add_act(&make, make_create_eval_def(&make, def));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);

	uint var;
	strbuf_find(&vars.names, STRV("A_VAR"), &var);

	strv_t exp = make_vars_get_expanded(&vars, var);
	EXPECT_STRN(exp.data, "VAL", exp.len);
	strv_t res = make_vars_get_resolved(&vars, var);
	EXPECT_STRN(res.data, "VAL", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_vars_print)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	uint var = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var)), MSTR(STRV("A")));

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	make_vars_eval(&make, &vars);

	char buf[256] = {0};
	make_vars_print(&vars, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf, "VAR              A                                                                A\n");

	make_vars_free(&vars);
	make_free(&make);

	END;
}

TEST(make_print_rule_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRV("rule"))), 1));

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
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

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
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_add_act(&make, make_create_rule(&make, MRULEACT(MSTR(STRV("rule")), STRV("/action")), 1));

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
	make_init(&make, 2, 2, 1, 8, ALLOC_STD);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRV("rule"))), 1));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("depend"))));

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
	make_init(&make, 3, 3, 1, 8, ALLOC_STD);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRV("rule"))), 1));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("depend1"))));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("depend2"))));

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
	make_init(&make, 6, 6, 1, 8, ALLOC_STD);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRV("rule"))), 1));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRV("cmd1"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRV("cmd2"))));

	make_rule_t if_rule = make_rule_add_act(&make, rule, make_create_if(&make, MSTR(STRV("L")), MSTR(STRV("R"))));
	make_if_add_true_act(&make, if_rule, make_create_cmd(&make, MCMD(STRV("cmd3"))));
	make_if_add_false_act(&make, if_rule, make_create_cmd(&make, MCMD(STRV("cmd4"))));

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

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 228);
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
		   "CMD\n"
		   "    ARG1: cmd3\n"
		   "    ARG2: \n"
		   "    TYPE: 0\n"
		   "CMD\n"
		   "    ARG1: cmd4\n"
		   "    ARG2: \n"
		   "    TYPE: 0\n");
	make_free(&make);

	END;
}

TEST(make_print_cmd)
{
	START;

	make_t make = {0};
	make_init(&make, 5, 5, 1, 8, ALLOC_STD);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRV("rule"))), 1));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRV("cmd"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDCHILD(STRV("dir"), STRV_NULL)));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDCHILD(STRV("dir"), STRV("action"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDERR(STRV("msg"))));

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
	RUN(make_ext_set_val_oom_val);
	RUN(make_vars_init_free);
	RUN(make_vars_eval);
	RUN(make_vars_replace);
	RUN(make_vars_eval_oom_add_var);
	RUN(make_vars_eval_oom_var_inst);
	RUN(make_vars_eval_oom_var_append);
	RUN(make_vars_eval_oom_if);
	RUN(make_vars_eval_invalid_eval_def);
	RUN(make_vars_eval_var_type);
	RUN(make_vars_eval_ext_not_set);
	RUN(make_vars_get_expanded);
	RUN(make_vars_get_resolved);
	RUN(make_inc_print);
	RUN(make_print);
	RUN(make_dbg);
	RUN(make_eval_print_empty);
	RUN(make_eval_print_var_inst_empty);
	RUN(make_eval_print_var_inst);
	RUN(make_eval_print_var_inst2);
	RUN(make_eval_print_var_inst_out);
	RUN(make_eval_print_var_ref);
	RUN(make_eval_print_var_app);
	RUN(make_eval_print_var_ext_inst);
	RUN(make_eval_print_var_ext);
	RUN(make_eval_print_if_empty);
	RUN(make_eval_print_if_lr);
	RUN(make_eval_print_var_if_true);
	RUN(make_eval_print_var_if_false);
	RUN(make_eval_print_def_empty);
	RUN(make_eval_print_def);
	RUN(make_eval_print_def_no_arg);
	RUN(make_eval_print_def_args);
	RUN(make_eval_print_def_var_invalid);
	RUN(make_eval_print_def_var_imm);
	RUN(make_eval_print_def_var);
	RUN(make_eval_print_var_not_found);
	RUN(make_eval_print_inc);
	RUN(make_eval_print_inc_empty);
	RUN(make_eval_ext_override);
	RUN(make_eval_inst_app);
	RUN(make_eval_ref_app);
	RUN(make_eval_inst_if);
	RUN(make_eval_ref_if);
	RUN(make_eval_name);
	RUN(make_eval_def_name);
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

TEST(make_vars)
{
	START;

	make_t make = {0};
	make_init(&make, 5, 5, 1, 8, ALLOC_STD);

	uint ext_empty = ARR_END, ext_set = ARR_END, ext_var = ARR_END;
	make_add_act(&make, make_create_var_ext(&make, STRV("EXT_NOT"), NULL));
	make_add_act(&make, make_create_var_ext(&make, STRV("EXT_EMPTY"), &ext_empty));
	make_add_act(&make, make_create_var_ext(&make, STRV("EXT_SET"), &ext_set));
	make_add_act(&make, make_create_var_ext(&make, STRV("EXT_VAR"), &ext_var));

	uint var_ext_not = ARR_END, var_ext_empty = ARR_END, var_ext_set = ARR_END;
	make_add_act(&make, make_create_var_ext(&make, STRV("VAR_EXT_NOT"), &var_ext_not));
	make_add_act(&make, make_create_var_ext(&make, STRV("VAR_EXT_EMPTY"), &var_ext_empty));
	make_add_act(&make, make_create_var_ext(&make, STRV("VAR_EXT_SET"), &var_ext_set));

	uint var_id = ARR_END;
	make_var_t var =
		make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("V1")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("IMM"), MAKE_VAR_INST, NULL)), MSTR(STRV("$(VAR)")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("REC"), MAKE_VAR_REF, NULL)), MSTR(STRV("$(VAR)")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("V2")));

	uint app_imm_id = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("APP_IMM"), MAKE_VAR_INST, &app_imm_id)), MVAR(var));
	make_var_add_val(
		&make, make_add_act(&make, make_create_var(&make, STRV("APP_IMM"), MAKE_VAR_APP, &app_imm_id)), MSTR(STRV("$(VAR)")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("APP_IMM"), MAKE_VAR_APP, &app_imm_id)), MVAR(var));

	uint app_rec_id = ARR_END;
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("APP_REC"), MAKE_VAR_REF, &app_rec_id)), MVAR(var));
	make_var_add_val(
		&make, make_add_act(&make, make_create_var(&make, STRV("APP_REC"), MAKE_VAR_APP, &app_rec_id)), MSTR(STRV("$(VAR)")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("APP_REC"), MAKE_VAR_APP, &app_rec_id)), MVAR(var));

	make_add_act(&make, make_create_empty(&make));

	make_var_add_val(
		&make, make_add_act(&make, make_create_var(&make, STRV("VAR_EXT_NOT"), MAKE_VAR_INST, &var_ext_not)), MSTR(STRV("VAR")));
	make_var_add_val(&make,
			 make_add_act(&make, make_create_var(&make, STRV("VAR_EXT_EMPTY"), MAKE_VAR_INST, &var_ext_empty)),
			 MSTR(STRV("VAR")));
	make_var_add_val(
		&make, make_add_act(&make, make_create_var(&make, STRV("VAR_EXT_SET"), MAKE_VAR_INST, &var_ext_set)), MSTR(STRV("VAR")));

	make_add_act(&make, make_create_empty(&make));

	make_def_t def	   = make_add_act(&make, make_create_def(&make, STRV("def")));
	make_var_t def_var = make_var_add_val(
		&make, make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_VAR"), MAKE_VAR_INST, NULL)), MSTR(STRV("D")));
	make_var_add_val(&make,
			 make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_VAR_IMM"), MAKE_VAR_INST, NULL)),
			 MSTR(STRV("$$(DEF_VAR)")));
	make_var_add_val(
		&make, make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_VAR_IMM_VAR"), MAKE_VAR_INST, NULL)), MVAR(def_var));
	make_var_add_val(
		&make, make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_IMM"), MAKE_VAR_INST, NULL)), MSTR(STRV("$(VAR)")));
	make_var_add_val(&make, make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_IMM_VAR"), MAKE_VAR_INST, NULL)), MVAR(var));
	make_var_add_val(&make,
			 make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_IMM_ESC"), MAKE_VAR_INST, NULL)),
			 MSTR(STRV("$$(VAR)")));
	make_var_add_val(
		&make, make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_REC"), MAKE_VAR_REF, NULL)), MSTR(STRV("$(VAR)")));
	make_var_add_val(&make, make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_REC_VAR"), MAKE_VAR_REF, NULL)), MVAR(var));
	make_var_add_val(&make,
			 make_def_add_act(&make, def, make_create_var(&make, STRV("DEF_REC_ESC"), MAKE_VAR_REF, NULL)),
			 MSTR(STRV("$$(VAR)")));
	make_var_add_val(
		&make, make_def_add_act(&make, def, make_create_var(&make, STRV("$(0)"), MAKE_VAR_INST, NULL)), MSTR(STRV("$$($(1))")));

	make_add_act(&make, make_create_empty(&make));

	make_eval_def_add_arg(&make, make_add_act(&make, make_create_eval_def(&make, def)), MSTR(STRV("VAR")));

	make_add_act(&make, make_create_empty(&make));

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRV("VAR"), MAKE_VAR_INST, &var_id)), MSTR(STRV("V3")));

	make_add_act(&make, make_create_empty(&make));

	make_rule_t all = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRV("all"))), 1));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo VAR             = $(VAR)           #V3"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo IMM             = $(IMM)           #V1"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo REC             = $(REC)           #V3"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo APP_IMM         = $(APP_IMM)       #V2 V2 V2"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo APP_REC         = $(APP_REC)       #V3 V3 V3"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo EXT_NOT         = $(EXT_NOT)       #"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo EXT_EMPTY       = $(EXT_EMPTY)     #"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo EXT_SET         = $(EXT_SET)       #EXT"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo EXT_VAR         = $(EXT_VAR)       #V3"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo VAR_EXT_NOT     = $(VAR_EXT_NOT)   #VAR"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo VAR_EXT_EMPTY   = $(VAR_EXT_EMPTY) #"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo VAR_EXT_SET     = $(VAR_EXT_SET)   #EXT"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_VAR         = $(DEF_VAR)       #D"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_VAR_IMM     = $(DEF_VAR_IMM)   #D"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_VAR_IMM_VAR = $(DEF_VAR_IMM_VAR) #D"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_IMM         = $(DEF_IMM)       #V2"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_IMM_VAR     = $(DEF_IMM_VAR)   #V2"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_IMM_ESC     = $(DEF_IMM_ESC)   #V2"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_REC         = $(DEF_REC)       #V2"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_REC_VAR     = $(DEF_REC_VAR)   #V3"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo DEF_REC_ESC     = $(DEF_REC_ESC)   #V3"))));
	make_rule_add_act(&make, all, make_create_cmd(&make, MCMD(STRV("@echo def             = $(def)           #V2"))));

	make_ext_set_val(&make, ext_empty, MSTR(STRV("")));
	make_ext_set_val(&make, ext_set, MSTR(STRV("EXT")));
	make_ext_set_val(&make, ext_var, MSTR(STRV("$(VAR)")));

	make_ext_set_val(&make, var_ext_empty, MSTR(STRV("")));
	make_ext_set_val(&make, var_ext_set, MSTR(STRV("EXT")));

	char buf[4096] = {0};
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1506);
	EXPECT_STR(buf,
		   "VAR := V1\n"
		   "IMM := $(VAR)\n"
		   "REC = $(VAR)\n"
		   "VAR := V2\n"
		   "APP_IMM := $(VAR)\n"
		   "APP_IMM += $(VAR)\n"
		   "APP_IMM += $(VAR)\n"
		   "APP_REC = $(VAR)\n"
		   "APP_REC += $(VAR)\n"
		   "APP_REC += $(VAR)\n"
		   "\n"
		   "VAR_EXT_NOT := VAR\n"
		   "VAR_EXT_EMPTY := VAR\n"
		   "VAR_EXT_SET := VAR\n"
		   "\n"
		   "define def\n"
		   "DEF_VAR := D\n"
		   "DEF_VAR_IMM := $$(DEF_VAR)\n"
		   "DEF_VAR_IMM_VAR := $$(DEF_VAR)\n"
		   "DEF_IMM := $(VAR)\n"
		   "DEF_IMM_VAR := $(VAR)\n"
		   "DEF_IMM_ESC := $$(VAR)\n"
		   "DEF_REC = $(VAR)\n"
		   "DEF_REC_VAR = $(VAR)\n"
		   "DEF_REC_ESC = $$(VAR)\n"
		   "$(0) := $$($(1))\n"
		   "endef\n"
		   "\n"
		   "$(eval $(call def,VAR))\n"
		   "\n"
		   "VAR := V3\n"
		   "\n"
		   "all:\n"
		   "	@echo VAR             = $(VAR)           #V3\n"
		   "	@echo IMM             = $(IMM)           #V1\n"
		   "	@echo REC             = $(REC)           #V3\n"
		   "	@echo APP_IMM         = $(APP_IMM)       #V2 V2 V2\n"
		   "	@echo APP_REC         = $(APP_REC)       #V3 V3 V3\n"
		   "	@echo EXT_NOT         = $(EXT_NOT)       #\n"
		   "	@echo EXT_EMPTY       = $(EXT_EMPTY)     #\n"
		   "	@echo EXT_SET         = $(EXT_SET)       #EXT\n"
		   "	@echo EXT_VAR         = $(EXT_VAR)       #V3\n"
		   "	@echo VAR_EXT_NOT     = $(VAR_EXT_NOT)   #VAR\n"
		   "	@echo VAR_EXT_EMPTY   = $(VAR_EXT_EMPTY) #\n"
		   "	@echo VAR_EXT_SET     = $(VAR_EXT_SET)   #EXT\n"
		   "	@echo DEF_VAR         = $(DEF_VAR)       #D\n"
		   "	@echo DEF_VAR_IMM     = $(DEF_VAR_IMM)   #D\n"
		   "	@echo DEF_VAR_IMM_VAR = $(DEF_VAR_IMM_VAR) #D\n"
		   "	@echo DEF_IMM         = $(DEF_IMM)       #V2\n"
		   "	@echo DEF_IMM_VAR     = $(DEF_IMM_VAR)   #V2\n"
		   "	@echo DEF_IMM_ESC     = $(DEF_IMM_ESC)   #V2\n"
		   "	@echo DEF_REC         = $(DEF_REC)       #V2\n"
		   "	@echo DEF_REC_VAR     = $(DEF_REC_VAR)   #V3\n"
		   "	@echo DEF_REC_ESC     = $(DEF_REC_ESC)   #V3\n"
		   "	@echo def             = $(def)           #V2\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 3676);

	make_vars_t vars = {0};
	make_vars_init(&make, &vars, ALLOC_STD);

	EXPECT_EQ(make_vars_eval(&make, &vars), 0);

	make_vars_print(&vars, PRINT_DST_BUF(buf, sizeof(buf), 0));
	EXPECT_STR(buf,
		   "EXT_NOT                                                                           \n"
		   "EXT_EMPTY                                                                         \n"
		   "EXT_SET          EXT                                                              EXT\n"
		   "EXT_VAR          $(VAR)                                                           $(VAR)\n"
		   "VAR_EXT_NOT      VAR                                                              VAR\n"
		   "VAR_EXT_EMPTY                                                                     \n"
		   "VAR_EXT_SET      EXT                                                              EXT\n"
		   "VAR              V3                                                               V3\n"
		   "IMM              $(VAR)                                                           V1\n"
		   "REC              $(VAR)                                                           $(VAR)\n"
		   "APP_IMM          $(VAR) $(VAR) $(VAR)                                             V2 V2 V2\n"
		   "APP_REC          $(VAR) $(VAR) $(VAR)                                             $(VAR) $(VAR) $(VAR)\n"
		   "DEF_VAR          D                                                                D\n"
		   "DEF_VAR_IMM      $(DEF_VAR)                                                       D\n"
		   "DEF_VAR_IMM_VAR  $(DEF_VAR)                                                       D\n"
		   "DEF_IMM          V2                                                               V2\n"
		   "DEF_IMM_VAR      V2                                                               V2\n"
		   "DEF_IMM_ESC      $(VAR)                                                           V2\n"
		   "DEF_REC          V2                                                               V2\n"
		   "DEF_REC_VAR      V2                                                               V2\n"
		   "DEF_REC_ESC      $(VAR)                                                           $(VAR)\n"
		   "def              $(VAR)                                                           V2\n");

	uint id;
	strv_t exp, res;

	strbuf_find(&vars.names, STRV("VAR"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "V3", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V3", res.len);

	strbuf_find(&vars.names, STRV("IMM"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V1", res.len);

	strbuf_find(&vars.names, STRV("REC"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V3", res.len);

	strbuf_find(&vars.names, STRV("APP_IMM"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR) $(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V2 V2 V2", res.len);

	strbuf_find(&vars.names, STRV("APP_REC"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR) $(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V3 V3 V3", res.len);

	strbuf_find(&vars.names, STRV("EXT_NOT"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "", res.len);

	strbuf_find(&vars.names, STRV("EXT_EMPTY"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "", res.len);

	strbuf_find(&vars.names, STRV("EXT_SET"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "EXT", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "EXT", res.len);

	strbuf_find(&vars.names, STRV("EXT_VAR"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V3", res.len);

	strbuf_find(&vars.names, STRV("VAR_EXT_NOT"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "VAR", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "VAR", res.len);

	strbuf_find(&vars.names, STRV("VAR_EXT_EMPTY"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "", res.len);

	strbuf_find(&vars.names, STRV("VAR_EXT_SET"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "EXT", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "EXT", res.len);

	strbuf_find(&vars.names, STRV("DEF_VAR"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "D", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "D", res.len);

	strbuf_find(&vars.names, STRV("DEF_VAR_IMM"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(DEF_VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "D", res.len);

	strbuf_find(&vars.names, STRV("DEF_VAR_IMM_VAR"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(DEF_VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "D", res.len);

	strbuf_find(&vars.names, STRV("DEF_IMM"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "V2", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V2", res.len);

	strbuf_find(&vars.names, STRV("DEF_IMM_ESC"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V2", res.len);

	strbuf_find(&vars.names, STRV("DEF_IMM_ESC_VAR"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V2", res.len);

	strbuf_find(&vars.names, STRV("DEF_REC"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "V2", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V2", res.len);

	strbuf_find(&vars.names, STRV("DEF_REC_ESC"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V3", res.len);

	strbuf_find(&vars.names, STRV("DEF_REC_ESC_VAR"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V3", res.len);

	strbuf_find(&vars.names, STRV("def"), &id);
	exp = make_vars_get_expanded(&vars, id);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_vars_get_resolved(&vars, id);
	EXPECT_STRN(res.data, "V2", res.len);

	make_vars_free(&vars);
	make_free(&make);

	END;
}

STEST(make)
{
	SSTART;

	RUN(make_init_free);
	RUN(make_create);
	RUN(make_add);
	RUN(make_rule_get_target);
	RUN(make_eval_print);
	RUN(make_vars);

	SEND;
}
