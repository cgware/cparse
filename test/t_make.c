#include "file/make.h"
#include "file/make_priv.h"

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

	make_free(&make);
	make_free(NULL);

	EXPECT_EQ(make.arrs.data, NULL);
	EXPECT_EQ(make.acts.data, NULL);

	END;
}

TEST(make_empty)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_empty(NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(make_empty(&make, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_empty(&make, NULL), 0);

	make_free(&make);
	END;
}

TEST(make_var)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t var;

	EXPECT_EQ(make_var(NULL, STRV(""), MAKE_VAR_INST, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(make_var(&make, STRV(""), MAKE_VAR_INST, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_var(&make, STRV(""), MAKE_VAR_INST, &var), 0);
	EXPECT_EQ(var, 0);
	EXPECT_EQ(make_var(&make, STRV(""), MAKE_VAR_INST, &var), 0);
	EXPECT_EQ(var, 1);
	EXPECT_EQ(make_var(&make, STRV(""), MAKE_VAR_APP, &var), 0);
	EXPECT_EQ(var, 2);

	make_free(&make);

	END;
}

TEST(make_var_oom)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 1, 1, 1, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_var(&make, STRV("a"), MAKE_VAR_INST, NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_var_var)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t var;
	make_var(&make, STRV(""), MAKE_VAR_INST, &var);

	EXPECT_EQ(make_var_var(NULL, make.acts.cnt, MAKE_VAR_INST, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_var_var(&make, make.acts.cnt, MAKE_VAR_INST, NULL), 1);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(make_var_var(&make, var, MAKE_VAR_INST, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_var_var(&make, var, MAKE_VAR_INST, &var), 0);
	EXPECT_EQ(var, 1);
	EXPECT_EQ(make_var_var(&make, var, MAKE_VAR_INST, &var), 0);
	EXPECT_EQ(var, 2);

	make_free(&make);

	END;
}

TEST(make_var_ext)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t var;

	EXPECT_EQ(make_var_ext(NULL, STRV(""), NULL), 1);
	mem_oom(1);
	EXPECT_EQ(make_var_ext(&make, STRV("a"), NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_var_ext(&make, STRV(""), &var), 0);
	EXPECT_EQ(var, 0);
	EXPECT_EQ(make_var_ext(&make, STRV(""), &var), 0);
	EXPECT_EQ(var, 1);
	EXPECT_EQ(make_var_ext(&make, STRV(""), &var), 0);
	EXPECT_EQ(var, 2);

	make_free(&make);

	END;
}

TEST(make_rule)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t rule;

	EXPECT_EQ(make_rule(NULL, MRULE(MVAR(0)), 0, NULL), 1);
	make_create_str_t str = (make_create_str_t){.type = 2};
	EXPECT_EQ(make_rule(&make, MRULE(str), 0, &rule), 0);
	EXPECT_EQ(rule, 0);
	EXPECT_EQ(make_rule(&make, MRULE(MVAR(0)), 0, &rule), 0);
	EXPECT_EQ(rule, 1);
	mem_oom(1);
	EXPECT_EQ(make_rule(&make, MRULE(MSTR(STRV(""))), 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_rule(&make, MRULE(MSTR(STRV(""))), 0, &rule), 0);
	EXPECT_EQ(rule, 2);

	make_free(&make);

	END;
}

TEST(make_rule_oom_target)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_rule(&make, MRULE(MSTR(STRV("a"))), 0, NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_rule_oom_action)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_rule(&make, MRULEACT(MSTR(STRV("aaaaaaaa")), STRV("a")), 0, NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_phony)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t empty;

	EXPECT_EQ(make_phony(NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(make_phony(&make, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_phony(&make, &empty), 0);
	EXPECT_EQ(empty, 0);

	make_free(&make);

	END;
}

TEST(make_cmd)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t cmd;

	EXPECT_EQ(make_cmd(NULL, MCMD(STRV("")), NULL), 1);
	mem_oom(1);
	EXPECT_EQ(make_cmd(&make, MCMD(STRV("")), NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_cmd(&make, MCMD(STRV("")), &cmd), 0);
	EXPECT_EQ(cmd, 0);

	make_free(&make);

	END;
}

TEST(make_cmd_oom_arg1)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_cmd(&make, MCMD(STRV("a")), NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_cmd_oom_arg2)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_cmd(&make, MCMDCHILD(STRV("aaaaaaaa"), STRV("a")), NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_if)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t mif;

	EXPECT_EQ(make_if(NULL, MVAR(0), MVAR(0), NULL), 1);
	EXPECT_EQ(make_if(&make, MVAR(0), MVAR(0), &mif), 0);
	EXPECT_EQ(mif, 0);
	mem_oom(1);
	EXPECT_EQ(make_if(&make, MSTR(STRV("")), MSTR(STRV("")), NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_if(&make, MSTR(STRV("")), MSTR(STRV("")), &mif), 0);
	EXPECT_EQ(mif, 1);

	make_free(&make);

	END;
}

TEST(make_if_oom_l)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_if(&make, MSTR(STRV("a")), MSTR(STRV("")), NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_if_oom_r)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_if(&make, MSTR(STRV("aaaaaaaa")), MSTR(STRV("a")), NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_def)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t def;

	EXPECT_EQ(make_def(NULL, STRV_NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(make_def(&make, STRV(""), NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_def(&make, STRV(""), &def), 0);
	EXPECT_EQ(def, 0);

	make_free(&make);

	END;
}

TEST(make_def_oom_name)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_def(&make, STRV("a"), NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_eval_def)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t def, eval;

	make_def(&make, STRV("def"), &def);

	EXPECT_EQ(make_eval_def(NULL, make.acts.cnt, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(make_eval_def(&make, def, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_eval_def(&make, def, &eval), 0);
	EXPECT_EQ(eval, 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval_def(&make, make.acts.cnt, NULL), 1);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_inc)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_inc(NULL, STRV_NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(make_inc(&make, STRV(""), NULL), 1);
	mem_oom(0);
	EXPECT_EQ(make_inc(&make, STRV(""), NULL), 0);

	make_free(&make);

	END;
}

TEST(make_inc_oom_name)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(make_inc(&make, STRV("a"), NULL), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_create)
{
	SSTART;
	RUN(make_empty);
	RUN(make_var);
	RUN(make_var_oom);
	RUN(make_var_var);
	RUN(make_var_ext);
	RUN(make_rule);
	RUN(make_rule_oom_target);
	RUN(make_rule_oom_action);
	RUN(make_phony);
	RUN(make_cmd);
	RUN(make_cmd_oom_arg1);
	RUN(make_cmd_oom_arg2);
	RUN(make_if);
	RUN(make_if_oom_l);
	RUN(make_if_oom_r);
	RUN(make_def);
	RUN(make_def_oom_name);
	RUN(make_eval_def);
	RUN(make_inc);
	RUN(make_inc_oom_name);
	SEND;
}

TEST(make_add_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t root, act;

	EXPECT_EQ(make_add_act(NULL, make.acts.cnt, make.acts.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_add_act(&make, make.acts.cnt, make.acts.cnt), 1);
	log_set_quiet(0, 0);
	make_empty(&make, &root);
	make_rule(&make, MRULE(MSTR(STRV("rule"))), 0, &act);
	EXPECT_EQ(make_add_act(&make, root, act), 0);

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

	make_act_t empty, var, ext;

	make_empty(&make, &empty);
	make_var(&make, STRV(""), MAKE_VAR_INST, &var);

	make_var_ext(&make, STRV(""), &ext);
	EXPECT_EQ(make_var_add_val(NULL, make.acts.cnt, MVAR(0)), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_var_add_val(&make, make.acts.cnt, MVAR(0)), 1);
	EXPECT_EQ(make_var_add_val(&make, empty, MVAR(0)), 1);
	EXPECT_EQ(make_var_add_val(&make, ext, MVAR(0)), 1);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRV("a"))), 1);
	mem_oom(0);
	EXPECT_EQ(make_var_add_val(&make, var, MVAR(0)), 0);
	mem_oom(1);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRV("a"))), 1);
	mem_oom(0);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRV(""))), 0);

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

	make_act_t var;

	make_var(&make, STRV("aaaaaaaa"), MAKE_VAR_INST, &var);

	mem_oom(1);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRV("a"))), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_rule_add_depend)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t empty, rule;

	make_empty(&make, &empty);
	make_rule(&make, MRULE(MSTR(STRV(""))), 0, &rule);

	EXPECT_EQ(make_rule_add_depend(NULL, make.acts.cnt, MRULE(MVAR(0))), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_rule_add_depend(&make, make.acts.cnt, MRULE(MVAR(0))), 1);
	EXPECT_EQ(make_rule_add_depend(&make, empty, MRULE(MVAR(0))), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MVAR(0))), 0);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("")))), 0);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULEACT(MSTR(STRV("")), STRV(""))), 0);

	make_free(&make);

	END;
}

TEST(make_rule_add_depend_oom_str)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t rule;

	make_rule(&make, MRULE(MSTR(STRV(""))), 0, &rule);

	make.strs_used = make.strs.used = make.strs.size;

	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("a")))), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_rule_add_depend_oom_action)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t rule;

	make_rule(&make, MRULE(MSTR(STRV(""))), 0, &rule);

	make.strs_used = make.strs.used = make.strs.size - sizeof(size_t);

	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULEACT(MSTR(STRV("")), STRV("a"))), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_rule_add_depend_oom_depend)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 1, 0, 2, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t rule;

	make_rule(&make, MRULE(MSTR(STRV(""))), 0, &rule);

	make.targets.cnt = make.targets.cap;

	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("")))), 1);
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

	make_act_t rule;

	make_rule(&make, MRULE(MSTR(STRV(""))), 0, &rule);
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRV(""))));

	make.targets.cnt = make.targets.cap;

	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("")))), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_rule_add_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t empty, rule;

	make_empty(&make, &empty);
	make_rule(&make, MRULE(MSTR(STRV(""))), 0, &rule);

	EXPECT_EQ(make_rule_add_act(NULL, make.acts.cnt, make.acts.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_rule_add_act(&make, make.acts.cnt, make.acts.cnt), 1);
	EXPECT_EQ(make_rule_add_act(&make, empty, make.acts.cnt), 1);
	EXPECT_EQ(make_rule_add_act(&make, rule, make.acts.cnt), 1);
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

	make_act_t empty, mif;

	make_empty(&make, &empty);
	make_if(&make, MSTR(STRV("")), MSTR(STRV("")), &mif);

	EXPECT_EQ(make_if_add_true_act(NULL, make.acts.cnt, make.acts.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_if_add_true_act(&make, make.acts.cnt, make.acts.cnt), 1);
	EXPECT_EQ(make_if_add_true_act(&make, empty, make.acts.cnt), 1);
	EXPECT_EQ(make_if_add_true_act(&make, mif, make.acts.cnt), 1);
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

	make_act_t empty, mif;

	make_empty(&make, &empty);
	make_if(&make, MSTR(STRV("")), MSTR(STRV("")), &mif);

	EXPECT_EQ(make_if_add_false_act(NULL, make.acts.cnt, make.acts.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_if_add_false_act(&make, make.acts.cnt, make.acts.cnt), 1);
	EXPECT_EQ(make_if_add_false_act(&make, empty, make.acts.cnt), 1);
	EXPECT_EQ(make_if_add_false_act(&make, mif, make.acts.cnt), 1);
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

	make_act_t empty, def;

	make_empty(&make, &empty);
	make_def(&make, STRV(""), &def);

	EXPECT_EQ(make_def_add_act(NULL, make.acts.cnt, make.acts.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_def_add_act(&make, make.acts.cnt, make.acts.cnt), 1);
	EXPECT_EQ(make_def_add_act(&make, empty, make.acts.cnt), 1);
	EXPECT_EQ(make_def_add_act(&make, def, make.acts.cnt), 1);
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
	make_init(&make, 0, 2, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t def, eval;

	make_def(&make, STRV("def"), &def);
	mem_oom(1);
	EXPECT_EQ(make_eval_def(&make, def, NULL), 1);
	mem_oom(0);
	make_eval_def(&make, def, &eval);
	EXPECT_EQ(make_eval_def_add_arg(NULL, make.acts.cnt, MSTR(STRV_NULL)), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval_def_add_arg(&make, make.acts.cnt, MSTR(STRV_NULL)), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_eval_def_add_arg(&make, eval, MSTR(STRV(""))), 0);
	EXPECT_EQ(make_eval_def_add_arg(&make, eval, MSTR(STRV(""))), 0);

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

	make_act_t def, eval;

	make_def(&make, STRV("def"), &def);
	make_eval_def(&make, def, &eval);

	mem_oom(1);
	EXPECT_EQ(make_eval_def_add_arg(&make, eval, MSTR(STRV(""))), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_eval_def_add_arg_oom_str)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 2, 2, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t def, eval;

	make_def(&make, STRV(""), &def);
	make_eval_def(&make, def, &eval);

	make.strs_used = make.strs.used = make.strs.size;

	mem_oom(1);
	EXPECT_EQ(make_eval_def_add_arg(&make, eval, MSTR(STRV("a"))), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_inc_add_act)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t empty, def;

	make_empty(&make, &empty);
	make_inc(&make, STRV(""), &def);

	EXPECT_EQ(make_inc_add_act(NULL, make.acts.cnt, make.acts.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_inc_add_act(&make, make.acts.cnt, make.acts.cnt), 1);
	EXPECT_EQ(make_inc_add_act(&make, empty, make.acts.cnt), 1);
	EXPECT_EQ(make_inc_add_act(&make, def, make.acts.cnt), 1);
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
	RUN(make_rule_add_depend_oom_str);
	RUN(make_rule_add_depend_oom_action);
	RUN(make_rule_add_depend_oom_depend);
	RUN(make_rule_add_depend_oom_depends);
	RUN(make_rule_add_act);
	RUN(make_if_add_true_act);
	RUN(make_if_add_false_act);
	RUN(make_def_add_act);
	RUN(make_eval_def_add_arg);
	RUN(make_eval_def_add_arg_oom_arg);
	RUN(make_eval_def_add_arg_oom_str);
	RUN(make_inc_add_act);
	SEND;
}

TEST(make_rule_get_target)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 8, ALLOC_STD);

	make_act_t root, var, rule, ruleact, act;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_rule(&make, MRULE(MVAR(var)), 0, &root);
	make_rule(&make, MRULE(MSTR(STRV("a"))), 0, &rule);
	make_add_act(&make, root, rule);
	make_rule(&make, MRULEACT(MSTR(STRV("a")), STRV("/action")), 0, &ruleact);
	make_add_act(&make, root, ruleact);

	EXPECT_EQ(make_rule_get_target(NULL, MRULE(MSTR(STRV(""))), NULL), 1);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MSTR(STRV(""))), NULL), 1);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MSTR(STRV("a"))), &act), 0);
	EXPECT_EQ(act, rule);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MVAR(var)), &act), 0);
	EXPECT_EQ(act, root);

	make_free(&make);

	END;
}

TEST(make_ext_set_val)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	make_act_t ext, var;

	make_var_ext(&make, STRV("EXT"), &ext);
	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);

	EXPECT_EQ(make_ext_set_val(NULL, make.acts.cnt, MVAR(0)), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_ext_set_val(&make, make.acts.cnt, MVAR(0)), 1);
	EXPECT_EQ(make_ext_set_val(&make, var, MVAR(0)), 1);
	log_set_quiet(0, 0);
	mem_oom(1);
	EXPECT_EQ(make_ext_set_val(&make, ext, MVAR(0)), 1);
	mem_oom(0);
	EXPECT_EQ(make_ext_set_val(&make, ext, MVAR(0)), 0);
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

	make_act_t ext;

	make_var_ext(&make, STRV("EXT45678"), &ext);

	mem_oom(1);
	EXPECT_EQ(make_ext_set_val(&make, ext, MSTR(STRV("a"))), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_eval)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char b[16] = {0};
	str_t buf  = STRB(b, 0);

	EXPECT_EQ(make_eval(NULL, make.acts.cnt, NULL), 1);
	EXPECT_EQ(make_eval(&make, make.acts.cnt, &buf), 0);

	make_free(&make);

	END;
}

TEST(make_eval_loop)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t empty;
	make_empty(&make, &empty);
	list_app(&make.acts, empty, empty);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval(&make, empty, &tmp), 1);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_eval_replace)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 1, ALLOC_STD);

	make_act_t root, act;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &root);
	make_var_add_val(&make, root, MSTR(STRV("$(ABC")));
	make_var_var(&make, root, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("$(ABC)")));
	make_add_act(&make, root, act);
	make_var(&make, STRV("A"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")));
	make_add_act(&make, root, act);
	make_var_var(&make, act, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("$(A) $(A) $(A) $(A) $(A) $(A) $(A) $(A)")));
	make_add_act(&make, root, act);

	char buf[512] = {0};
	str_t tmp     = STRB(buf, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval(&make, root, &tmp), 1);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_eval_oom_add_var)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t var;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);

	make.strs_used = make.strs.used = make.strs.size;

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	mem_oom(1);
	EXPECT_EQ(make_eval(&make, var, &tmp), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_eval_oom_var_inst)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_act_t var;
	make_var(&make, STRV(""), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("a")));

	make.strs_used = make.strs.used = make.strs.size - sizeof(size_t) * 2;

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	mem_oom(1);
	EXPECT_EQ(make_eval(&make, var, &tmp), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_eval_oom_var_append)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t act;

	make_var(&make, STRV(""), MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MSTR(STRV("a")));

	make.strs_used = make.strs.used = make.strs.size - sizeof(size_t) * 2;

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	mem_oom(1);
	EXPECT_EQ(make_eval(&make, act, &tmp), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_eval_oom_if)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t mif;

	make_if(&make, MSTR(STRV("L")), MSTR(STRV("R")), &mif);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	mem_oom(1);
	EXPECT_EQ(make_eval(&make, mif, &tmp), 1);
	mem_oom(0);

	make_free(&make);

	END;
}

TEST(make_eval_invalid_eval_def)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t def, eval;

	make_def(&make, STRV_NULL, &def);
	make_eval_def(&make, def, &eval);
	make_add_act(&make, def, eval);

	make_act_data_t *data  = list_get(&make.acts, eval);
	data->val.eval_def.def = make.acts.cnt;

	char buf[64] = {0};
	str_t tmp    = STRB(buf, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval(&make, def, &tmp), 1);
	EXPECT_EQ(make_print(&make, def, DST_BUF(buf)), 14);
	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 17);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_eval_var_type)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t var;

	make_var(&make, STRV("VAR"), 2, &var);
	make_var_add_val(&make, var, MSTR(STRV("A")));

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, var, &tmp), 1);

	make_free(&make);

	END;
}

TEST(make_eval_ext_not_set)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t ext;

	make_var_ext(&make, STRV("EXT"), &ext);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);
	EXPECT_EQ(make_eval(&make, ext, &tmp), 0);

	strv_t exp, res;

	exp = make_get_expanded(&make, ext);
	EXPECT_STRN(exp.data, "", exp.len);
	res = make_get_resolved(&make, ext, &tmp);
	EXPECT_STRN(res.data, "", res.len);

	make_free(&make);

	END;
}

TEST(make_get_expanded)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_get_expanded(NULL, 0).data, NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_get_expanded(&make, 0).data, NULL);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_get_resolved)
{
	START;

	make_t make = {0};
	log_set_quiet(0, 1);
	make_init(&make, 0, 0, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(make_get_resolved(NULL, 0, NULL).data, NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_get_resolved(&make, 0, NULL).data, NULL);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_get_resolved_null_buf)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t ext;
	make_var_ext(&make, STRV(""), &ext);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);
	make_eval(&make, ext, &tmp);

	str_t null = STR_NULL;
	EXPECT_EQ(make_get_resolved(&make, ext, &null).data, NULL);

	make_free(&make);

	END;
}

TEST(make_inc_print)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[8] = {0};
	EXPECT_EQ(make_inc_print(NULL, make.acts.cnt, DST_BUF(buf)), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(make_inc_print(&make, make.acts.cnt, DST_BUF(buf)), 0);
	log_set_quiet(0, 0);

	make_act_t inc, empty;

	make_inc(&make, STRV(""), &inc);
	make_empty(&make, &empty);
	make_inc_add_act(&make, inc, empty);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_inc_print(&make, empty, DST_BUF(buf)), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(make_inc_print(&make, inc, DST_BUF(buf)), 1);

	make_free(&make);

	END;
}

TEST(make_print)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[8] = {0};
	EXPECT_EQ(make_print(NULL, make.acts.cnt, DST_BUF(buf)), 0);
	EXPECT_EQ(make_print(&make, make.acts.cnt, DST_BUF(buf)), 0);

	make_act_t root, var;

	make_empty(&make, &root);

	EXPECT_EQ(make_print(&make, root, DST_BUF(buf)), 1);

	make_var(&make, STRV(""), 2, &var);
	make_add_act(&make, root, var);
	EXPECT_EQ(make_print(&make, root, DST_BUF(buf)), 1);

	make_free(&make);

	END;
}

TEST(make_print_loop)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[8] = {0};

	make_act_t empty;

	make_empty(&make, &empty);
	list_app(&make.acts, empty, empty);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_print(&make, empty, DST_BUF(buf)), 1);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_dbg)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[8] = {0};
	EXPECT_EQ(make_dbg(NULL, DST_BUF(buf)), 0);
	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 0);

	make_act_t empty;

	make_empty(&make, &empty);

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 6);

	make_free(&make);

	END;
}

TEST(make_dbg_var_loop)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[128] = {0};

	make_act_t var;

	make_var(&make, STRV(""), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("")));
	list_app(&make.arrs, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 42);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_dbg_rule_loop)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[128] = {0};

	make_act_t rule;

	make_rule(&make, MRULE(MSTR(STRV(""))), 0, &rule);
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRV(""))));
	list_app(&make.targets, 0, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 40);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_dbg_eval_def_loop)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	char buf[128] = {0};

	make_act_t def, eval;

	make_def(&make, STRV(""), &def);
	make_eval_def(&make, def, &eval);
	list_app(&make.arrs, 0, 0);
	make_add_act(&make, def, eval);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 57);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_eval_print_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t empty;

	make_empty(&make, &empty);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, empty, &tmp), 0);

	EXPECT_EQ(make_print(&make, empty, DST_BUF(buf)), 1);
	EXPECT_STR(buf, "\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 6);

	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t var;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);

	char buf[128] = {0};
	str_t tmp     = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, var, &tmp), 0);
	strv_t var_exp = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp.data, "", var_exp.len);

	EXPECT_EQ(make_print(&make, var, DST_BUF(buf)), 7);
	EXPECT_STR(buf, "VAR :=\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 36);

	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_act_t var;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL")));

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, var, &tmp), 0);
	strv_t var_exp = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp.data, "VAL", var_exp.len);

	EXPECT_EQ(make_print(&make, var, DST_BUF(buf)), 11);
	EXPECT_STR(buf, "VAR := VAL\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst2)
{
	START;

	make_t make = {0};
	make_init(&make, 3, 3, 1, 4, ALLOC_STD);

	make_act_t var;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL1")));
	make_var_add_val(&make, var, MSTR(STRV("VAL2")));

	char buf[32] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, var, &tmp), 0);
	strv_t var_exp = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp.data, "VAL1 VAL2", var_exp.len);

	EXPECT_EQ(make_print(&make, var, DST_BUF(buf)), 17);
	EXPECT_STR(buf, "VAR := VAL1 VAL2\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_var_inst_out)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_act_t act, b;

	make_var(&make, STRV("A"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("V")));
	make_var(&make, STRV("B"), MAKE_VAR_INST, &b);
	make_var_add_val(&make, b, MSTR(STRV("$$(A)")));
	make_add_act(&make, act, b);

	char buf[32] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, act, &tmp), 0);
	strv_t var_exp = make_get_expanded(&make, b);
	EXPECT_STRN(var_exp.data, "$$(A)", var_exp.len);

	strv_t var_res = make_get_resolved(&make, b, &tmp);
	EXPECT_STRN(var_res.data, "$(A)", var_res.len);

	EXPECT_EQ(make_print(&make, act, DST_BUF(buf)), 18);
	EXPECT_STR(buf,
		   "A := V\n"
		   "B := $$(A)\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_var_ref)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_act_t var;

	make_var(&make, STRV("VAR"), MAKE_VAR_REF, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL")));

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, var, &tmp), 0);
	strv_t var_exp = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp.data, "VAL", var_exp.len);

	EXPECT_EQ(make_print(&make, var, DST_BUF(buf)), 10);
	EXPECT_STR(buf, "VAR = VAL\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_var_app)
{
	START;

	make_t make = {0};
	make_init(&make, 2, 2, 1, 4, ALLOC_STD);

	make_act_t var, act;
	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL1")));
	make_var_var(&make, var, MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAL2")));
	make_add_act(&make, var, act);

	char buf[256] = {0};
	str_t tmp     = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, var, &tmp), 0);
	strv_t var_exp = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp.data, "VAL1 VAL2", var_exp.len);

	EXPECT_EQ(make_print(&make, var, DST_BUF(buf)), 24);
	EXPECT_STR(buf,
		   "VAR := VAL1\n"
		   "VAR += VAL2\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 98);

	make_free(&make);

	END;
}

TEST(make_eval_print_var_ext_inst)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_act_t ext;

	make_var_ext(&make, STRV("VAR"), &ext);

	make_ext_set_val(&make, ext, MSTR(STRV("VAL")));

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, ext, &tmp), 0);
	strv_t var_exp = make_get_expanded(&make, ext);
	EXPECT_STRN(var_exp.data, "VAL", var_exp.len);

	EXPECT_EQ(make_print(&make, ext, DST_BUF(buf)), 0);

	make_free(&make);

	END;
}

TEST(make_eval_print_var_ext)
{
	START;

	make_t make = {0};
	make_init(&make, 8, 8, 1, 16, ALLOC_STD);

	make_act_t root, ext2, ext3, ext4, var, var_app, out;

	make_var_ext(&make, STRV("EXT1"), &root);
	make_var_ext(&make, STRV("EXT2"), &ext2);
	make_add_act(&make, root, ext2);
	make_var_ext(&make, STRV("EXT3"), &ext3);
	make_add_act(&make, root, ext3);
	make_var_ext(&make, STRV("EXT4"), &ext4);
	make_add_act(&make, root, ext4);

	make_var(&make, STRV("VAR"), MAKE_VAR_APP, &var);
	make_add_act(&make, root, var);
	make_var_var(&make, var, MAKE_VAR_APP, &var_app);
	make_add_act(&make, root, var_app);

	make_var(&make, STRV("OUT"), MAKE_VAR_INST, &out);
	make_add_act(&make, root, out);

	make_var_add_val(&make, var, MVAR(root));
	make_var_add_val(&make, var, MVAR(ext2));
	make_var_add_val(&make, var_app, MVAR(ext3));
	make_var_add_val(&make, var_app, MVAR(ext4));

	make_var_add_val(&make, out, MVAR(var));

	make_ext_set_val(&make, root, MSTR(STRV("VAL1")));
	make_ext_set_val(&make, ext2, MSTR(STRV("VAL2")));
	make_ext_set_val(&make, ext3, MSTR(STRV("VAL3")));
	make_ext_set_val(&make, ext4, MSTR(STRV("VAL4")));

	char buf[64] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, root, &tmp), 0);

	strv_t out_exp = make_get_expanded(&make, out);
	EXPECT_STRN(out_exp.data, "$(VAR)", out_exp.len);
	strv_t out_res = make_get_resolved(&make, out, &tmp);
	EXPECT_STRN(out_res.data, "VAL1 VAL2 VAL3 VAL4", out_res.len);

	EXPECT_EQ(make_print(&make, root, DST_BUF(buf)), 60);
	EXPECT_STR(buf,
		   "VAR += $(EXT1) $(EXT2)\n"
		   "VAR += $(EXT3) $(EXT4)\n"
		   "OUT := $(VAR)\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_if_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t mif;

	make_if(&make, MSTR(STRV_NULL), MSTR(STRV_NULL), &mif);

	char buf[64] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, mif, &tmp), 0);

	EXPECT_EQ(make_print(&make, mif, DST_BUF(buf)), 15);
	EXPECT_STR(buf,
		   "ifeq (,)\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 23);
	EXPECT_STR(buf,
		   "IF\n"
		   "    L: ''\n"
		   "    R: ''\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_if_lr)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 4, ALLOC_STD);

	make_act_t mif;

	make_if(&make, MSTR(STRV("L")), MSTR(STRV("R")), &mif);

	char buf[64] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, mif, &tmp), 0);

	EXPECT_EQ(make_print(&make, mif, DST_BUF(buf)), 17);
	EXPECT_STR(buf,
		   "ifeq (L,R)\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 25);
	EXPECT_STR(buf,
		   "IF\n"
		   "    L: 'L'\n"
		   "    R: 'R'\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_var_if_true)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t cond, if_cond, var, act;

	make_var_ext(&make, STRV("COND"), &cond);
	make_if(&make, MVAR(cond), MSTR(STRV("A")), &if_cond);
	make_add_act(&make, cond, if_cond);
	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL")));
	make_if_add_true_act(&make, if_cond, var);
	make_var(&make, STRV("VAR2"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAL2")));
	make_if_add_true_act(&make, if_cond, act);

	char buf[512] = {0};
	str_t tmp     = STRB(buf, 0);

	make_ext_set_val(&make, cond, MSTR(STRV("A")));
	EXPECT_EQ(make_eval(&make, cond, &tmp), 0);
	strv_t var_exp_a = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp_a.data, "VAL", var_exp_a.len);

	make_ext_set_val(&make, cond, MSTR(STRV("B")));
	EXPECT_EQ(make_eval(&make, cond, &tmp), 0);

	EXPECT_EQ(make_print(&make, cond, DST_BUF(buf)), 47);
	EXPECT_STR(buf,
		   "ifeq ($(COND),A)\n"
		   "VAR := VAL\n"
		   "VAR2 := VAL2\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 182);
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
		   "        VAL\n"
		   "VAR\n"
		   "    NAME    : VAR2\n"
		   "    VALUES  :\n"
		   "        VAL2\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_var_if_false)
{
	START;

	make_t make = {0};
	make_init(&make, 6, 6, 1, 8, ALLOC_STD);

	make_act_t cond, if_cond, var, act;

	make_var_ext(&make, STRV("COND"), &cond);
	make_if(&make, MVAR(cond), MSTR(STRV("A")), &if_cond);
	make_add_act(&make, cond, if_cond);
	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL1")));
	make_if_add_true_act(&make, if_cond, var);
	make_var_var(&make, var, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAL2")));
	make_if_add_false_act(&make, if_cond, act);

	char buf[512] = {0};
	str_t tmp     = STRB(buf, 0);

	make_ext_set_val(&make, cond, MSTR(STRV("A")));
	EXPECT_EQ(make_eval(&make, cond, &tmp), 0);
	strv_t var_exp_a = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp_a.data, "VAL1", var_exp_a.len);

	make_ext_set_val(&make, cond, MSTR(STRV("B")));
	EXPECT_EQ(make_eval(&make, cond, &tmp), 0);
	strv_t var_exp_b = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp_b.data, "VAL2", var_exp_b.len);

	EXPECT_EQ(make_print(&make, cond, DST_BUF(buf)), 52);
	EXPECT_STR(buf,
		   "ifeq ($(COND),A)\n"
		   "VAR := VAL1\n"
		   "else\n"
		   "VAR := VAL2\n"
		   "endif\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 182);
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

	make_free(&make);

	END;
}

TEST(make_eval_print_def_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 1, ALLOC_STD);

	make_act_t def;

	make_def(&make, STRV_NULL, &def);

	char buf[64] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, def, &tmp), 0);

	EXPECT_EQ(make_print(&make, def, DST_BUF(buf)), 14);
	EXPECT_STR(buf,
		   "define \n"
		   "endef\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 17);
	EXPECT_STR(buf,
		   "DEF\n"
		   "    NAME: ''\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_def)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t def, var;

	make_def(&make, STRV("def"), &def);
	make_var(&make, STRV("$(1)"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL")));
	make_def_add_act(&make, def, var);

	char buf[512] = {0};
	str_t tmp     = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, def, &tmp), 0);

	EXPECT_EQ(make_print(&make, def, DST_BUF(buf)), 29);
	EXPECT_STR(buf,
		   "define def\n"
		   "$(1) := VAL\n"
		   "endef\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 69);
	EXPECT_STR(buf,
		   "DEF\n"
		   "    NAME: 'def'\n"
		   "VAR\n"
		   "    NAME    : $(1)\n"
		   "    VALUES  :\n"
		   "        VAL\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_def_no_arg)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t def, act, var;

	make_def(&make, STRV("def"), &def);
	make_var(&make, STRV("$(1)"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL")));
	make_def_add_act(&make, def, var);

	make_eval_def(&make, def, &act);
	make_add_act(&make, def, act);

	char buf[512] = {0};
	str_t tmp     = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, def, &tmp), 0);

	strv_t var_exp_a = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp_a.data, "VAL", var_exp_a.len);

	EXPECT_EQ(make_print(&make, def, DST_BUF(buf)), 49);
	EXPECT_STR(buf,
		   "define def\n"
		   "$(1) := VAL\n"
		   "endef\n"
		   "$(eval $(call def))\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 115);
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

	make_free(&make);

	END;
}

TEST(make_eval_print_def_args)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t def, var, eval;

	make_def(&make, STRV("def"), &def);
	make_var(&make, STRV("$(1)"), MAKE_VAR_INST, &var);
	make_def_add_act(&make, def, var);

	make_var_add_val(&make, var, MSTR(STRV("$(0)")));
	make_var_add_val(&make, var, MSTR(STRV("$(1)")));
	make_var_add_val(&make, var, MSTR(STRV("$(2)")));
	make_var_add_val(&make, var, MSTR(STRV("$(3)")));
	make_var_add_val(&make, var, MSTR(STRV("$(4)")));

	make_eval_def(&make, def, &eval);
	make_add_act(&make, def, eval);
	make_eval_def_add_arg(&make, eval, MSTR(STRV("VAR")));

	char buf[512] = {0};
	str_t tmp     = STRB(buf, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval(&make, def, &tmp), 1);
	log_set_quiet(0, 0);
	strv_t var_exp_a = make_get_expanded(&make, var);
	EXPECT_STRN(var_exp_a.data, "def VAR   ", var_exp_a.len);

	EXPECT_EQ(make_print(&make, def, DST_BUF(buf)), 74);
	EXPECT_STR(buf,
		   "define def\n"
		   "$(1) := $(0) $(1) $(2) $(3) $(4)\n"
		   "endef\n"
		   "$(eval $(call def,VAR))\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 180);
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

	make_free(&make);

	END;
}

TEST(make_eval_print_def_var_invalid)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t def, act, b;

	make_def(&make, STRV("def"), &def);

	make_var(&make, STRV("A"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("V")));
	make_def_add_act(&make, def, act);
	make_var(&make, STRV("B"), MAKE_VAR_INST, &b);
	make_var_add_val(&make, b, MSTR(STRV("$(A)")));
	make_def_add_act(&make, def, b);

	make_eval_def(&make, def, &act);
	make_add_act(&make, def, act);

	char buf[512] = {0};
	str_t tmp     = STRB(buf, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval(&make, def, &tmp), 1);
	log_set_quiet(0, 0);

	strv_t var_exp_a = make_get_expanded(&make, b);
	EXPECT_STRN(var_exp_a.data, "", var_exp_a.len);

	EXPECT_EQ(make_print(&make, def, DST_BUF(buf)), 54);
	EXPECT_STR(buf,
		   "define def\n"
		   "A := V\n"
		   "B := $(A)\n"
		   "endef\n"
		   "$(eval $(call def))\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 157);
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

	make_free(&make);

	END;
}

TEST(make_eval_print_def_var_imm)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t root, act, def, b;

	make_var(&make, STRV("A"), MAKE_VAR_INST, &root);
	make_var_add_val(&make, root, MSTR(STRV("V")));

	make_def(&make, STRV("def"), &def);
	make_add_act(&make, root, def);
	make_var(&make, STRV("B"), MAKE_VAR_INST, &b);
	make_var_add_val(&make, b, MSTR(STRV("$(A)")));
	make_def_add_act(&make, def, b);

	make_eval_def(&make, def, &act);
	make_add_act(&make, root, act);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, root, &tmp), 0);

	strv_t exp, res;

	exp = make_get_expanded(&make, b);
	EXPECT_STRN(exp.data, "V", exp.len);
	res = make_get_expanded(&make, b);
	EXPECT_STRN(res.data, "V", res.len);

	make_free(&make);

	END;
}

TEST(make_eval_print_def_var)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t def, a, b, act;

	make_def(&make, STRV("def"), &def);

	make_var(&make, STRV("A"), MAKE_VAR_INST, &a);
	make_var_add_val(&make, a, MSTR(STRV("V")));
	make_def_add_act(&make, def, a);

	make_var(&make, STRV("B"), MAKE_VAR_INST, &b);
	make_var_add_val(&make, b, MVAR(a));
	make_def_add_act(&make, def, b);
	make_var(&make, STRV("C"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("$$(A)")));
	make_def_add_act(&make, def, act);

	make_rule(&make, MRULE(MVAR(a)), 1, &act);
	make_def_add_act(&make, def, act);

	make_eval_def(&make, def, &act);
	make_add_act(&make, def, act);

	char buf[512] = {0};
	str_t tmp     = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, def, &tmp), 0);

	strv_t val;

	val = make_get_expanded(&make, b);
	EXPECT_STRN(val.data, "$(A)", val.len);
	val = make_get_resolved(&make, b, &tmp);
	EXPECT_STRN(val.data, "V", val.len);

	val = make_get_expanded(&make, b);
	EXPECT_STRN(val.data, "$(A)", val.len);
	val = make_get_resolved(&make, b, &tmp);
	EXPECT_STRN(val.data, "V", val.len);

	EXPECT_EQ(make_print(&make, def, DST_BUF(buf)), 74);
	EXPECT_STR(buf,
		   "define def\n"
		   "A := V\n"
		   "B := $$(A)\n"
		   "C := $$(A)\n"
		   "$$(A):\n"
		   "\n"
		   "endef\n"
		   "$(eval $(call def))\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 242);
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

	make_free(&make);

	END;
}

TEST(make_eval_print_var_not_found)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t act;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MVAR(make.acts.cnt));

	char buf[256] = {0};
	str_t tmp     = STRB(buf, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(make_eval(&make, act, &tmp), 1);

	EXPECT_EQ(make_print(&make, act, DST_BUF(buf)), 8);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_eval_print_inc)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t inc, var, act;

	make_inc(&make, STRV("inc.mk"), &inc);
	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL")));
	make_inc_add_act(&make, inc, var);
	make_var(&make, STRV("VAR2"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAL2")));
	make_inc_add_act(&make, inc, act);

	char buf[128] = {0};
	str_t tmp     = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, inc, &tmp), 0);

	strv_t val;

	val = make_get_expanded(&make, var);
	EXPECT_STRN(val.data, "VAL", val.len);
	val = make_get_resolved(&make, var, &tmp);
	EXPECT_STRN(val.data, "VAL", val.len);

	EXPECT_EQ(make_print(&make, inc, DST_BUF(buf)), 15);
	EXPECT_STR(buf, "include inc.mk\n");

	EXPECT_EQ(make_inc_print(&make, inc, DST_BUF(buf)), 24);
	EXPECT_STR(buf,
		   "VAR := VAL\n"
		   "VAR2 := VAL2\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 125);
	EXPECT_STR(buf,
		   "INCLUDE\n"
		   "    PATH: 'inc.mk'\n"
		   "VAR\n"
		   "    NAME    : VAR\n"
		   "    VALUES  :\n"
		   "        VAL\n"
		   "VAR\n"
		   "    NAME    : VAR2\n"
		   "    VALUES  :\n"
		   "        VAL2\n");

	make_free(&make);

	END;
}

TEST(make_eval_print_inc_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t act;

	make_inc(&make, STRV("inc.mk"), &act);

	char buf[128] = {0};
	str_t tmp     = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, act, &tmp), 0);

	EXPECT_EQ(make_print(&make, act, DST_BUF(buf)), 15);
	EXPECT_STR(buf, "include inc.mk\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 27);
	EXPECT_STR(buf,
		   "INCLUDE\n"
		   "    PATH: 'inc.mk'\n");

	make_free(&make);

	END;
}

TEST(make_eval_ext_override)
{
	START;

	make_t make = {0};
	make_init(&make, 3, 3, 1, 8, ALLOC_STD);

	make_act_t act, ext;

	make_var_ext(&make, STRV("EXT"), &ext);

	make_var_var(&make, ext, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAL")));
	make_add_act(&make, ext, act);
	make_var_var(&make, ext, MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAL")));
	make_add_act(&make, ext, act);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	{
		make_eval(&make, ext, &tmp);
		strv_t exp = make_get_expanded(&make, ext);
		EXPECT_STRN(exp.data, "VAL VAL", exp.len);
		strv_t res = make_get_resolved(&make, ext, &tmp);
		EXPECT_STRN(res.data, "VAL VAL", res.len);
	}
	{
		make_ext_set_val(&make, ext, MSTR(STRV("EXT")));
		make_eval(&make, ext, &tmp);
		strv_t exp = make_get_expanded(&make, ext);
		EXPECT_STRN(exp.data, "EXT", exp.len);
		strv_t res = make_get_resolved(&make, ext, &tmp);
		EXPECT_STRN(res.data, "EXT", res.len)
	}

	make_free(&make);

	END;
}

TEST(make_eval_inst_app)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t var, t, act;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("A")));
	make_var(&make, STRV("T"), MAKE_VAR_INST, &t);
	make_var_add_val(&make, t, MVAR(var));
	make_add_act(&make, var, t);
	make_var_var(&make, t, MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MVAR(var));
	make_add_act(&make, var, act);

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("B")));
	make_add_act(&make, var, act);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	make_eval(&make, var, &tmp);

	strv_t exp = make_get_expanded(&make, t);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR)", exp.len);
	strv_t res = make_get_resolved(&make, t, &tmp);
	EXPECT_STRN(res.data, "A A", res.len);

	make_free(&make);

	END;
}

TEST(make_eval_ref_app)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t var, t, act;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("A")));
	make_var(&make, STRV("T"), MAKE_VAR_REF, &t);
	make_var_add_val(&make, t, MVAR(var));
	make_add_act(&make, var, t);
	make_var_var(&make, t, MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MVAR(var));
	make_add_act(&make, var, act);

	make_var_var(&make, var, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("B")));
	make_add_act(&make, var, act);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	make_eval(&make, var, &tmp);

	strv_t exp = make_get_expanded(&make, t);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR)", exp.len);
	strv_t res = make_get_resolved(&make, t, &tmp);
	EXPECT_STRN(res.data, "B B", res.len);

	make_free(&make);

	END;
}

TEST(make_eval_inst_if)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t var, t, act, r, if_cond;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("A")));
	make_var(&make, STRV("T"), MAKE_VAR_INST, &t);
	make_var_add_val(&make, t, MVAR(var));
	make_add_act(&make, var, t);

	make_var_var(&make, var, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("B")));
	make_add_act(&make, var, act);

	make_if(&make, MVAR(t), MSTR(STRV("A")), &if_cond);
	make_add_act(&make, var, if_cond);
	make_var(&make, STRV("R"), MAKE_VAR_INST, &r);
	make_var_add_val(&make, r, MSTR(STRV("C")));
	make_if_add_true_act(&make, if_cond, r);
	make_var_var(&make, r, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("D")));
	make_if_add_false_act(&make, if_cond, act);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	make_eval(&make, var, &tmp);

	strv_t exp = make_get_expanded(&make, r);
	EXPECT_STRN(exp.data, "C", exp.len);
	strv_t res = make_get_resolved(&make, r, &tmp);
	EXPECT_STRN(res.data, "C", res.len);

	make_free(&make);

	END;
}

TEST(make_eval_ref_if)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t var, t, act, r, if_cond;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("A")));
	make_var(&make, STRV("T"), MAKE_VAR_REF, &t);
	make_var_add_val(&make, t, MVAR(var));
	make_add_act(&make, var, t);

	make_var_var(&make, var, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("B")));
	make_add_act(&make, var, act);

	make_if(&make, MVAR(t), MSTR(STRV("A")), &if_cond);
	make_add_act(&make, var, if_cond);
	make_var(&make, STRV("R"), MAKE_VAR_INST, &r);
	make_var_add_val(&make, r, MSTR(STRV("C")));
	make_if_add_true_act(&make, if_cond, r);
	make_var_var(&make, r, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("D")));
	make_if_add_false_act(&make, if_cond, act);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	make_eval(&make, var, &tmp);

	strv_t exp = make_get_expanded(&make, r);
	EXPECT_STRN(exp.data, "D", exp.len);
	strv_t res = make_get_resolved(&make, r, &tmp);
	EXPECT_STRN(res.data, "D", res.len);

	make_free(&make);

	END;
}

TEST(make_eval_name)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t act, var;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("A")));
	make_var(&make, STRV("$(VAR)_VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL")));
	make_add_act(&make, act, var);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	make_eval(&make, act, &tmp);

	strv_t exp = make_get_expanded(&make, var);
	EXPECT_STRN(exp.data, "VAL", exp.len);
	strv_t res = make_get_resolved(&make, var, &tmp);
	EXPECT_STRN(res.data, "VAL", res.len);

	make_free(&make);

	END;
}

TEST(make_eval_def_name)
{
	START;

	make_t make = {0};
	make_init(&make, 4, 4, 1, 8, ALLOC_STD);

	make_act_t root, act, def, var;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &root);
	make_var_add_val(&make, root, MSTR(STRV("A")));

	make_def(&make, STRV("def"), &def);
	make_add_act(&make, root, def);

	make_var(&make, STRV("$(VAR)_VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("VAL")));
	make_def_add_act(&make, def, var);

	make_eval_def(&make, def, &act);
	make_add_act(&make, root, act);

	char buf[16] = {0};
	str_t tmp    = STRB(buf, 0);

	make_eval(&make, root, &tmp);

	strv_t exp = make_get_expanded(&make, var);
	EXPECT_STRN(exp.data, "VAL", exp.len);
	strv_t res = make_get_resolved(&make, var, &tmp);
	EXPECT_STRN(res.data, "VAL", res.len);

	make_free(&make);

	END;
}

TEST(make_print_vars)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t act;

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("A")));

	char buf[256] = {0};
	str_t tmp     = STRB(buf, 0);

	make_eval(&make, act, &tmp);

	make_print_vars(&make, DST_BUF(buf));
	EXPECT_STR(buf, "VAR              A                                                                A\n");

	make_free(&make);

	END;
}

TEST(make_print_rule_empty)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t act;

	make_rule(&make, MRULE(MSTR(STRV("rule"))), 1, &act);

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, act, DST_BUF(buf)), 7);
	EXPECT_STR(buf,
		   "rule:\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 35);

	make_free(&make);

	END;
}

TEST(make_print_rule_empty_var)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t act;

	make_rule(&make, MRULE(MVAR(0)), 1, &act);

	char buf[64] = {0};
	log_set_quiet(0, 1);
	EXPECT_EQ(make_print(&make, act, DST_BUF(buf)), 3);
	EXPECT_STR(buf,
		   ":\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 31);
	log_set_quiet(0, 0);

	make_free(&make);

	END;
}

TEST(make_print_rule_empty_action)
{
	START;

	make_t make = {0};
	make_init(&make, 1, 1, 1, 2, ALLOC_STD);

	make_act_t act;

	make_rule(&make, MRULEACT(MSTR(STRV("rule")), STRV("/action")), 1, &act);

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, act, DST_BUF(buf)), 14);
	EXPECT_STR(buf,
		   "rule/action:\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 42);

	make_free(&make);

	END;
}

TEST(make_print_rule_depend)
{
	START;

	make_t make = {0};
	make_init(&make, 2, 2, 1, 8, ALLOC_STD);

	make_act_t rule;

	make_rule(&make, MRULE(MSTR(STRV("rule"))), 1, &rule);
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("depend"))));

	char buf[64] = {0};
	EXPECT_EQ(make_print(&make, rule, DST_BUF(buf)), 14);
	EXPECT_STR(buf,
		   "rule: depend\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 50);

	make_free(&make);

	END;
}

TEST(make_print_rule_depends)
{
	START;

	make_t make = {0};
	make_init(&make, 3, 3, 1, 8, ALLOC_STD);

	make_act_t rule;

	make_rule(&make, MRULE(MSTR(STRV("rule"))), 1, &rule);
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("depend1"))));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRV("depend2"))));

	char buf[128] = {0};
	EXPECT_EQ(make_print(&make, rule, DST_BUF(buf)), 23);
	EXPECT_STR(buf,
		   "rule: depend1 depend2\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 67);

	make_free(&make);

	END;
}

TEST(make_print_rule_acts)
{
	START;

	make_t make = {0};
	make_init(&make, 6, 6, 1, 8, ALLOC_STD);

	make_act_t rule, act, if_rule;

	make_rule(&make, MRULE(MSTR(STRV("rule"))), 1, &rule);
	make_cmd(&make, MCMD(STRV("cmd1")), &act);
	make_rule_add_act(&make, rule, act);
	make_cmd(&make, MCMD(STRV("cmd2")), &act);
	make_rule_add_act(&make, rule, act);

	make_if(&make, MSTR(STRV("L")), MSTR(STRV("R")), &if_rule);
	make_rule_add_act(&make, rule, if_rule);
	make_cmd(&make, MCMD(STRV("cmd3")), &act);
	make_if_add_true_act(&make, if_rule, act);
	make_cmd(&make, MCMD(STRV("cmd4")), &act);
	make_if_add_false_act(&make, if_rule, act);

	char buf[512] = {0};
	EXPECT_EQ(make_print(&make, rule, DST_BUF(buf)), 53);
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

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 228);
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

	make_act_t rule, act;

	make_rule(&make, MRULE(MSTR(STRV("rule"))), 1, &rule);
	make_cmd(&make, MCMD(STRV("cmd")), &act);
	make_rule_add_act(&make, rule, act);
	make_cmd(&make, MCMDCHILD(STRV("dir"), STRV_NULL), &act);
	make_rule_add_act(&make, rule, act);
	make_cmd(&make, MCMDCHILD(STRV("dir"), STRV("action")), &act);
	make_rule_add_act(&make, rule, act);
	make_cmd(&make, MCMDERR(STRV("msg")), &act);
	make_rule_add_act(&make, rule, act);

	char buf[256] = {0};
	EXPECT_EQ(make_print(&make, rule, DST_BUF(buf)), 67);
	EXPECT_STR(buf,
		   "rule:\n"
		   "\tcmd\n"
		   "\t@$(MAKE) -C dir\n"
		   "\t@$(MAKE) -C dir action\n"
		   "\t$(error msg)\n"
		   "\n");

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 205);

	make_free(&make);

	END;
}

TEST(make_eval_print)
{
	SSTART;
	RUN(make_ext_set_val);
	RUN(make_ext_set_val_oom_val);
	RUN(make_eval);
	RUN(make_eval_loop);
	RUN(make_eval_replace);
	RUN(make_eval_oom_add_var);
	RUN(make_eval_oom_var_inst);
	RUN(make_eval_oom_var_append);
	RUN(make_eval_oom_if);
	RUN(make_eval_invalid_eval_def);
	RUN(make_eval_var_type);
	RUN(make_eval_ext_not_set);
	RUN(make_get_expanded);
	RUN(make_get_resolved);
	RUN(make_get_resolved_null_buf);
	RUN(make_inc_print);
	RUN(make_print);
	RUN(make_print_loop);
	RUN(make_dbg);
	RUN(make_dbg_var_loop);
	RUN(make_dbg_rule_loop);
	RUN(make_dbg_eval_def_loop);
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
	RUN(make_print_vars);
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

	make_act_t root, ext_empty, ext_set, ext_var;
	make_act_t var_ext_not, var_ext_empty, var_ext_set;
	make_act_t act, var, imm, rec, app_imm, app_rec, def, all;
	make_act_t def_var, def_var_imm, def_var_imm_var, def_imm, def_imm_esc, def_imm_var, def_rec, def_rec_esc, def_rec_esc_var;

	make_var_ext(&make, STRV("EXT_NOT"), &root);
	make_var_ext(&make, STRV("EXT_EMPTY"), &ext_empty);
	make_add_act(&make, root, ext_empty);
	make_var_ext(&make, STRV("EXT_SET"), &ext_set);
	make_add_act(&make, root, ext_set);
	make_var_ext(&make, STRV("EXT_VAR"), &ext_var);
	make_add_act(&make, root, ext_var);

	make_var_ext(&make, STRV("VAR_EXT_NOT"), &var_ext_not);
	make_add_act(&make, root, var_ext_not);
	make_var_ext(&make, STRV("VAR_EXT_EMPTY"), &var_ext_empty);
	make_add_act(&make, root, var_ext_empty);
	make_var_ext(&make, STRV("VAR_EXT_SET"), &var_ext_set);
	make_add_act(&make, root, var_ext_set);

	make_var(&make, STRV("VAR"), MAKE_VAR_INST, &var);
	make_var_add_val(&make, var, MSTR(STRV("V1")));
	make_add_act(&make, root, var);
	make_var(&make, STRV("IMM"), MAKE_VAR_INST, &imm);
	make_var_add_val(&make, imm, MSTR(STRV("$(VAR)")));
	make_add_act(&make, root, imm);
	make_var(&make, STRV("REC"), MAKE_VAR_REF, &rec);
	make_var_add_val(&make, rec, MSTR(STRV("$(VAR)")));
	make_add_act(&make, root, rec);
	make_var_var(&make, var, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("V2")));
	make_add_act(&make, root, act);

	make_var(&make, STRV("APP_IMM"), MAKE_VAR_INST, &app_imm);
	make_var_add_val(&make, app_imm, MVAR(var));
	make_add_act(&make, root, app_imm);
	make_var_var(&make, app_imm, MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MSTR(STRV("$(VAR)")));
	make_add_act(&make, root, act);
	make_var_var(&make, act, MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MVAR(var));
	make_add_act(&make, root, act);

	make_var(&make, STRV("APP_REC"), MAKE_VAR_REF, &app_rec);
	make_var_add_val(&make, app_rec, MVAR(var));
	make_add_act(&make, root, app_rec);
	make_var_var(&make, app_rec, MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MSTR(STRV("$(VAR)")));
	make_add_act(&make, root, act);
	make_var_var(&make, act, MAKE_VAR_APP, &act);
	make_var_add_val(&make, act, MVAR(var));
	make_add_act(&make, root, act);

	make_empty(&make, &act);
	make_add_act(&make, root, act);

	make_var_var(&make, var_ext_not, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAR")));
	make_add_act(&make, root, act);
	make_var_var(&make, var_ext_empty, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAR")));
	make_add_act(&make, root, act);
	make_var_var(&make, var_ext_set, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("VAR")));
	make_add_act(&make, root, act);

	make_empty(&make, &act);
	make_add_act(&make, root, act);

	make_def(&make, STRV("def"), &def);
	make_add_act(&make, root, def);
	make_var(&make, STRV("DEF_VAR"), MAKE_VAR_INST, &def_var);
	make_var_add_val(&make, def_var, MSTR(STRV("D")));
	make_def_add_act(&make, def, def_var);
	make_var(&make, STRV("DEF_VAR_IMM"), MAKE_VAR_INST, &def_var_imm);
	make_var_add_val(&make, def_var_imm, MSTR(STRV("$$(DEF_VAR)")));
	make_def_add_act(&make, def, def_var_imm);
	make_var(&make, STRV("DEF_VAR_IMM_VAR"), MAKE_VAR_INST, &def_var_imm_var);
	make_var_add_val(&make, def_var_imm_var, MVAR(def_var));
	make_def_add_act(&make, def, def_var_imm_var);
	make_var(&make, STRV("DEF_IMM"), MAKE_VAR_INST, &def_imm);
	make_var_add_val(&make, def_imm, MSTR(STRV("$(VAR)")));
	make_def_add_act(&make, def, def_imm);
	make_var(&make, STRV("DEF_IMM_VAR"), MAKE_VAR_INST, &def_imm_var);
	make_var_add_val(&make, def_imm_var, MVAR(var));
	make_def_add_act(&make, def, def_imm_var);
	make_var(&make, STRV("DEF_IMM_ESC"), MAKE_VAR_INST, &def_imm_esc);
	make_var_add_val(&make, def_imm_esc, MSTR(STRV("$$(VAR)")));
	make_def_add_act(&make, def, def_imm_esc);
	make_var(&make, STRV("DEF_REC"), MAKE_VAR_REF, &def_rec);
	make_var_add_val(&make, def_rec, MSTR(STRV("$(VAR)")));
	make_def_add_act(&make, def, def_rec);
	make_var(&make, STRV("DEF_REC_VAR"), MAKE_VAR_REF, &act);
	make_var_add_val(&make, act, MVAR(var));
	make_def_add_act(&make, def, act);
	make_var(&make, STRV("DEF_REC_ESC"), MAKE_VAR_REF, &def_rec_esc);
	make_var_add_val(&make, def_rec_esc, MSTR(STRV("$$(VAR)")));
	make_def_add_act(&make, def, def_rec_esc);
	make_var(&make, STRV("$(0)"), MAKE_VAR_INST, &def_rec_esc_var);
	make_var_add_val(&make, def_rec_esc_var, MSTR(STRV("$$($(1))")));
	make_def_add_act(&make, def, def_rec_esc_var);

	make_empty(&make, &act);
	make_add_act(&make, root, act);

	make_eval_def(&make, def, &act);
	make_eval_def_add_arg(&make, act, MSTR(STRV("VAR")));
	make_add_act(&make, root, act);

	make_empty(&make, &act);
	make_add_act(&make, root, act);

	make_var_var(&make, var, MAKE_VAR_INST, &act);
	make_var_add_val(&make, act, MSTR(STRV("V3")));
	make_add_act(&make, root, act);

	make_empty(&make, &act);
	make_add_act(&make, root, act);

	make_rule(&make, MRULE(MSTR(STRV("all"))), 1, &all);
	make_add_act(&make, root, all);
	make_cmd(&make, MCMD(STRV("@echo VAR             = $(VAR)           #V3")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo IMM             = $(IMM)           #V1")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo REC             = $(REC)           #V3")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo APP_IMM         = $(APP_IMM)       #V2 V2 V2")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo APP_REC         = $(APP_REC)       #V3 V3 V3")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo EXT_NOT         = $(EXT_NOT)       #")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo EXT_EMPTY       = $(EXT_EMPTY)     #")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo EXT_SET         = $(EXT_SET)       #EXT")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo EXT_VAR         = $(EXT_VAR)       #V3")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo VAR_EXT_NOT     = $(VAR_EXT_NOT)   #VAR")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo VAR_EXT_EMPTY   = $(VAR_EXT_EMPTY) #")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo VAR_EXT_SET     = $(VAR_EXT_SET)   #EXT")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_VAR         = $(DEF_VAR)       #D")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_VAR_IMM     = $(DEF_VAR_IMM)   #D")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_VAR_IMM_VAR = $(DEF_VAR_IMM_VAR) #D")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_IMM         = $(DEF_IMM)       #V2")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_IMM_VAR     = $(DEF_IMM_VAR)   #V2")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_IMM_ESC     = $(DEF_IMM_ESC)   #V2")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_REC         = $(DEF_REC)       #V2")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_REC_VAR     = $(DEF_REC_VAR)   #V3")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo DEF_REC_ESC     = $(DEF_REC_ESC)   #V3")), &act);
	make_rule_add_act(&make, all, act);
	make_cmd(&make, MCMD(STRV("@echo def             = $(def)           #V2")), &act);
	make_rule_add_act(&make, all, act);

	make_ext_set_val(&make, ext_empty, MSTR(STRV("")));
	make_ext_set_val(&make, ext_set, MSTR(STRV("EXT")));
	make_ext_set_val(&make, ext_var, MSTR(STRV("$(VAR)")));

	make_ext_set_val(&make, var_ext_empty, MSTR(STRV("")));
	make_ext_set_val(&make, var_ext_set, MSTR(STRV("EXT")));

	char buf[4096] = {0};
	EXPECT_EQ(make_print(&make, root, DST_BUF(buf)), 1506);
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

	EXPECT_EQ(make_dbg(&make, DST_BUF(buf)), 3676);

	str_t tmp = STRB(buf, 0);

	EXPECT_EQ(make_eval(&make, root, &tmp), 0);

	make_print_vars(&make, DST_BUF(buf));
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
		   "VAR                                                                               \n"
		   "APP_IMM          $(VAR) $(VAR) $(VAR)                                             V2 V2 V2\n"
		   "APP_IMM                                                                           \n"
		   "APP_IMM                                                                           \n"
		   "APP_REC          $(VAR) $(VAR) $(VAR)                                             $(VAR) $(VAR) $(VAR)\n"
		   "APP_REC                                                                           \n"
		   "APP_REC                                                                           \n"
		   "VAR_EXT_NOT                                                                       \n"
		   "VAR_EXT_EMPTY                                                                     \n"
		   "VAR_EXT_SET                                                                       \n"
		   "DEF_VAR          D                                                                D\n"
		   "DEF_VAR_IMM      $(DEF_VAR)                                                       D\n"
		   "DEF_VAR_IMM_VAR  $(DEF_VAR)                                                       D\n"
		   "DEF_IMM          V2                                                               V2\n"
		   "DEF_IMM_VAR      V2                                                               V2\n"
		   "DEF_IMM_ESC      $(VAR)                                                           V2\n"
		   "DEF_REC          V2                                                               V2\n"
		   "DEF_REC_VAR      V2                                                               V2\n"
		   "DEF_REC_ESC      $(VAR)                                                           $(VAR)\n"
		   "$(0)             $(VAR)                                                           V2\n"
		   "VAR                                                                               \n");

	strv_t exp, res;

	exp = make_get_expanded(&make, var);
	EXPECT_STRN(exp.data, "V3", exp.len);
	res = make_get_resolved(&make, var, &tmp);
	EXPECT_STRN(res.data, "V3", res.len);

	exp = make_get_expanded(&make, imm);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_get_resolved(&make, imm, &tmp);
	EXPECT_STRN(res.data, "V1", res.len);

	exp = make_get_expanded(&make, rec);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_get_resolved(&make, rec, &tmp);
	EXPECT_STRN(res.data, "V3", res.len);

	exp = make_get_expanded(&make, app_imm);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR) $(VAR)", exp.len);
	res = make_get_resolved(&make, app_imm, &tmp);
	EXPECT_STRN(res.data, "V2 V2 V2", res.len);

	exp = make_get_expanded(&make, app_rec);
	EXPECT_STRN(exp.data, "$(VAR) $(VAR) $(VAR)", exp.len);
	res = make_get_resolved(&make, app_rec, &tmp);
	EXPECT_STRN(res.data, "V3 V3 V3", res.len);

	exp = make_get_expanded(&make, root);
	EXPECT_STRN(exp.data, "", exp.len);
	res = make_get_resolved(&make, root, &tmp);
	EXPECT_STRN(res.data, "", res.len);

	exp = make_get_expanded(&make, ext_empty);
	EXPECT_STRN(exp.data, "", exp.len);
	res = make_get_resolved(&make, ext_empty, &tmp);
	EXPECT_STRN(res.data, "", res.len);

	exp = make_get_expanded(&make, ext_set);
	EXPECT_STRN(exp.data, "EXT", exp.len);
	res = make_get_resolved(&make, ext_set, &tmp);
	EXPECT_STRN(res.data, "EXT", res.len);

	exp = make_get_expanded(&make, ext_var);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_get_resolved(&make, ext_var, &tmp);
	EXPECT_STRN(res.data, "V3", res.len);

	exp = make_get_expanded(&make, var_ext_not);
	EXPECT_STRN(exp.data, "VAR", exp.len);
	res = make_get_resolved(&make, var_ext_not, &tmp);
	EXPECT_STRN(res.data, "VAR", res.len);

	exp = make_get_expanded(&make, var_ext_empty);
	EXPECT_STRN(exp.data, "", exp.len);
	res = make_get_resolved(&make, var_ext_empty, &tmp);
	EXPECT_STRN(res.data, "", res.len);

	exp = make_get_expanded(&make, var_ext_set);
	EXPECT_STRN(exp.data, "EXT", exp.len);
	res = make_get_resolved(&make, var_ext_set, &tmp);
	EXPECT_STRN(res.data, "EXT", res.len);

	exp = make_get_expanded(&make, def_var);
	EXPECT_STRN(exp.data, "D", exp.len);
	res = make_get_resolved(&make, def_var, &tmp);
	EXPECT_STRN(res.data, "D", res.len);

	exp = make_get_expanded(&make, def_var_imm);
	EXPECT_STRN(exp.data, "$(DEF_VAR)", exp.len);
	res = make_get_resolved(&make, def_var_imm, &tmp);
	EXPECT_STRN(res.data, "D", res.len);

	exp = make_get_expanded(&make, def_var_imm_var);
	EXPECT_STRN(exp.data, "$(DEF_VAR)", exp.len);
	res = make_get_resolved(&make, def_var_imm_var, &tmp);
	EXPECT_STRN(res.data, "D", res.len);

	exp = make_get_expanded(&make, def_imm);
	EXPECT_STRN(exp.data, "V2", exp.len);
	res = make_get_resolved(&make, def_imm, &tmp);
	EXPECT_STRN(res.data, "V2", res.len);

	exp = make_get_expanded(&make, def_imm_var);
	EXPECT_STRN(exp.data, "V2", exp.len);
	res = make_get_resolved(&make, def_imm_var, &tmp);
	EXPECT_STRN(res.data, "V2", res.len);

	exp = make_get_expanded(&make, def_imm_esc);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_get_resolved(&make, def_imm_esc, &tmp);
	EXPECT_STRN(res.data, "V2", res.len);

	exp = make_get_expanded(&make, def_rec);
	EXPECT_STRN(exp.data, "V2", exp.len);
	res = make_get_resolved(&make, def_rec, &tmp);
	EXPECT_STRN(res.data, "V2", res.len);

	exp = make_get_expanded(&make, def_rec_esc);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_get_resolved(&make, def_rec_esc, &tmp);
	EXPECT_STRN(res.data, "V3", res.len);

	exp = make_get_expanded(&make, def_rec_esc_var);
	EXPECT_STRN(exp.data, "$(VAR)", exp.len);
	res = make_get_resolved(&make, def_rec_esc_var, &tmp);
	EXPECT_STRN(res.data, "V2", res.len);

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
