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

	EXPECT_EQ(estx_add_rule(NULL), ESTX_RULE_END);
	mem_oom(1);
	EXPECT_EQ(estx_add_rule(&estx), ESTX_RULE_END);
	mem_oom(0);
	EXPECT_EQ(estx_add_rule(&estx), 0);

	estx_free(&estx);

	END;
}

TEST(estx_create_term)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(ESTX_TERM_RULE(NULL, -1, 0), ESTX_TERM_END);
	mem_oom(1);
	EXPECT_EQ(ESTX_TERM_RULE(&estx, -1, 0), ESTX_TERM_END);
	mem_oom(0);
	EXPECT_EQ(ESTX_TERM_RULE(&estx, -1, 0), 0);

	estx_free(&estx);

	END;
}

TEST(estx_rule_set_term)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_rule_set_term(NULL, ESTX_RULE_END, -1), ESTX_TERM_END);
	EXPECT_EQ(estx_rule_set_term(&estx, ESTX_RULE_END, -1), ESTX_TERM_END);
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

	estx_term_t term = ESTX_TERM_RULE(&estx, -1, 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_term_add_term(NULL, ESTX_TERM_END, ESTX_TERM_RULE(&estx, -1, 0)), ESTX_TERM_END);
	EXPECT_EQ(estx_term_add_term(&estx, ESTX_TERM_END, ESTX_TERM_RULE(&estx, -1, 0)), ESTX_TERM_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(estx_term_add_term(&estx, term, ESTX_TERM_RULE(&estx, -1, 0)), 3);

	estx_free(&estx);

	END;
}

TEST(estx_print_no_term)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	estx_add_rule(&estx);

	char buf[64] = {0};
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 4);
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

	estx_term_t file = estx_rule_set_term(&estx, estx_add_rule(&estx), ESTX_TERM_CON(&estx));
	estx_term_t line = estx_rule_set_term(&estx, estx_add_rule(&estx), ESTX_TERM_CON(&estx));

	estx_term_add_term(&estx, file, ESTX_TERM_RULE(&estx, line, 0));

	estx_term_add_term(&estx, line, ESTX_TERM_TOKEN(&estx, TOKEN_UNKNOWN, ESTX_TERM_OCC_OPT));
	estx_term_add_term(&estx, line, ESTX_TERM_TOKEN(&estx, TOKEN_ALPHA, ESTX_TERM_OCC_REP));
	estx_term_add_term(&estx, line, ESTX_TERM_LITERAL(&estx, STRH(";"), ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP));
	estx_term_add_term(&estx, line, ESTX_TERM_LITERAL(&estx, STRH("'"), 0));

	const estx_term_t group = estx_term_add_term(&estx, line, ESTX_TERM_GROUP(&estx, 0));
	const estx_term_t a	= ESTX_TERM_LITERAL(&estx, STRH("A"), 0);
	const estx_term_t b	= ESTX_TERM_LITERAL(&estx, STRH("B"), 0);
	const estx_term_t alt	= estx_term_add_term(&estx, group, ESTX_TERM_ALT(&estx));
	estx_term_add_term(&estx, alt, a);
	estx_term_add_term(&estx, alt, b);

	char buf[256] = {0};
	EXPECT_EQ(estx_print(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	EXPECT_EQ(estx_print(&estx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 49);
	EXPECT_STR(buf,
		   "0 = 1\n"
		   "1 = UNKNOWN? ALPHA+ ';'* \"'\" ( 'A' | 'B' )\n");

	EXPECT_EQ(estx_print_tree(&estx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 128);
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

	estx_term_add_term(&estx, line, estx_create_term(&estx, (estx_term_data_t){.type = -1}));
	// estx_rule_add_term(&estx, line, ESTX_TERM_OR(-1, -1)); //TODO

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 49);
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

	estx_term_t file = estx_rule_set_term(&estx, estx_add_rule(&estx), ESTX_TERM_CON(&estx));

	estx_term_add_term(&estx, file, estx_create_term(&estx, (estx_term_data_t){.type = -1}));

	char buf[64] = {0};
	EXPECT_EQ(estx_print_tree(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print_tree(&estx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 15);
	log_set_quiet(0, 0);

	estx_free(&estx);

	END;
}

STEST(esyntax)
{
	SSTART;

	RUN(estx_init_free);
	RUN(estx_add_rule);
	RUN(estx_create_term);
	RUN(estx_rule_set_term);
	RUN(estx_term_add_term);
	RUN(estx_print_no_term);
	RUN(estx_print);
	RUN(estx_print_tree);

	SEND;
}
