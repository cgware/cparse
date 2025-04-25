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

	EXPECT_EQ(stx_add_rule(NULL), STX_RULE_END);
	mem_oom(1);
	EXPECT_EQ(stx_add_rule(&stx), STX_RULE_END);
	mem_oom(0);
	EXPECT_EQ(stx_add_rule(&stx), 0);

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

	EXPECT_EQ(STX_TERM_RULE(NULL, -1), STX_TERM_END);
	mem_oom(1);
	EXPECT_EQ(STX_TERM_RULE(&stx, -1), STX_TERM_END);
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

	stx_rule_t rule = stx_add_rule(&stx);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_set_term(NULL, STX_RULE_END, -1), STX_TERM_END);
	EXPECT_EQ(stx_rule_set_term(&stx, STX_RULE_END, -1), STX_TERM_END);
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

	stx_rule_t rule = stx_add_rule(&stx);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_term(NULL, STX_RULE_END, STX_TERM_RULE(&stx, -1)), STX_TERM_END);
	EXPECT_EQ(stx_rule_add_term(&stx, STX_RULE_END, STX_TERM_RULE(&stx, -1)), STX_TERM_END);
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

	stx_add_rule(&stx);

	EXPECT_EQ(stx_rule_add_or(NULL, STX_RULE_END, 0), STX_TERM_END);
	stx_term_t term = STX_TERM_LITERAL(&stx, STRV("T"));
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_or(NULL, STX_RULE_END, 1, term), STX_TERM_END);
	EXPECT_EQ(stx_rule_add_or(NULL, STX_RULE_END, 2, term, term), STX_TERM_END);
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

	stx_rule_t rule = stx_add_rule(&stx);

	STX_TERM_LITERAL(&stx, STRV("T"));

	EXPECT_EQ(stx_rule_add_arr(NULL, STX_RULE_END, STX_TERM_TOKEN(&stx, TOKEN_UPPER), STX_TERM_NONE(&stx)), STX_TERM_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_arr(&stx, STX_RULE_END, STX_TERM_TOKEN(&stx, TOKEN_UPPER), STX_TERM_RULE(&stx, rule)), STX_TERM_END);
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
	EXPECT_EQ(stx_term_add_term(NULL, STX_TERM_END, STX_TERM_RULE(&stx, -1)), STX_TERM_END);
	EXPECT_EQ(stx_term_add_term(&stx, STX_TERM_END, STX_TERM_RULE(&stx, -1)), STX_TERM_END);
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

	stx_rule_t file = stx_add_rule(&stx);
	stx_rule_t line = stx_add_rule(&stx);

	stx_rule_add_term(&stx, file, STX_TERM_RULE(&stx, line));

	stx_rule_add_term(&stx, line, STX_TERM_TOKEN(&stx, -1));
	stx_rule_add_term(&stx, line, STX_TERM_TOKEN(&stx, TOKEN_ALPHA));
	stx_rule_add_term(&stx, line, STX_TERM_LITERAL(&stx, STRV(";")));
	stx_rule_add_term(&stx, line, STX_TERM_LITERAL(&stx, STRV("'")));

	const stx_term_t a = STX_TERM_LITERAL(&stx, STRV("A"));
	const stx_term_t b = STX_TERM_LITERAL(&stx, STRV("B"));
	stx_rule_add_term(&stx, line, STX_TERM_OR(&stx, a, b));

	char buf[64] = {0};
	EXPECT_EQ(stx_print(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print(&stx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 52);
	log_set_quiet(0, 0);
	EXPECT_STR(buf,
		   "<0> ::= <1>\n"
		   "<1> ::= UNKNOWN ALPHA ';' \"'\" 'A' | 'B'\n");

	stx_rule_add_term(&stx, line, stx_create_term(&stx, (stx_term_data_t){.type = -1}));
	// stx_rule_add_term(&stx, line, STX_TERM_OR(-1, -1)); //TODO

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print(&stx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 52);
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

	const stx_rule_t file	    = stx_add_rule(&stx);
	const stx_rule_t functions  = stx_add_rule(&stx);
	const stx_rule_t function   = stx_add_rule(&stx);
	const stx_rule_t identifier = stx_add_rule(&stx);
	const stx_rule_t chars	    = stx_add_rule(&stx);

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

	EXPECT_EQ(stx_print_tree(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	EXPECT_EQ(stx_print_tree(&stx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 212);
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
	EXPECT_EQ(stx_print_tree(&stx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 212);
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
