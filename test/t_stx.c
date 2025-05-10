#include "stx.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(stx_init_free)
{
	START;

	stx_t stx = {0};

	EXPECT_EQ(stx_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(stx_init(&stx, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_init(&stx, 0, ALLOC_STD), &stx);
	log_set_quiet(0, 0);

	EXPECT_NE(stx.nodes.data, NULL);
	EXPECT_NE(stx.strs.data, NULL);

	stx_free(&stx);
	stx_free(NULL);

	EXPECT_EQ(stx.nodes.data, NULL);
	EXPECT_EQ(stx.strs.data, NULL);

	END;
}

TEST(stx_rule)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_node_t rule;

	EXPECT_EQ(stx_rule(NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(stx_rule(&stx, STRV("rule"), &rule), 0);
	EXPECT_EQ(rule, 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_oom)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, ALLOC_STD);

	stx_node_t rule;

	mem_oom(1);
	EXPECT_EQ(stx_rule(&stx, STRV("123456789"), NULL), 1);
	stx.nodes.cap = 0;
	EXPECT_EQ(stx_rule(&stx, STRV("rule"), NULL), 1);
	stx.nodes.cap = 1;
	mem_oom(0);

	EXPECT_EQ(stx_rule(&stx, STRV("rule"), &rule), 0);
	EXPECT_EQ(rule, 0);
	EXPECT_EQ(stx.strs.used, 4);

	stx_free(&stx);

	END;
}

TEST(stx_term_rule)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, ALLOC_STD);

	stx_node_t rule, term;

	EXPECT_EQ(stx_term_rule(NULL, 0, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_term_rule(&stx, 0, NULL), 1);
	log_set_quiet(0, 0);

	stx_rule(&stx, STRV(""), &rule);

	mem_oom(1);
	EXPECT_EQ(stx_term_rule(&stx, rule, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(stx_term_rule(&stx, rule, &term), 0);
	EXPECT_EQ(term, 1);

	stx_free(&stx);

	END;
}

TEST(stx_term_tok)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_node_t term;

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
	stx_init(&stx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_node_t term;

	EXPECT_EQ(stx_term_lit(NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(stx_term_lit(&stx, STRV("lit"), &term), 0);
	EXPECT_EQ(term, 0);

	stx_free(&stx);

	END;
}
TEST(stx_term_lit_oom)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, ALLOC_STD);

	stx_node_t lit;

	mem_oom(1);
	EXPECT_EQ(stx_term_lit(&stx, STRV("123456789"), NULL), 1);
	stx.nodes.cap = 0;
	EXPECT_EQ(stx_term_lit(&stx, STRV("lit"), &lit), 1);
	stx.nodes.cap = 1;
	mem_oom(0);

	EXPECT_EQ(stx_term_lit(&stx, STRV("lit"), &lit), 0);
	EXPECT_EQ(lit, 0);
	EXPECT_EQ(stx.strs.used, 3);

	stx_free(&stx);

	END;
}

TEST(stx_term_or)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_node_t term;

	EXPECT_EQ(stx_term_or(NULL, 0, 0, NULL), 1);

	stx_term_lit(&stx, STRV(""), &term);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_term_or(&stx, stx.nodes.cnt, term, NULL), 1);
	EXPECT_EQ(stx_term_or(&stx, term, stx.nodes.cnt, NULL), 1);
	log_set_quiet(0, 0);

	mem_oom(1);
	EXPECT_EQ(stx_term_or(&stx, term, term, NULL), 1);
	mem_oom(0);

	EXPECT_EQ(stx_term_or(&stx, term, term, &term), 0);
	EXPECT_EQ(term, 1);

	stx_free(&stx);

	END;
}

TEST(stx_find_rule)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_node_t rule, got;
	stx_term_lit(&stx, STRV("rule"), NULL);
	stx_rule(&stx, STRV("rules"), NULL);
	stx_rule(&stx, STRV("rule"), &rule);

	EXPECT_EQ(stx_find_rule(NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(stx_find_rule(&stx, STRV_NULL, NULL), 1);
	EXPECT_EQ(stx_find_rule(&stx, STRV("asd"), NULL), 1);
	EXPECT_EQ(stx_find_rule(&stx, STRV("rule"), &got), 0);
	EXPECT_EQ(got, rule);

	stx_free(&stx);

	END;
}

TEST(stx_get_node)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_node_t term;

	EXPECT_EQ(stx_get_node(NULL, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_get_node(&stx, 0), NULL);
	log_set_quiet(0, 0);

	stx_term_lit(&stx, STRV_NULL, &term);
	EXPECT_NE(stx_get_node(&stx, 0), NULL);

	stx_free(&stx);

	END;
}

TEST(stx_data_lit)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, ALLOC_STD);

	stx_node_t term;

	stx_term_lit(&stx, STRV("lit"), &term);

	const stx_node_data_t *data = stx_get_node(&stx, term);

	EXPECT_EQ(stx_data_lit(NULL, NULL).data, NULL);

	strv_t lit = stx_data_lit(&stx, data);
	EXPECT_STRN(lit.data, "lit", lit.len);

	stx_free(&stx);

	END;
}

TEST(stx_add_term)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_node_t rule;
	stx_rule(&stx, STRV("rule"), &rule);

	stx_node_t term;
	stx_term_lit(&stx, STRV(""), &term);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_add_term(NULL, stx.nodes.cnt, term), 1);
	EXPECT_EQ(stx_add_term(&stx, stx.nodes.cnt, term), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(stx_add_term(&stx, rule, term), 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_or)
{
	START;

	stx_t stx = {0};
	log_set_quiet(0, 1);
	stx_init(&stx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	stx_rule(&stx, STRV("rule"), NULL);

	EXPECT_EQ(stx_rule_add_or(NULL, stx.nodes.cnt, 0), 1);
	stx_node_t term;
	stx_term_lit(&stx, STRV("T"), &term);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_or(NULL, stx.nodes.cnt, 1, term), 1);
	EXPECT_EQ(stx_rule_add_or(NULL, stx.nodes.cnt, 2, term, term), 1);
	log_set_quiet(0, 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_arr)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 1, ALLOC_STD);

	stx_node_t rule;
	stx_rule(&stx, STRV("rule"), &rule);

	stx_node_t term;
	stx_term_tok(&stx, TOK_UPPER, &term);

	mem_oom(1);
	EXPECT_EQ(stx_rule_add_arr(&stx, rule, term), 1);
	mem_oom(0);

	EXPECT_EQ(stx_rule_add_arr(NULL, stx.nodes.cnt, term), 1);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_arr(&stx, stx.nodes.cnt, term), 1);
	log_set_quiet(0, 0);

	EXPECT_EQ(stx_rule_add_arr(&stx, rule, term), 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_arr_copy_oom)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 3, ALLOC_STD);

	stx_node_t rule;
	stx_rule(&stx, STRV("rule"), &rule);

	stx_node_t term;
	stx_term_tok(&stx, TOK_UPPER, &term);

	mem_oom(1);
	EXPECT_EQ(stx_rule_add_arr(&stx, rule, term), 1);
	mem_oom(0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_arr_sep)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 3, ALLOC_STD);

	stx_node_t rule;
	stx_rule(&stx, STRV("rule"), &rule);

	stx_node_t term, sep;
	stx_term_rule(&stx, rule, &sep);
	stx_term_tok(&stx, TOK_UPPER, &term);

	mem_oom(1);
	EXPECT_EQ(stx_rule_add_arr_sep(&stx, rule, term, sep), 1);
	mem_oom(0);

	EXPECT_EQ(stx_rule_add_arr_sep(NULL, stx.nodes.cnt, term, sep), 1);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_rule_add_arr_sep(&stx, stx.nodes.cnt, term, sep), 1);
	log_set_quiet(0, 0);

	EXPECT_EQ(stx_rule_add_arr_sep(&stx, rule, term, sep), 0);

	stx_free(&stx);

	END;
}

TEST(stx_rule_add_arr_sep_copy_oom)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 4, ALLOC_STD);

	stx_node_t rule;
	stx_rule(&stx, STRV("rule"), &rule);

	stx_node_t term, sep;
	stx_term_rule(&stx, rule, &sep);
	stx_term_tok(&stx, TOK_UPPER, &term);

	mem_oom(1);
	EXPECT_EQ(stx_rule_add_arr_sep(&stx, rule, term, sep), 1);
	mem_oom(0);

	stx_free(&stx);

	END;
}

TEST(stx_print)
{
	START;

	stx_t stx = {0};
	stx_init(&stx, 20, ALLOC_STD);

	stx_node_t file, line;
	stx_rule(&stx, STRV("file"), &file);
	stx_rule(&stx, STRV("line"), &line);

	stx_node_t term;
	stx_term_rule(&stx, line, &term);
	stx_add_term(&stx, file, term);

	stx_term_tok(&stx, -1, &term);
	stx_add_term(&stx, line, term);
	stx_term_tok(&stx, TOK_ALPHA, &term);
	stx_add_term(&stx, line, term);
	stx_term_lit(&stx, STRV(";"), &term);
	stx_add_term(&stx, line, term);
	stx_term_lit(&stx, STRV("'"), &term);
	stx_add_term(&stx, line, term);

	stx_node_t a, b, orv;
	stx_term_lit(&stx, STRV("A"), &a);
	stx_term_lit(&stx, STRV("B"), &b);
	stx_term_or(&stx, a, b, &orv);
	stx_add_term(&stx, line, orv);

	char buf[64] = {0};
	EXPECT_EQ(stx_print(NULL, DST_BUF(buf)), 0);

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print(&stx, DST_BUF(buf)), 61);
	log_set_quiet(0, 0);
	EXPECT_STR(buf,
		   "<file> ::= <line>\n"
		   "<line> ::= UNKNOWN ALPHA ';' \"'\" 'A' | 'B'\n");

	stx_term_tok(&stx, 0, &term);
	stx_node_data_t *data = stx_get_node(&stx, term);
	data->type	      = -1;

	stx_add_term(&stx, line, term);

	stx_get_node(&stx, orv)->val.orv.l = stx.nodes.cnt;
	stx_get_node(&stx, orv)->val.orv.r = stx.nodes.cnt;

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print(&stx, DST_BUF(buf)), 53);
	log_set_quiet(0, 0);
	EXPECT_STR(buf,
		   "<file> ::= <line>\n"
		   "<line> ::= UNKNOWN ALPHA ';' \"'\" |\n");

	stx_free(&stx);

	END;
}

TEST(stx_print_tree)
{
	START;

	stx_t stx = {0};

	stx_init(&stx, 20, ALLOC_STD);

	stx_node_t file, functions, function, identifier, chars;

	stx_rule(&stx, STRV("file"), &file);
	stx_rule(&stx, STRV("funcs"), &functions);
	stx_rule(&stx, STRV("func"), &function);
	stx_rule(&stx, STRV("id"), &identifier);
	stx_rule(&stx, STRV("chars"), &chars);

	stx_node_t term;
	stx_term_rule(&stx, functions, &term);
	stx_add_term(&stx, file, term);
	stx_term_tok(&stx, TOK_EOF, &term);
	stx_add_term(&stx, file, term);

	stx_term_rule(&stx, function, &term);
	stx_rule_add_arr(&stx, functions, term);

	stx_term_rule(&stx, identifier, &term);
	stx_add_term(&stx, function, term);

	stx_term_rule(&stx, chars, &term);
	stx_rule_add_arr(&stx, identifier, term);

	stx_node_t t0, t1, t2, t3;
	stx_term_tok(&stx, TOK_ALPHA, &t0);
	stx_term_tok(&stx, TOK_DIGIT, &t1);
	stx_term_lit(&stx, STRV("_"), &t2);
	stx_term_lit(&stx, STRV("'"), &t3);
	stx_rule_add_or(&stx, chars, 4, t0, t1, t2, t3);

	char buf[512] = {0};

	EXPECT_EQ(stx_print_tree(NULL, DST_BUF(buf)), 0);

	EXPECT_EQ(stx_print_tree(&stx, DST_BUF(buf)), 251);
	EXPECT_STR(buf,
		   "<file>\n"
		   "├─<funcs>\n"
		   "└─EOF\n"
		   "\n"
		   "<funcs>\n"
		   "or┬─<func>\n"
		   "│ └─<funcs>\n"
		   "└───<func>\n"
		   "\n"
		   "<func>\n"
		   "└─<id>\n"
		   "\n"
		   "<id>\n"
		   "or┬─<chars>\n"
		   "│ └─<id>\n"
		   "└───<chars>\n"
		   "\n"
		   "<chars>\n"
		   "or──ALPHA\n"
		   "└─or──DIGIT\n"
		   "  └─or──'_'\n"
		   "    └───\"'\"\n");

	stx_term_tok(&stx, 0, &term);
	stx_node_data_t *data = stx_get_node(&stx, term);
	data->type	      = -1;

	stx_add_term(&stx, file, term);
	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print_tree(&stx, DST_BUF(buf)), 251);
	log_set_quiet(0, 0);

	stx_free(&stx);

	END;
}

TEST(stx_print_empty_rule)
{
	START;

	stx_t stx = {0};

	stx_init(&stx, 1, ALLOC_STD);

	stx_node_t file;
	stx_rule(&stx, STRV("file"), &file);

	char buf[16] = {0};

	EXPECT_EQ(stx_print(&stx, DST_BUF(buf)), 11);
	EXPECT_STR(buf, "<file> ::=\n");
	EXPECT_EQ(stx_print_tree(&stx, DST_BUF(buf)), 7);
	EXPECT_STR(buf, "<file>\n");

	stx_free(&stx);

	END;
}

TEST(stx_print_invalid_rule)
{
	START;

	stx_t stx = {0};

	stx_init(&stx, 2, ALLOC_STD);

	stx_node_t file, term;
	stx_rule(&stx, STRV("file"), &file);

	stx_term_lit(&stx, STRV(""), &term);
	stx_add_term(&stx, file, term);
	*stx_get_node(&stx, term) = (stx_node_data_t){
		.type	  = STX_TERM_RULE,
		.val.rule = -1,
	};

	char buf[16] = {0};

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_print(&stx, DST_BUF(buf)), 11);
	EXPECT_STR(buf, "<file> ::=\n");
	EXPECT_EQ(stx_print_tree(&stx, DST_BUF(buf)), 7);
	log_set_quiet(0, 0);
	EXPECT_STR(buf, "<file>\n");

	stx_free(&stx);

	END;
}

STEST(stx)
{
	SSTART;

	RUN(stx_init_free);
	RUN(stx_rule);
	RUN(stx_rule_oom);
	RUN(stx_term_rule);
	RUN(stx_term_lit);
	RUN(stx_term_lit_oom);
	RUN(stx_term_tok);
	RUN(stx_term_or);
	RUN(stx_find_rule);
	RUN(stx_get_node);
	RUN(stx_data_lit);
	RUN(stx_add_term);
	RUN(stx_rule_add_or);
	RUN(stx_rule_add_arr);
	RUN(stx_rule_add_arr_copy_oom);
	RUN(stx_rule_add_arr_sep);
	RUN(stx_rule_add_arr_sep_copy_oom);
	RUN(stx_print);
	RUN(stx_print_tree);
	RUN(stx_print_empty_rule);
	RUN(stx_print_invalid_rule);

	SEND;
}
