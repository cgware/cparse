#include "esyntax.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(estx_init_free)
{
	START;

	estx_t estx = {0};

	EXPECT_EQ(estx_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(estx_init(&estx, 0, 0, ALLOC_STD), NULL);
	EXPECT_EQ(estx_init(&estx, 1, 0, ALLOC_STD), NULL);
	EXPECT_EQ(estx_init(&estx, 0, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_init(&estx, 0, 0, ALLOC_STD), &estx);
	log_set_quiet(0, 0);

	EXPECT_NE(estx.rules.data, NULL);
	EXPECT_NE(estx.terms.data, NULL);

	estx_free(&estx);
	estx_free(NULL);

	EXPECT_EQ(estx.rules.data, NULL);
	EXPECT_EQ(estx.terms.data, NULL);

	END;
}

TEST(estx_add_rule)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_rule_t rule;

	EXPECT_EQ(estx_add_rule(NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(estx_add_rule(&estx, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_add_rule(&estx, &rule), 0);
	EXPECT_EQ(rule, 0);

	estx_free(&estx);

	END;
}

TEST(estx_term_rule)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_term_t term;

	EXPECT_EQ(estx_term_rule(NULL, 0, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(estx_term_rule(&estx, 0, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_rule(&estx, 0, ESTX_TERM_OCC_ONE, &term), 0);
	EXPECT_EQ(term, 0);

	estx_free(&estx);

	END;
}

TEST(estx_term_tok)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_term_t term;

	EXPECT_EQ(estx_term_tok(NULL, 0, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(estx_term_tok(&estx, 0, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_tok(&estx, 0, ESTX_TERM_OCC_ONE, &term), 0);
	EXPECT_EQ(term, 0);

	estx_free(&estx);

	END;
}

TEST(estx_term_lit)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_term_t term;

	EXPECT_EQ(estx_term_lit(NULL, STRV_NULL, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(estx_term_lit(&estx, STRV_NULL, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_lit(&estx, STRV_NULL, ESTX_TERM_OCC_ONE, &term), 0);
	EXPECT_EQ(term, 0);

	estx_free(&estx);

	END;
}

TEST(estx_term_lit_strs)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	size_t strs_size = estx.strs.size;
	estx.strs.size	 = 0;

	mem_oom(1);
	EXPECT_EQ(estx_term_lit(&estx, STRV(" "), ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(0);

	estx.strs.size = strs_size;

	estx_free(&estx);

	END;
}

TEST(estx_term_alt)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_term_t term;

	EXPECT_EQ(estx_term_alt(NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(estx_term_alt(&estx, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_alt(&estx, &term), 0);
	EXPECT_EQ(term, 0);

	estx_free(&estx);

	END;
}

TEST(estx_term_con)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_term_t term;

	EXPECT_EQ(estx_term_con(NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(estx_term_con(&estx, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_con(&estx, &term), 0);
	EXPECT_EQ(term, 0);

	estx_free(&estx);

	END;
}

TEST(estx_term_group)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_term_t term;

	EXPECT_EQ(estx_term_group(NULL, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(estx_term_group(&estx, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_group(&estx, ESTX_TERM_OCC_ONE, &term), 0);
	EXPECT_EQ(term, 0);

	estx_free(&estx);

	END;
}

TEST(estx_rule_set_term)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	estx_rule_t rule;
	estx_add_rule(&estx, &rule);

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_rule_set_term(NULL, 0, -1), 1);
	EXPECT_EQ(estx_rule_set_term(&estx, estx.rules.cnt, -1), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(estx_rule_set_term(&estx, rule, 0), 0);

	estx_free(&estx);

	END;
}

TEST(estx_term_add_term)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_term_t term;
	estx_term_rule(&estx, -1, 0, &term);

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_term_add_term(NULL, 0, term), 1);
	EXPECT_EQ(estx_term_add_term(&estx, estx.terms.cnt, term), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(estx_term_add_term(&estx, term, term), 0);

	estx_free(&estx);

	END;
}

TEST(estx_print_no_term)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	estx_add_rule(&estx, NULL);

	char buf[64] = {0};
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 4);
	log_set_quiet(0, 0);
	EXPECT_STR(buf, "0 =\n");

	estx_free(&estx);

	END;
}

TEST(estx_print)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	estx_rule_t rule;
	estx_term_t file, line;
	estx_add_rule(&estx, &rule);
	estx_term_con(&estx, &file);
	estx_rule_set_term(&estx, rule, file);
	estx_add_rule(&estx, &rule);
	estx_term_con(&estx, &line);
	estx_rule_set_term(&estx, rule, line);

	estx_term_t term;
	estx_term_rule(&estx, line, ESTX_TERM_OCC_ONE, &term);
	estx_term_add_term(&estx, file, term);

	estx_term_tok(&estx, TOKEN_UNKNOWN, ESTX_TERM_OCC_OPT, &term);
	estx_term_add_term(&estx, line, term);
	estx_term_tok(&estx, TOKEN_ALPHA, ESTX_TERM_OCC_REP, &term);
	estx_term_add_term(&estx, line, term);
	estx_term_lit(&estx, STRV(";"), ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP, &term);
	estx_term_add_term(&estx, line, term);
	estx_term_lit(&estx, STRV("'"), ESTX_TERM_OCC_ONE, &term);
	estx_term_add_term(&estx, line, term);

	estx_term_t group;
	estx_term_group(&estx, ESTX_TERM_OCC_ONE, &group);
	estx_term_add_term(&estx, line, group);
	estx_term_t a, b;
	estx_term_lit(&estx, STRV("A"), ESTX_TERM_OCC_ONE, &a);
	estx_term_lit(&estx, STRV("B"), ESTX_TERM_OCC_ONE, &b);
	estx_term_t alt;
	estx_term_alt(&estx, &alt);
	estx_term_add_term(&estx, group, alt);
	estx_term_add_term(&estx, alt, a);
	estx_term_add_term(&estx, alt, b);

	char buf[256] = {0};
	EXPECT_EQ(estx_print(NULL, DST_BUF(buf)), 0);

	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 49);
	EXPECT_STR(buf,
		   "0 = 1\n"
		   "1 = UNKNOWN? ALPHA+ ';'* \"'\" ( 'A' | 'B' )\n");

	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 128);
	EXPECT_STR(buf,
		   "<0>\n"
		   "con\n"
		   "└─<1>\n"
		   "\n"
		   "<1>\n"
		   "con\n"
		   "├─UNKNOWN?\n"
		   "├─ALPHA+\n"
		   "├─';'*\n"
		   "├─\"'\"\n"
		   "└─group\n"
		   "  └─alt\n"
		   "    ├─'A'\n"
		   "    └─'B'\n");

	estx_term_con(&estx, &term);
	estx_term_add_term(&estx, line, term);
	estx_term_data_t *data = estx_get_term_data(&estx, term);
	data->type	       = -1;

	// estx_rule_add_term(&estx, line, ESTX_TERM_OR(-1, -1)); //TODO

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 49);
	log_set_quiet(0, 0);
	EXPECT_STR(buf,
		   "0 = 1\n"
		   "1 = UNKNOWN? ALPHA+ ';'* \"'\" ( 'A' | 'B' )\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_tree)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	estx_rule_t rule;
	estx_term_t file;
	estx_add_rule(&estx, &rule);
	estx_term_con(&estx, &file);
	estx_rule_set_term(&estx, rule, file);

	estx_term_t term;
	estx_term_con(&estx, &term);
	estx_term_add_term(&estx, file, term);
	estx_term_data_t *data = estx_get_term_data(&estx, term);
	data->type	       = -1;

	char buf[64] = {0};
	EXPECT_EQ(estx_print_tree(NULL, DST_BUF(buf)), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 15);
	log_set_quiet(0, 0);

	estx_free(&estx);

	END;
}

STEST(esyntax)
{
	SSTART;

	RUN(estx_init_free);
	RUN(estx_add_rule);
	RUN(estx_term_rule);
	RUN(estx_term_tok);
	RUN(estx_term_lit);
	RUN(estx_term_lit_strs);
	RUN(estx_term_alt);
	RUN(estx_term_con);
	RUN(estx_term_group);
	RUN(estx_rule_set_term);
	RUN(estx_term_add_term);
	RUN(estx_print_no_term);
	RUN(estx_print);
	RUN(estx_print_tree);

	SEND;
}
