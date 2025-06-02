#include "estx.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(estx_init_free)
{
	START;

	estx_t estx = {0};

	EXPECT_EQ(estx_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(estx_init(&estx, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_init(&estx, 0, ALLOC_STD), &estx);
	log_set_quiet(0, 0);

	EXPECT_NE(estx.nodes.data, NULL);
	EXPECT_NE(estx.strs.data, NULL);

	estx_free(&estx);
	estx_free(NULL);

	EXPECT_EQ(estx.nodes.data, NULL);
	EXPECT_EQ(estx.strs.data, NULL);

	END;
}

TEST(estx_rule)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t rule;

	EXPECT_EQ(estx_rule(NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(estx_rule(&estx, STRV("rule"), &rule), 0);
	EXPECT_EQ(rule, 0);

	estx_free(&estx);

	END;
}

TEST(estx_rule_oom)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, ALLOC_STD);

	estx_node_t rule;

	mem_oom(1);
	EXPECT_EQ(estx_rule(&estx, STRV("123456789"), NULL), 1);
	estx.nodes.cap = 0;
	EXPECT_EQ(estx_rule(&estx, STRV("rule"), NULL), 1);
	estx.nodes.cap = 1;
	mem_oom(0);

	EXPECT_EQ(estx_rule(&estx, STRV("rule"), &rule), 0);
	EXPECT_EQ(rule, 0);
	EXPECT_EQ(estx.strs.used, sizeof(size_t) + 4);

	estx_free(&estx);

	END;
}

TEST(estx_term_rule)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, ALLOC_STD);

	estx_node_t rule, term;

	EXPECT_EQ(estx_term_rule(NULL, 0, ESTX_TERM_OCC_ONE, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_term_rule(&estx, 0, ESTX_TERM_OCC_ONE, NULL), 1);
	log_set_quiet(0, 0);

	estx_rule(&estx, STRV(""), &rule);

	mem_oom(1);
	EXPECT_EQ(estx_term_rule(&estx, rule, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_rule(&estx, rule, ESTX_TERM_OCC_ONE, &term), 0);
	EXPECT_EQ(term, 1);

	estx_free(&estx);

	END;
}

TEST(estx_term_tok)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t term;

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
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t term;

	EXPECT_EQ(estx_term_lit(NULL, STRV_NULL, ESTX_TERM_OCC_ONE, NULL), 1);
	EXPECT_EQ(estx_term_lit(&estx, STRV("lit"), ESTX_TERM_OCC_ONE, &term), 0);
	EXPECT_EQ(term, 0);

	estx_free(&estx);

	END;
}
TEST(estx_term_lit_oom)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, ALLOC_STD);

	estx_node_t lit;

	mem_oom(1);
	EXPECT_EQ(estx_term_lit(&estx, STRV("123456789"), ESTX_TERM_OCC_ONE, NULL), 1);
	estx.nodes.cap = 0;
	EXPECT_EQ(estx_term_lit(&estx, STRV("lit"), ESTX_TERM_OCC_ONE, &lit), 1);
	estx.nodes.cap = 1;
	mem_oom(0);

	EXPECT_EQ(estx_term_lit(&estx, STRV("lit"), ESTX_TERM_OCC_ONE, &lit), 0);
	EXPECT_EQ(lit, 0);
	EXPECT_EQ(estx.strs.used, sizeof(size_t) + 3);

	estx_free(&estx);

	END;
}

TEST(estx_term_alt)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t term;

	EXPECT_EQ(estx_term_alt(NULL, 0, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_term_alt(&estx, 0, NULL), 1);
	log_set_quiet(0, 0);

	estx_node_t node;
	estx_term_tok(&estx, TOK_NULL, ESTX_TERM_OCC_ONE, &node);

	mem_oom(1);
	EXPECT_EQ(estx_term_alt(&estx, node, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_alt(&estx, node, &term), 0);
	EXPECT_EQ(term, 1);

	estx_free(&estx);

	END;
}

TEST(estx_term_con)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t term;

	EXPECT_EQ(estx_term_con(NULL, 0, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_term_con(&estx, 0, NULL), 1);
	log_set_quiet(0, 0);

	estx_node_t node;
	estx_term_tok(&estx, TOK_NULL, ESTX_TERM_OCC_ONE, &node);

	mem_oom(1);
	EXPECT_EQ(estx_term_con(&estx, node, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_con(&estx, node, &term), 0);
	EXPECT_EQ(term, 1);

	estx_free(&estx);

	END;
}

TEST(estx_term_group)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t term;

	EXPECT_EQ(estx_term_group(NULL, 0, ESTX_TERM_OCC_ONE, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_term_group(&estx, 0, ESTX_TERM_OCC_ONE, NULL), 1);
	log_set_quiet(0, 0);

	estx_node_t node;
	estx_term_tok(&estx, TOK_NULL, ESTX_TERM_OCC_ONE, &node);

	mem_oom(1);
	EXPECT_EQ(estx_term_group(&estx, node, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_group(&estx, node, ESTX_TERM_OCC_ONE, &term), 0);
	EXPECT_EQ(term, 1);

	estx_free(&estx);

	END;
}

TEST(estx_find_rule)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t rule, got;
	estx_term_lit(&estx, STRV("rule"), ESTX_TERM_OCC_ONE, NULL);
	estx_rule(&estx, STRV("rules"), NULL);
	estx_rule(&estx, STRV("rule"), &rule);

	EXPECT_EQ(estx_find_rule(NULL, STRV_NULL, NULL), 1);
	EXPECT_EQ(estx_find_rule(&estx, STRV_NULL, NULL), 1);
	EXPECT_EQ(estx_find_rule(&estx, STRV("asd"), NULL), 1);
	EXPECT_EQ(estx_find_rule(&estx, STRV("rule"), &got), 0);
	EXPECT_EQ(got, rule);

	estx_free(&estx);

	END;
}

TEST(estx_get_node)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t term;

	EXPECT_EQ(estx_get_node(NULL, 0), NULL);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_get_node(&estx, 0), NULL);
	log_set_quiet(0, 0);

	estx_term_lit(&estx, STRV_NULL, ESTX_TERM_OCC_ONE, &term);
	EXPECT_NE(estx_get_node(&estx, 0), NULL);

	estx_free(&estx);

	END;
}

TEST(estx_data_lit)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 1, ALLOC_STD);

	estx_node_t term;

	estx_term_lit(&estx, STRV("lit"), ESTX_TERM_OCC_ONE, &term);

	const estx_node_data_t *data = estx_get_node(&estx, term);

	EXPECT_EQ(estx_data_lit(NULL, NULL).data, NULL);

	strv_t lit = estx_data_lit(&estx, data);
	EXPECT_STRN(lit.data, "lit", lit.len);

	estx_free(&estx);

	END;
}

TEST(estx_add_term)
{
	START;

	estx_t estx = {0};
	log_set_quiet(0, 1);
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);

	estx_node_t term, term2;
	estx_term_rule(&estx, rule, ESTX_TERM_OCC_ONE, &term);
	estx_term_rule(&estx, rule, ESTX_TERM_OCC_ONE, &term2);

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_add_term(NULL, 0, term), 1);
	EXPECT_EQ(estx_add_term(&estx, estx.nodes.cnt, term), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(estx_add_term(&estx, term, term2), 0);

	estx_free(&estx);

	END;
}

TEST(estx_print)
{
	START;

	EXPECT_EQ(estx_print(NULL, DST_NONE()), 0);

	END;
}

TEST(estx_print_tree)
{
	START;

	EXPECT_EQ(estx_print_tree(NULL, DST_NONE()), 0);

	END;
}

TEST(estx_print_rule)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 3, ALLOC_STD);

	estx_node_t rule, terms, term;

	estx_rule(&estx, STRV("rule"), &rule);
	estx_term_rule(&estx, rule, ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP, &terms);
	estx_term_rule(&estx, rule, ESTX_TERM_OCC_OPT, &term);
	estx_add_term(&estx, terms, term);
	estx_term_rule(&estx, rule, ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, terms, term);
	estx_add_term(&estx, rule, terms);

	char buf[64] = {0};
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 23);
	EXPECT_STR(buf, "rule = rule*rule?rule+\n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 49);
	EXPECT_STR(buf,
		   "<rule>\n"
		   "├─<rule>*\n"
		   "├─<rule>?\n"
		   "└─<rule>+\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_lit)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 3, ALLOC_STD);

	estx_node_t rule, terms, term;

	estx_rule(&estx, STRV("lit"), &rule);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP, &terms);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_OPT, &term);
	estx_add_term(&estx, terms, term);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, terms, term);
	estx_add_term(&estx, rule, terms);
	estx_term_lit(&estx, STRV("'"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);

	char buf[64] = {0};
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 22);
	EXPECT_STR(buf, "lit = 'a'*'a'?'a'+\"'\"\n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 49);
	EXPECT_STR(buf,
		   "<lit>\n"
		   "├─'a'*\n"
		   "├─'a'?\n"
		   "├─'a'+\n"
		   "└─\"'\"\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_tok)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 3, ALLOC_STD);

	estx_node_t rule, terms, term;

	estx_rule(&estx, STRV("tok"), &rule);
	estx_term_tok(&estx, TOK_UNKNOWN, ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP, &terms);
	estx_term_tok(&estx, TOK_UNKNOWN, ESTX_TERM_OCC_OPT, &term);
	estx_add_term(&estx, terms, term);
	estx_term_tok(&estx, TOK_UNKNOWN, ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, terms, term);
	estx_add_term(&estx, rule, terms);

	char buf[64] = {0};
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 31);
	EXPECT_STR(buf, "tok = UNKNOWN*UNKNOWN?UNKNOWN+\n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 51);
	EXPECT_STR(buf,
		   "<tok>\n"
		   "├─UNKNOWN*\n"
		   "├─UNKNOWN?\n"
		   "└─UNKNOWN+\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_alt)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 3, ALLOC_STD);

	estx_node_t rule, terms, term;

	estx_rule(&estx, STRV("alt"), &rule);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &terms);
	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_term_alt(&estx, terms, &term);
	estx_add_term(&estx, rule, term);

	char buf[64] = {0};
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 16);
	EXPECT_STR(buf, "alt = 'a' | 'b'\n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 40);
	EXPECT_STR(buf,
		   "<alt>\n"
		   "└─alt\n"
		   "  ├─'a'\n"
		   "  └─'b'\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_con)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 3, ALLOC_STD);

	estx_node_t rule, terms, term;

	estx_rule(&estx, STRV("con"), &rule);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &terms);
	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_term_con(&estx, terms, &term);
	estx_add_term(&estx, rule, term);

	char buf[64] = {0};
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 14);
	EXPECT_STR(buf, "con = 'a' 'b'\n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 40);
	EXPECT_STR(buf,
		   "<con>\n"
		   "└─con\n"
		   "  ├─'a'\n"
		   "  └─'b'\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_group)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 3, ALLOC_STD);

	estx_node_t rule, terms, term;

	estx_rule(&estx, STRV("group"), &rule);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &term);
	estx_term_group(&estx, term, ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP, &terms);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &term);
	estx_term_group(&estx, term, ESTX_TERM_OCC_OPT, &term);
	estx_add_term(&estx, terms, term);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &term);
	estx_term_group(&estx, term, ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, terms, term);
	estx_add_term(&estx, rule, terms);

	char buf[128] = {0};
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 27);
	EXPECT_STR(buf, "group = ('a')*('a')?('a')+\n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 87);
	EXPECT_STR(buf,
		   "<group>\n"
		   "├─group*\n"
		   "│ └─'a'\n"
		   "├─group?\n"
		   "│ └─'a'\n"
		   "└─group+\n"
		   "  └─'a'\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_empty_rules)
{
	START;

	estx_t estx = {0};

	estx_init(&estx, 1, ALLOC_STD);

	estx_rule(&estx, STRV("rule"), NULL);
	estx_rule(&estx, STRV("rule"), NULL);

	char buf[32] = {0};
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 16);
	EXPECT_STR(buf,
		   "rule = \n"
		   "rule = \n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 15);
	EXPECT_STR(buf,
		   "<rule>\n"
		   "\n"
		   "<rule>\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_invalid_rule)
{
	START;

	estx_t estx = {0};

	estx_init(&estx, 2, ALLOC_STD);

	estx_node_t rule, term;
	estx_rule(&estx, STRV("rule"), &rule);

	estx_term_tok(&estx, TOK_UNKNOWN, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);
	*estx_get_node(&estx, term) = (estx_node_data_t){
		.type	  = ESTX_TERM_RULE,
		.val.rule = estx.nodes.cnt,
	};

	char buf[64] = {0};

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 8);
	EXPECT_STR(buf, "rule = \n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 7);
	EXPECT_STR(buf, "<rule>\n");
	log_set_quiet(0, 0);

	estx_free(&estx);

	END;
}

TEST(estx_print_invalid_alt)
{
	START;

	estx_t estx = {0};

	estx_init(&estx, 2, ALLOC_STD);

	estx_node_t rule, term;
	estx_rule(&estx, STRV("rule"), &rule);

	estx_term_tok(&estx, TOK_UNKNOWN, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);
	*estx_get_node(&estx, term) = (estx_node_data_t){
		.type	   = ESTX_TERM_ALT,
		.val.terms = estx.nodes.cnt,
	};

	char buf[64] = {0};

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 8);
	EXPECT_STR(buf, "rule = \n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 17);
	EXPECT_STR(buf,
		   "<rule>\n"
		   "└─alt\n");
	log_set_quiet(0, 0);

	estx_free(&estx);

	END;
}

TEST(estx_print_unknown_term)
{
	START;

	estx_t estx = {0};

	estx_init(&estx, 2, ALLOC_STD);

	estx_node_t rule, term;
	estx_rule(&estx, STRV("rule"), &rule);

	estx_term_tok(&estx, TOK_UNKNOWN, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);
	*estx_get_node(&estx, term) = (estx_node_data_t){
		.type = ESTX_UNKNOWN,
	};

	char buf[64] = {0};

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 8);
	EXPECT_STR(buf, "rule = \n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 7);
	EXPECT_STR(buf, "<rule>\n");
	log_set_quiet(0, 0);

	estx_free(&estx);

	END;
}

STEST(estx)
{
	SSTART;

	RUN(estx_init_free);
	RUN(estx_rule);
	RUN(estx_rule_oom);
	RUN(estx_term_rule);
	RUN(estx_term_tok);
	RUN(estx_term_lit);
	RUN(estx_term_lit_oom);
	RUN(estx_term_alt);
	RUN(estx_term_con);
	RUN(estx_term_group);
	RUN(estx_find_rule);
	RUN(estx_get_node);
	RUN(estx_data_lit);
	RUN(estx_add_term);
	RUN(estx_print);
	RUN(estx_print_tree);
	RUN(estx_print_rule);
	RUN(estx_print_lit);
	RUN(estx_print_tok);
	RUN(estx_print_alt);
	RUN(estx_print_con);
	RUN(estx_print_group);
	RUN(estx_print_empty_rules);
	RUN(estx_print_invalid_rule);
	RUN(estx_print_invalid_alt);
	RUN(estx_print_unknown_term);

	SEND;
}
