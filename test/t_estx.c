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
	EXPECT_EQ(estx.strs.used, 4);

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
	EXPECT_EQ(estx.strs.used, 3);

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
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t term;

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
	estx_init(&estx, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	estx_node_t term;

	EXPECT_EQ(estx_term_group(NULL, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(estx_term_group(&estx, ESTX_TERM_OCC_ONE, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(estx_term_group(&estx, ESTX_TERM_OCC_ONE, &term), 0);
	EXPECT_EQ(term, 0);

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

	estx_node_t term;
	estx_term_rule(&estx, rule, ESTX_TERM_OCC_ONE, &term);

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_add_term(NULL, 0, term), 1);
	EXPECT_EQ(estx_add_term(&estx, estx.nodes.cnt, term), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(estx_add_term(&estx, term, term), 0);

	estx_free(&estx);

	END;
}

TEST(estx_print)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 16, ALLOC_STD);

	estx_node_t frule, lrule;
	estx_node_t file, line;
	estx_rule(&estx, STRV("file"), &frule);
	estx_term_con(&estx, &file);
	estx_add_term(&estx, frule, file);
	estx_rule(&estx, STRV("line"), &lrule);
	estx_term_con(&estx, &line);
	estx_add_term(&estx, lrule, line);

	estx_node_t term;
	estx_term_rule(&estx, lrule, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, file, term);

	estx_term_tok(&estx, TOK_UNKNOWN, ESTX_TERM_OCC_OPT, &term);
	estx_add_term(&estx, line, term);
	estx_term_tok(&estx, TOK_ALPHA, ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, line, term);
	estx_term_lit(&estx, STRV(";"), ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, line, term);
	estx_term_lit(&estx, STRV("'"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, line, term);

	estx_node_t group;
	estx_term_group(&estx, ESTX_TERM_OCC_ONE, &group);
	estx_add_term(&estx, line, group);
	estx_node_t a, b;
	estx_term_lit(&estx, STRV("A"), ESTX_TERM_OCC_ONE, &a);
	estx_term_lit(&estx, STRV("B"), ESTX_TERM_OCC_ONE, &b);
	estx_node_t alt;
	estx_term_alt(&estx, &alt);
	estx_add_term(&estx, group, alt);
	estx_add_term(&estx, alt, a);
	estx_add_term(&estx, alt, b);

	char buf[256] = {0};
	EXPECT_EQ(estx_print(NULL, DST_BUF(buf)), 0);

	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 58);
	EXPECT_STR(buf,
		   "file = line\n"
		   "line = UNKNOWN? ALPHA+ ';'* \"'\" ( 'A' | 'B' )\n");

	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 177);
	EXPECT_STR(buf,
		   "<file>\n"
		   "rule\n"
		   "└─con\n"
		   "  └─<line>\n"
		   "\n"
		   "<line>\n"
		   "rule\n"
		   "└─con\n"
		   "  ├─UNKNOWN?\n"
		   "  ├─ALPHA+\n"
		   "  ├─';'*\n"
		   "  ├─\"'\"\n"
		   "  └─group\n"
		   "    └─alt\n"
		   "      ├─'A'\n"
		   "      └─'B'\n");

	estx_term_con(&estx, &term);
	estx_add_term(&estx, line, term);
	estx_node_data_t *data = estx_get_node(&estx, term);
	data->type	       = -1;

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 58);
	log_set_quiet(0, 0);
	EXPECT_STR(buf,
		   "file = line\n"
		   "line = UNKNOWN? ALPHA+ ';'* \"'\" ( 'A' | 'B' )\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_tree)
{
	START;

	estx_t estx = {0};
	estx_init(&estx, 3, ALLOC_STD);

	estx_node_t rule;
	estx_node_t file;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_term_con(&estx, &file);
	estx_add_term(&estx, rule, file);

	estx_node_t term;
	estx_term_con(&estx, &term);
	estx_add_term(&estx, file, term);
	estx_node_data_t *data = estx_get_node(&estx, term);
	data->type	       = -1;

	char buf[64] = {0};
	EXPECT_EQ(estx_print_tree(NULL, DST_BUF(buf)), 0);
	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 31);
	log_set_quiet(0, 0);

	estx_free(&estx);

	END;
}

TEST(estx_print_empty_rule)
{
	START;

	estx_t estx = {0};

	estx_init(&estx, 1, ALLOC_STD);

	estx_node_t file;
	estx_rule(&estx, STRV("file"), &file);

	char buf[16] = {0};

	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 7);
	EXPECT_STR(buf, "file =\n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 12);
	EXPECT_STR(buf,
		   "<file>\n"
		   "rule\n");

	estx_free(&estx);

	END;
}

TEST(estx_print_invalid_rule)
{
	START;

	estx_t estx = {0};

	estx_init(&estx, 2, ALLOC_STD);

	estx_node_t file, term;
	estx_rule(&estx, STRV("file"), &file);

	estx_term_lit(&estx, STRV(""), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, file, term);
	*estx_get_node(&estx, term) = (estx_node_data_t){
		.type	  = ESTX_TERM_RULE,
		.val.rule = -1,
	};

	char buf[16] = {0};

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_print(&estx, DST_BUF(buf)), 7);
	EXPECT_STR(buf, "file =\n");
	EXPECT_EQ(estx_print_tree(&estx, DST_BUF(buf)), 13);
	EXPECT_STR(buf,
		   "<file>\n"
		   "rule\n"
		   "\n");
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
	RUN(estx_print_empty_rule);
	RUN(estx_print_invalid_rule);

	SEND;
}
