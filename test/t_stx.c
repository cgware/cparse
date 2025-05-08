#include "stx.h"

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

TEST(stx_term_rule)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_term_t term;

	EXPECT_EQ(stx_term_rule(NULL, 0, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(stx_term_rule(&stx, 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(stx_term_rule(&stx, 0, &term), 0);
	EXPECT_EQ(term, 0);

	stx_free(&stx);

	END;
}

TEST(stx_term_tok)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_term_t term;

	EXPECT_EQ(stx_term_tok(NULL, 0, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(stx_term_tok(&stx, 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(stx_term_tok(&stx, 0, &term), 0);
	EXPECT_EQ(term, 0);

	stx_free(&stx);

	END;
}

TEST(stx_term_lit)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_term_t term;

	EXPECT_EQ(stx_term_lit(NULL, STRV_NULL, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(stx_term_lit(&stx, STRV(""), &term), 1);

	size_t strs_size = stx.strs.size;
	stx.strs.size	 = 0;
	EXPECT_EQ(stx_term_lit(&stx, STRV(""), &term), 1);
	stx.strs.size = strs_size;

	mem_oom(0);
	EXPECT_EQ(stx_term_lit(&stx, STRV_NULL, &term), 0);
	EXPECT_EQ(term, 0);

	stx_free(&stx);

	END;
}
TEST(stx_term_lit_strs)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 1, ALLOC_STD);
	log_set_quiet(0, 0);

	size_t strs_size = stx.strs.size;
	stx.strs.size	 = 0;

	mem_oom(1);
	EXPECT_EQ(stx_term_lit(&stx, STRV(" "), NULL), 1);
	mem_oom(0);

	stx.strs.size = strs_size;

	stx_free(&stx);

	END;
}

TEST(stx_term_or)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_term_t term;

	EXPECT_EQ(stx_term_or(NULL, 0, 0, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(stx_term_or(&stx, 0, 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(stx_term_or(&stx, 0, 0, &term), 0);
	EXPECT_EQ(term, 0);

	stx_free(&stx);

	END;
}

TEST(stx_get_term_data)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_term_t term;

	EXPECT_EQ(stx_get_term_data(NULL, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_get_term_data(&stx, 0), NULL);
	log_set_quiet(0, 0);

	stx_term_lit(&stx, STRV_NULL, &term);
	EXPECT_NE(stx_get_term_data(&stx, 0), NULL);

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

	stx_term_t term;
	stx_term_rule(&stx, -1, &term);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_term(NULL, stx.rules.cnt, term), 1);
	EXPECT_EQ(stx_rule_add_term(&stx, stx.rules.cnt, term), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(stx_rule_add_term(&stx, rule, term), 0);

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

	EXPECT_EQ(stx_rule_add_or(NULL, stx.rules.cnt, 0), 1);
	stx_term_t term;
	stx_term_lit(&stx, STRV("T"), &term);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_or(NULL, stx.rules.cnt, 1, term), 1);
	EXPECT_EQ(stx_rule_add_or(NULL, stx.rules.cnt, 2, term, term), 1);
	log_set_quiet(0, 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_arr)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);

	stx_term_t term;
	stx_term_tok(&stx, TOK_UPPER, &term);

	mem_oom(1);
	EXPECT_EQ(stx_rule_add_arr(&stx, rule, term), 1);
	mem_oom(0);

	EXPECT_EQ(stx_rule_add_arr(NULL, stx.rules.cnt, term), 1);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_arr(&stx, stx.rules.cnt, term), 1);
	log_set_quiet(0, 0);

	EXPECT_EQ(stx_rule_add_arr(&stx, rule, term), 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_arr_sep)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, 2, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);

	stx_term_t term, sep;
	stx_term_rule(&stx, rule, &sep);
	stx_term_tok(&stx, TOK_UPPER, &term);

	mem_oom(1);
	EXPECT_EQ(stx_rule_add_arr_sep(&stx, rule, term, sep), 1);
	mem_oom(0);

	EXPECT_EQ(stx_rule_add_arr_sep(NULL, stx.rules.cnt, term, sep), 1);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_arr_sep(&stx, stx.rules.cnt, term, sep), 1);
	log_set_quiet(0, 0);

	EXPECT_EQ(stx_rule_add_arr_sep(&stx, rule, term, sep), 0);

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

	stx_term_t term;
	stx_term_rule(&stx, -1, &term);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_term_add_term(NULL, stx.terms.cnt, term), 1);
	EXPECT_EQ(stx_term_add_term(&stx, stx.terms.cnt, term), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(stx_term_add_term(&stx, term, term), 0);

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

	stx_term_t term;
	stx_term_rule(&stx, line, &term);
	stx_rule_add_term(&stx, file, term);

	stx_term_tok(&stx, -1, &term);
	stx_rule_add_term(&stx, line, term);
	stx_term_tok(&stx, TOK_ALPHA, &term);
	stx_rule_add_term(&stx, line, term);
	stx_term_lit(&stx, STRV(";"), &term);
	stx_rule_add_term(&stx, line, term);
	stx_term_lit(&stx, STRV("'"), &term);
	stx_rule_add_term(&stx, line, term);

	stx_term_t a, b;
	stx_term_lit(&stx, STRV("A"), &a);
	stx_term_lit(&stx, STRV("B"), &b);
	stx_term_or(&stx, a, b, &term);
	stx_rule_add_term(&stx, line, term);

	char buf[64] = {0};
	EXPECT_EQ(stx_print(NULL, DST_BUF(buf)), 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print(&stx, DST_BUF(buf)), 52);
	log_set_quiet(0, 0);
	EXPECT_STR(buf,
		   "<0> ::= <1>\n"
		   "<1> ::= UNKNOWN ALPHA ';' \"'\" 'A' | 'B'\n");

	stx_term_tok(&stx, 0, &term);
	stx_term_data_t *data = stx_get_term_data(&stx, term);
	data->type	      = -1;

	stx_rule_add_term(&stx, line, term);
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

	stx_term_t term;
	stx_term_rule(&stx, functions, &term);
	stx_rule_add_term(&stx, file, term);
	stx_term_tok(&stx, TOK_EOF, &term);
	stx_rule_add_term(&stx, file, term);

	stx_term_rule(&stx, function, &term);
	stx_rule_add_arr(&stx, functions, term);

	stx_term_rule(&stx, identifier, &term);
	stx_rule_add_term(&stx, function, term);

	stx_term_rule(&stx, chars, &term);
	stx_rule_add_arr(&stx, identifier, term);

	stx_term_t t0, t1, t2, t3;
	stx_term_tok(&stx, TOK_ALPHA, &t0);
	stx_term_tok(&stx, TOK_DIGIT, &t1);
	stx_term_lit(&stx, STRV("_"), &t2);
	stx_term_lit(&stx, STRV("'"), &t3);
	stx_rule_add_or(&stx, chars, 4, t0, t1, t2, t3);

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

	stx_term_tok(&stx, 0, &term);
	stx_term_data_t *data = stx_get_term_data(&stx, term);
	data->type	      = -1;

	stx_rule_add_term(&stx, file, term);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print_tree(&stx, DST_BUF(buf)), 212);
	log_set_quiet(0, 0);

	stx_free(&stx);

	END;
}

STEST(stx)
{
	SSTART;

	RUN(stx_init_free);
	RUN(stx_add_rule);
	RUN(stx_term_rule);
	RUN(stx_term_lit);
	RUN(stx_term_lit_strs);
	RUN(stx_term_tok);
	RUN(stx_term_or);
	RUN(stx_get_term_data);
	RUN(stx_rule_add_term);
	RUN(stx_rule_add_or);
	RUN(stx_rule_add_arr);
	RUN(stx_rule_add_arr_sep);
	RUN(stx_term_add_term);
	RUN(stx_print);
	RUN(stx_print_tree);

	SEND;
}
