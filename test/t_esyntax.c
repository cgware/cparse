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

	EXPECT_EQ(estx_add_rule(NULL), (uint)-1);
	mem_oom(1);
	EXPECT_EQ(estx_add_rule(&estx), (uint)-1);
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

	EXPECT_EQ(ESTX_TERM_RULE(NULL, -1, 0), (uint)-1);
	mem_oom(1);
	EXPECT_EQ(ESTX_TERM_RULE(&estx, -1, 0), (uint)-1);
	mem_oom(0);
	EXPECT_EQ(ESTX_TERM_RULE(&estx, -1, 0), 0);

	estx_free(&estx);

	END;
}

TEST(estx_create_literal)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	estx_create_literal(NULL, STRV_NULL, 0);
	estx_create_literal(&estx, STRV_NULL, 0);
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
	EXPECT_EQ(estx_rule_set_term(NULL, 0, -1), (uint)-1);
	EXPECT_EQ(estx_rule_set_term(&estx, estx.rules.cnt, -1), (uint)-1);
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
	EXPECT_EQ(estx_term_add_term(NULL, 0, ESTX_TERM_RULE(&estx, -1, 0)), (uint)-1);
	EXPECT_EQ(estx_term_add_term(&estx, estx.terms.cnt, ESTX_TERM_RULE(&estx, -1, 0)), (uint)-1);
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

	estx_term_t file = estx_rule_set_term(&estx, estx_add_rule(&estx), ESTX_TERM_CON(&estx));
	estx_term_t line = estx_rule_set_term(&estx, estx_add_rule(&estx), ESTX_TERM_CON(&estx));

	estx_term_add_term(&estx, file, ESTX_TERM_RULE(&estx, line, 0));

	estx_term_add_term(&estx, line, ESTX_TERM_TOKEN(&estx, TOKEN_UNKNOWN, ESTX_TERM_OCC_OPT));
	estx_term_add_term(&estx, line, ESTX_TERM_TOKEN(&estx, TOKEN_ALPHA, ESTX_TERM_OCC_REP));
	estx_term_add_term(&estx, line, ESTX_TERM_LITERAL(&estx, STRV(";"), ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP));
	estx_term_add_term(&estx, line, ESTX_TERM_LITERAL(&estx, STRV("'"), 0));

	const estx_term_t group = estx_term_add_term(&estx, line, ESTX_TERM_GROUP(&estx, 0));
	const estx_term_t a	= ESTX_TERM_LITERAL(&estx, STRV("A"), 0);
	const estx_term_t b	= ESTX_TERM_LITERAL(&estx, STRV("B"), 0);
	const estx_term_t alt	= estx_term_add_term(&estx, group, ESTX_TERM_ALT(&estx));
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

	estx_term_add_term(&estx, line, estx_create_term(&estx, (estx_term_data_t){.type = -1}));
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

	estx_term_t file = estx_rule_set_term(&estx, estx_add_rule(&estx), ESTX_TERM_CON(&estx));

	estx_term_add_term(&estx, file, estx_create_term(&estx, (estx_term_data_t){.type = -1}));

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
	RUN(estx_create_term);
	RUN(estx_create_literal);
	RUN(estx_rule_set_term);
	RUN(estx_term_add_term);
	RUN(estx_print_no_term);
	RUN(estx_print);
	RUN(estx_print_tree);

	SEND;
}
