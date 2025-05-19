#include "eprs.h"

#include "ebnf.h"
#include "log.h"
#include "mem.h"
#include "test.h"

TEST(eprs_init_free)
{
	START;

	eprs_t eprs = {0};

	EXPECT_EQ(eprs_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(eprs_init(&eprs, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_init(&eprs, 0, ALLOC_STD), &eprs);
	log_set_quiet(0, 0);

	EXPECT_NE(eprs.nodes.data, NULL);

	eprs_free(&eprs);
	eprs_free(NULL);

	EXPECT_EQ(eprs.nodes.data, NULL);

	END;
}

TEST(eprs_reset)
{
	START;

	eprs_t eprs = {0};
	eprs_init(&eprs, 1, ALLOC_STD);

	eprs_node_lit(&eprs, 0, 0, NULL);

	eprs_reset(NULL, 0);
	eprs_reset(&eprs, 0);
	EXPECT_EQ(eprs.nodes.cnt, 0);

	eprs_free(&eprs);

	END;
}

TEST(eprs_node_rule)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	eprs_node_t node;

	EXPECT_EQ(eprs_node_rule(NULL, 0, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(eprs_node_rule(&eprs, 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(eprs_node_rule(&eprs, 0, &node), 0);
	EXPECT_EQ(node, 0);

	eprs_free(&eprs);

	END;
}

TEST(eprs_node_tok)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	eprs_node_t node;

	EXPECT_EQ(eprs_node_tok(NULL, (tok_t){0}, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(eprs_node_tok(&eprs, (tok_t){0}, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(eprs_node_tok(&eprs, (tok_t){0}, &node), 0);
	EXPECT_EQ(node, 0);

	eprs_free(&eprs);

	END;
}

TEST(eprs_node_lit)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	eprs_node_t node;

	EXPECT_EQ(eprs_node_lit(NULL, 0, 0, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(eprs_node_lit(&eprs, 0, 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(eprs_node_lit(&eprs, 0, 0, &node), 0);
	EXPECT_EQ(node, 0);

	eprs_free(&eprs);

	END;
}

TEST(eprs_add_node)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	eprs_node_t parent, child;

	eprs_node_rule(&eprs, 0, &parent);
	eprs_node_rule(&eprs, 0, &child);

	EXPECT_EQ(eprs_add_node(NULL, eprs.nodes.cnt, eprs.nodes.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_add_node(&eprs, eprs.nodes.cnt, eprs.nodes.cnt), 1);
	EXPECT_EQ(eprs_add_node(&eprs, parent, eprs.nodes.cnt), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(eprs_add_node(&eprs, parent, child), 0);

	eprs_free(&eprs);

	END;
}

TEST(eprs_remove_node)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	eprs_node_t node;
	eprs_node_rule(&eprs, 0, &node);

	EXPECT_EQ(eprs_remove_node(NULL, eprs.nodes.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_remove_node(&eprs, eprs.nodes.cnt), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(eprs_remove_node(&eprs, node), 0);

	eprs_free(&eprs);

	END;
}

TEST(eprs_get_rule)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	eprs_node_t root, node;
	eprs_node_lit(&eprs, 0, 0, &root);

	eprs_node_lit(&eprs, 0, 0, &node);
	eprs_add_node(&eprs, root, node);
	eprs_node_rule(&eprs, 0, &node);
	eprs_add_node(&eprs, root, node);

	EXPECT_EQ(eprs_get_rule(NULL, eprs.nodes.cnt, eprs.nodes.cnt, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_get_rule(&eprs, eprs.nodes.cnt, eprs.nodes.cnt, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(eprs_get_rule(&eprs, root, eprs.nodes.cnt, NULL), 1);
	eprs_node_t tmp;
	EXPECT_EQ(eprs_get_rule(&eprs, root, 0, &tmp), 0);
	EXPECT_EQ(tmp, node);
	EXPECT_EQ(eprs_get_rule(&eprs, node, 0, &tmp), 0);
	EXPECT_EQ(tmp, node);

	eprs_free(&eprs);

	END;
}

TEST(eprs_get_str)
{
	START;

	eprs_t eprs = {0};
	eprs_init(&eprs, 1, ALLOC_STD);

	eprs_node_t root, node;
	eprs_node_rule(&eprs, 0, &root);

	eprs_node_lit(&eprs, 0, 0, &node);
	eprs_add_node(&eprs, root, node);
	eprs_node_tok(&eprs, (tok_t){0}, &node);
	eprs_add_node(&eprs, root, node);
	eprs_node_rule(&eprs, 1, &node);
	eprs_add_node(&eprs, root, node);
	eprs_node_rule(&eprs, 0, &node);
	eprs_add_node(&eprs, root, node);
	*(int *)tree_get(&eprs.nodes, node) = 0;

	tok_t str = {0};

	EXPECT_EQ(eprs_get_str(NULL, eprs.nodes.cnt, NULL), 1);
	EXPECT_EQ(eprs_get_str(&eprs, eprs.nodes.cnt, NULL), 1);

	str.len = 0;
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_get_str(&eprs, root, &str), 0);
	log_set_quiet(0, 0);

	EXPECT_EQ(str.start, 0);
	EXPECT_EQ(str.len, 0);

	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_gen)
{
	START;

	EXPECT_EQ(eprs_parse(NULL, NULL, NULL, 0, NULL, DST_NONE()), 1);

	lex_t lex  = {0};
	strv_t src = STRV("<");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 1, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);

	estx_node_t term;
	estx_term_tok(&estx, TOK_NULL, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);
	estx_node_data_t *data = estx_get_node(&estx, term);
	data->type	       = -1;

	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 1);
	log_set_quiet(0, 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_rule_invalid)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_rule(&estx, rule, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);
	estx_get_node(&estx, term)->val.rule = estx.nodes.cnt;

	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 1);
	log_set_quiet(0, 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_rule)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV(" ");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 4, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 4, ALLOC_STD);

	estx_node_t line;
	estx_rule(&estx, STRV("line"), &line);
	estx_node_t term;
	estx_term_lit(&estx, STRV(" "), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, line, term);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_term_rule(&estx, line, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_tok_unexpected)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_tok(&estx, TOK_ALPHA, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected ALPHA\n"
		   "1\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_tok)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("A");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_tok(&estx, TOK_ALPHA, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_literal_unexpected_end)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_lit(&estx, STRV("123"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected '123'\n"
		   "1\n"
		   " ^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_literal_unexpected)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("13");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_lit(&estx, STRV("123"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected '123'\n"
		   "13\n"
		   " ^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_literal)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_lit(&estx, STRV("1"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, rule, term);

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_alt_failed)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("c");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 4, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 4, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t terms, term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &terms);
	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_node_t alt;
	estx_term_alt(&estx, terms, &alt);
	estx_add_term(&estx, rule, alt);

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected 'b'\n"
		   "c\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_alt)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 4, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 4, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t terms, term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &terms);
	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_node_t alt;
	estx_term_alt(&estx, terms, &alt);
	estx_add_term(&estx, rule, alt);

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_con_failed)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("c");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 4, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 4, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t terms, term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &terms);
	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_node_t con;
	estx_term_con(&estx, terms, &con);
	estx_add_term(&estx, rule, con);

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected 'a'\n"
		   "c\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_con)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("ab");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 4, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 4, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t terms, term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &terms);
	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_node_t con;
	estx_term_con(&estx, terms, &con);
	estx_add_term(&estx, rule, con);

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_group_failed)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("c");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 4, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 4, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t terms, term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &terms);
	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_node_t group;
	estx_term_group(&estx, terms, ESTX_TERM_OCC_ONE, &group);
	estx_add_term(&estx, rule, group);

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected 'a'\n"
		   "c\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_group)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("ab");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 4, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 4, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t terms, term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &terms);
	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_node_t group;
	estx_term_group(&estx, terms, ESTX_TERM_OCC_ONE, &group);
	estx_add_term(&estx, rule, group);

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_opt)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_OPT, &term);
	estx_add_term(&estx, rule, term);

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_rep_failed)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, rule, term);

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected 'a'\n"
		   "\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_rep_loop)
{
	START;

	lex_t lex  = {0};
	strv_t bnf = STRV("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, bnf, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 8, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 8, ALLOC_STD);

	estx_node_t file, line, val;
	estx_rule(&estx, STRV("file"), &file);
	estx_rule(&estx, STRV("line"), &line);
	estx_rule(&estx, STRV("val"), &val);

	estx_node_t term;
	estx_term_rule(&estx, line, ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, file, term);
	estx_term_tok(&estx, TOK_EOF, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, file, term);
	estx_term_rule(&estx, val, ESTX_TERM_OCC_OPT, &term);
	estx_add_term(&estx, line, term);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, val, term);

	log_set_quiet(0, 1);
	eprs_node_t root;
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, file, &root, DST_NONE()), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(root, 0);

	eprs_free(&eprs);
	lex_free(&lex);
	estx_free(&estx);

	END;
}

TEST(eprs_parse_rep)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 2, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	estx_node_t rule;
	estx_rule(&estx, STRV("rule"), &rule);
	estx_node_t term;
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, rule, term);

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_name)
{
	START;

	lex_t lex  = {0};
	strv_t bnf = STRV("b");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, bnf, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 8, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 8, ALLOC_STD);

	estx_node_t file, vala, valb;
	estx_rule(&estx, STRV("file"), &file);
	estx_rule(&estx, STRV("vala"), &vala);
	estx_rule(&estx, STRV("valb"), &valb);

	estx_node_t terms, term;
	estx_term_rule(&estx, vala, ESTX_TERM_OCC_ONE, &terms);
	estx_term_rule(&estx, valb, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, terms, term);
	estx_node_t file_alt;
	estx_term_alt(&estx, terms, &file_alt);
	estx_add_term(&estx, file, file_alt);

	estx_term_lit(&estx, STRV("b"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, valb, term);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, vala, term);

	eprs_node_t root;
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, file, &root, DST_NONE()), 0);

	char buf[64] = {0};
	EXPECT_EQ(eprs_print(&eprs, root, DST_BUF(buf)), 22);
	EXPECT_STR(buf,
		   "0\n"
		   "└─2\n"
		   "  └─'b'\n");

	eprs_free(&eprs);
	lex_free(&lex);
	estx_free(&estx);

	END;
}

TEST(eprs_parse_cache)
{
	START;

	lex_t lex  = {0};
	strv_t bnf = STRV("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, bnf, STRV(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 8, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 8, ALLOC_STD);

	estx_node_t file, line, ra;
	estx_rule(&estx, STRV("file"), &file);
	estx_rule(&estx, STRV("line"), &line);
	estx_rule(&estx, STRV("ra"), &ra);

	estx_node_t term;
	estx_term_rule(&estx, line, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, file, term);
	estx_term_tok(&estx, TOK_EOF, ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, file, term);
	estx_term_rule(&estx, ra, ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP, &term);
	estx_add_term(&estx, line, term);
	estx_term_lit(&estx, STRV("a"), ESTX_TERM_OCC_ONE, &term);
	estx_add_term(&estx, ra, term);

	eprs_node_t root;
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, file, &root, DST_NONE()), 0);

	char buf[64] = {0};
	EXPECT_EQ(eprs_print(&eprs, root, DST_BUF(buf)), 34);
	EXPECT_STR(buf,
		   "0\n"
		   "└─1\n"
		   "  └─2\n"
		   "    └─'a'\n");

	eprs_free(&eprs);
	lex_free(&lex);
	estx_free(&estx);

	END;
}

TEST(eprs_parse_ebnf)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, 0, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 2, ALLOC_STD);

	{
		strv_t bnf = STRV("<file> ::= <");
		lex_tokenize(&lex, bnf, STRV(__FILE__), __LINE__ - 1);

		estx_t estx = {0};
		estx_init(&estx, 2, ALLOC_STD);

		estx_node_t rule;
		estx_rule(&estx, STRV("rule"), &rule);

		estx_node_t term;
		estx_term_tok(&estx, TOK_NULL, ESTX_TERM_OCC_ONE, &term);
		estx_add_term(&estx, rule, term);
		estx_node_data_t *data = estx_get_node(&estx, term);
		data->type	       = -1;

		log_set_quiet(0, 1);
		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 1);
		log_set_quiet(0, 0);

		estx_free(&estx);
	}

	{
		strv_t bnf = STRV("<file> ::= <");
		lex_tokenize(&lex, bnf, STRV(__FILE__), __LINE__ - 1);

		estx_t estx = {0};
		estx_init(&estx, 2, ALLOC_STD);

		estx_node_t rule;
		estx_rule(&estx, STRV("rule"), &rule);
		estx_node_t term;
		estx_term_rule(&estx, rule, ESTX_TERM_OCC_ONE, &term);
		estx_get_node(&estx, term)->val.rule = estx.nodes.cnt;
		EXPECT_EQ(estx_add_term(&estx, rule, term), 0);

		log_set_quiet(0, 1);
		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 1);
		log_set_quiet(0, 0);

		estx_free(&estx);
	}

	{
		strv_t bnf = STRV("::");
		lex_tokenize(&lex, bnf, STRV(__FILE__), __LINE__ - 1);

		estx_t estx = {0};
		estx_init(&estx, 2, ALLOC_STD);

		estx_node_t rule;
		estx_rule(&estx, STRV("rule"), &rule);
		estx_node_t term;
		estx_term_lit(&estx, STRV("::="), ESTX_TERM_OCC_ONE, &term);
		estx_add_term(&estx, rule, term);

		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, DST_NONE()), 1);

		estx_free(&estx);
	}

	estx_t estx = {0};
	prs_t prs   = {0};
	estx_node_t estx_root;

	{
		ebnf_t ebnf = {0};
		ebnf_init(&ebnf, ALLOC_STD);
		ebnf_get_stx(&ebnf, ALLOC_STD, DST_NONE());

		uint line   = __LINE__ + 1;
		strv_t sbnf = STRV("file    = ebnf EOF\n"
				   "ebnf    = rule+\n"
				   "rule    = rname spaces '= ' alt NL\n"
				   "rname   = LOWER (LOWER | '_')*\n"
				   "alt     = concat (' | ' concat)*\n"
				   "concat  = factor (' ' factor)*\n"
				   "factor  = term (opt | rep | opt_rep)?\n"
				   "opt     = '?'\n"
				   "rep     = '+'\n"
				   "opt_rep = '*'\n"
				   "term    = literal | token | rname | group\n"
				   "literal = \"'\" (char | '\"')+ \"'\" | '\"' (char | \"'\")+ '\"'\n"
				   "token   = UPPER+\n"
				   "group   = '(' alt ')'\n"
				   "char    = ALPHA | DIGIT | SYMBOL | ' '\n"
				   "spaces  = ' '+\n");

		lex_tokenize(&lex, sbnf, STRV(__FILE__), line);

		prs_init(&prs, 100, ALLOC_STD);
		prs_node_t prs_root;
		prs_parse(&prs, &lex, &ebnf.stx, ebnf.file, &prs_root, DST_NONE());

		estx_init(&estx, 10, ALLOC_STD);
		estx_from_ebnf(&ebnf, &prs, prs_root, &estx, &estx_root);

		ebnf_free(&ebnf);
	}

	{
		strv_t sbnf = STRV("<file> ::= ");
		lex_tokenize(&lex, sbnf, STRV(__FILE__), __LINE__ - 1);

		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, estx_root, NULL, DST_NONE()), 1);
	}

	{
		strv_t sebnf = STRV("");
		lex_tokenize(&lex, sebnf, STRV(__FILE__), __LINE__ - 1);

		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, estx_root, NULL, DST_NONE()), 1);
	}

	{
		uint line   = __LINE__ + 1;
		strv_t sbnf = STRV("file    = ebnf EOF\n"
				   "ebnf    = rule+\n"
				   "rule    = rname spaces '= ' alt NL\n"
				   "rname   = LOWER (LOWER | '_')*\n"
				   "alt     = concat (' | ' concat)*\n"
				   "concat  = factor (' ' factor)*\n"
				   "factor  = term (opt | rep | opt_rep)?\n"
				   "opt     = '?'\n"
				   "rep     = '+'\n"
				   "opt_rep = '*'\n"
				   "term    = literal | token | rname | group\n"
				   "literal = \"'\" (char | '\"')+ \"'\" | '\"' (char | \"'\")+ '\"'\n"
				   "token   = UPPER+\n"
				   "group   = '(' alt ')'\n"
				   "char    = ALPHA | DIGIT | SYMBOL | ' '\n"
				   "spaces  = ' '+\n");

		lex_tokenize(&lex, sbnf, STRV(__FILE__), line);

		eprs_node_t root;
		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, estx_root, &root, DST_NONE()), 0);

		char *buf = mem_alloc(30000);
		EXPECT_EQ(eprs_print(&eprs, root, DST_BUFN(buf, 30000)), 20010);
		mem_free(buf, 30000);
	}

	eprs_free(&eprs);
	estx_free(&estx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(eprs_parse)
{
	SSTART;

	RUN(eprs_parse_gen);
	RUN(eprs_parse_rule_invalid);
	RUN(eprs_parse_rule);
	RUN(eprs_parse_tok_unexpected);
	RUN(eprs_parse_tok);
	RUN(eprs_parse_literal_unexpected_end);
	RUN(eprs_parse_literal_unexpected);
	RUN(eprs_parse_literal);
	RUN(eprs_parse_alt_failed);
	RUN(eprs_parse_alt);
	RUN(eprs_parse_con_failed);
	RUN(eprs_parse_con);
	RUN(eprs_parse_group_failed);
	RUN(eprs_parse_group);
	RUN(eprs_parse_opt);
	RUN(eprs_parse_rep_failed);
	RUN(eprs_parse_rep_loop);
	RUN(eprs_parse_rep);
	RUN(eprs_parse_name);
	RUN(eprs_parse_cache);
	RUN(eprs_parse_ebnf);

	SEND;
}

TEST(eprs_print)
{
	START;

	eprs_t eprs = {0};
	eprs_init(&eprs, 1, ALLOC_STD);

	eprs_node_t root, node;
	eprs_node_rule(&eprs, 0, &root);
	eprs_node_tok(&eprs, (tok_t){0}, &node);
	eprs_add_node(&eprs, root, node);
	eprs_node_lit(&eprs, 0, 0, &node);
	eprs_add_node(&eprs, root, node);
	eprs_node_rule(&eprs, 0, &node);
	eprs_add_node(&eprs, root, node);
	*(int *)tree_get(&eprs.nodes, node) = 0;

	char buf[64] = {0};
	EXPECT_EQ(eprs_print(NULL, eprs.nodes.cnt, DST_BUF(buf)), 0);

	EXPECT_EQ(eprs_print(&eprs, root, DST_BUF(buf)), 33);
	EXPECT_STR(buf,
		   "0\n"
		   "├─UNKNOWN()\n"
		   "├─''\n"
		   "└─");

	eprs_free(&eprs);

	END;
}

STEST(eprs)
{
	SSTART;

	RUN(eprs_init_free);
	RUN(eprs_reset);
	RUN(eprs_node_rule);
	RUN(eprs_node_tok);
	RUN(eprs_node_lit);
	RUN(eprs_add_node);
	RUN(eprs_remove_node);
	RUN(eprs_get_rule);
	RUN(eprs_get_str);
	RUN(eprs_parse);
	RUN(eprs_print);

	SEND;
}
