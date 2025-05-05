#include "syntax.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(stx_init_free)
{
	START;

	stx_t stx = {0};

	EXPECT_EQ(stx_init(NULL, 0, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(stx_init(&stx, 0, 0, ALLOC_STD), NULL);
	EXPECT_EQ(stx_init(&stx, 1, 0, ALLOC_STD), NULL);
	EXPECT_EQ(stx_init(&stx, 0, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_init(&stx, 0, 0, ALLOC_STD), &stx);
	log_set_quiet(0, 0);

	EXPECT_NE(stx.rules.data, NULL);
	EXPECT_NE(stx.terms.data, NULL);

	stx_free(&stx);
	stx_free(NULL);

	EXPECT_EQ(stx.rules.data, NULL);
	EXPECT_EQ(stx.terms.data, NULL);

	END;
}

TEST(stx_add_rule)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_rule_t rule;

	EXPECT_EQ(stx_add_rule(NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(stx_add_rule(&stx, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(stx_add_rule(&stx, &rule), 0);
	EXPECT_EQ(rule, 0);

	stx_free(&stx);

	END;
}

TEST(stx_create_term)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(STX_TERM_RULE(NULL, -1), (uint)-1);
	mem_oom(1);
	EXPECT_EQ(STX_TERM_RULE(&stx, -1), (uint)-1);
	mem_oom(0);
	EXPECT_EQ(STX_TERM_RULE(&stx, -1), 0);

	stx_free(&stx);

	END;
}

TEST(stx_create_literal)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	stx_create_literal(NULL, STRV_NULL);
	stx_create_literal(&stx, STRV_NULL);

	stx_free(&stx);

	END;
}

TEST(stx_rule_set_term)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_set_term(NULL, stx.rules.cnt, -1), (uint)-1);
	EXPECT_EQ(stx_rule_set_term(&stx, stx.rules.cnt, -1), (uint)-1);
	log_set_quiet(0, 0);
	EXPECT_EQ(stx_rule_set_term(&stx, rule, 0), 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_term)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_term(NULL, stx.rules.cnt, STX_TERM_RULE(&stx, -1)), (uint)-1);
	EXPECT_EQ(stx_rule_add_term(&stx, stx.rules.cnt, STX_TERM_RULE(&stx, -1)), (uint)-1);
	log_set_quiet(0, 0);
	EXPECT_EQ(stx_rule_add_term(&stx, rule, STX_TERM_RULE(&stx, -1)), 2);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_or)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_add_rule(&stx, NULL);

	EXPECT_EQ(stx_rule_add_or(NULL, stx.rules.cnt, 0), (uint)-1);
	stx_term_t term = STX_TERM_LITERAL(&stx, STRV("T"));
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_or(NULL, stx.rules.cnt, 1, term), (uint)-1);
	EXPECT_EQ(stx_rule_add_or(NULL, stx.rules.cnt, 2, term, term), (uint)-1);
	log_set_quiet(0, 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_arr)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);

	STX_TERM_LITERAL(&stx, STRV("T"));

	EXPECT_EQ(stx_rule_add_arr(NULL, stx.rules.cnt, STX_TERM_TOKEN(&stx, TOKEN_UPPER), STX_TERM_NONE(&stx)), (uint)-1);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_arr(&stx, stx.rules.cnt, STX_TERM_TOKEN(&stx, TOKEN_UPPER), STX_TERM_RULE(&stx, rule)), (uint)-1);
	log_set_quiet(0, 0);

	stx_free(&stx);

	END;
}

TEST(stx_term_add_term)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_term_t term = STX_TERM_RULE(&stx, -1);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_term_add_term(NULL, stx.terms.cnt, STX_TERM_RULE(&stx, -1)), (uint)-1);
	EXPECT_EQ(stx_term_add_term(&stx, stx.terms.cnt, STX_TERM_RULE(&stx, -1)), (uint)-1);
	log_set_quiet(0, 0);
	EXPECT_EQ(stx_term_add_term(&stx, term, STX_TERM_RULE(&stx, -1)), 3);

	stx_free(&stx);

	END;
}

TEST(stx_print)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	stx_rule_t file, line;
	stx_add_rule(&stx, &file);
	stx_add_rule(&stx, &line);

	stx_rule_add_term(&stx, file, STX_TERM_RULE(&stx, line));

	stx_rule_add_term(&stx, line, STX_TERM_TOKEN(&stx, -1));
	stx_rule_add_term(&stx, line, STX_TERM_TOKEN(&stx, TOKEN_ALPHA));
	stx_rule_add_term(&stx, line, STX_TERM_LITERAL(&stx, STRV(";")));
	stx_rule_add_term(&stx, line, STX_TERM_LITERAL(&stx, STRV("'")));

	const stx_term_t a = STX_TERM_LITERAL(&stx, STRV("A"));
	const stx_term_t b = STX_TERM_LITERAL(&stx, STRV("B"));
	stx_rule_add_term(&stx, line, STX_TERM_OR(&stx, a, b));

	char buf[64] = {0};
	EXPECT_EQ(stx_print(NULL, DST_BUF(buf)), 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print(&stx, DST_BUF(buf)), 52);
	log_set_quiet(0, 0);
	EXPECT_STR(buf,
		   "<0> ::= <1>\n"
		   "<1> ::= UNKNOWN ALPHA ';' \"'\" 'A' | 'B'\n");

	stx_rule_add_term(&stx, line, stx_create_term(&stx, (stx_term_data_t){.type = -1}));
	// stx_rule_add_term(&stx, line, STX_TERM_OR(-1, -1)); //TODO

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print(&stx, DST_BUF(buf)), 52);
	log_set_quiet(0, 0);
	EXPECT_STR(buf,
		   "<0> ::= <1>\n"
		   "<1> ::= UNKNOWN ALPHA ';' \"'\" 'A' | 'B'\n");

	stx_free(&stx);

	END;
}

TEST(stx_print_tree)
{
	START;

	stx_t stx = {0};

	stx_init(&stx, 10, 10, ALLOC_STD);

	stx_rule_t file, functions, function, identifier, chars;

	stx_add_rule(&stx, &file);
	stx_add_rule(&stx, &functions);
	stx_add_rule(&stx, &function);
	stx_add_rule(&stx, &identifier);
	stx_add_rule(&stx, &chars);

	stx_rule_add_term(&stx, file, STX_TERM_RULE(&stx, functions));
	stx_rule_add_term(&stx, file, STX_TERM_TOKEN(&stx, TOKEN_EOF));

	stx_rule_add_arr(&stx, functions, STX_TERM_RULE(&stx, function), STX_TERM_NONE(&stx));

	stx_rule_add_term(&stx, function, STX_TERM_RULE(&stx, identifier));

	stx_rule_add_arr(&stx, identifier, STX_TERM_RULE(&stx, chars), STX_TERM_NONE(&stx));

	stx_rule_add_or(&stx,
			chars,
			4,
			STX_TERM_TOKEN(&stx, TOKEN_ALPHA),
			STX_TERM_TOKEN(&stx, TOKEN_DIGIT),
			STX_TERM_LITERAL(&stx, STRV("_")),
			STX_TERM_LITERAL(&stx, STRV("'")));

	char buf[256] = {0};

	EXPECT_EQ(stx_print_tree(NULL, DST_BUF(buf)), 0);

	EXPECT_EQ(stx_print_tree(&stx, DST_BUF(buf)), 212);
	EXPECT_STR(buf,
		   "<0>\n"
		   "├─<1>\n"
		   "└─EOF\n"
		   "\n"
		   "<1>\n"
		   "or┬─<2>\n"
		   "│ └─<1>\n"
		   "└───<2>\n"
		   "\n"
		   "<2>\n"
		   "└─<3>\n"
		   "\n"
		   "<3>\n"
		   "or┬─<4>\n"
		   "│ └─<3>\n"
		   "└───<4>\n"
		   "\n"
		   "<4>\n"
		   "or──ALPHA\n"
		   "└─or──DIGIT\n"
		   "  └─or──'_'\n"
		   "    └───\"'\"\n");

	stx_rule_add_term(&stx, file, stx_create_term(&stx, (stx_term_data_t){.type = -1}));
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print_tree(&stx, DST_BUF(buf)), 212);
	log_set_quiet(0, 0);

	stx_free(&stx);

	END;
}

STEST(syntax)
{
	SSTART;

	RUN(stx_init_free);
	RUN(stx_add_rule);
	RUN(stx_create_term);
	RUN(stx_create_literal);
	RUN(stx_rule_set_term);
	RUN(stx_rule_add_term);
	RUN(stx_rule_add_or);
	RUN(stx_rule_add_arr);
	RUN(stx_term_add_term);
	RUN(stx_print);
	RUN(stx_print_tree);

	SEND;
}
