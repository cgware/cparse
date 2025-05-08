#include "parser.h"

#include "bnf.h"
#include "log.h"
#include "mem.h"
#include "test.h"

TEST(prs_init_free)
{
	START;

	prs_t prs = {0};

	EXPECT_EQ(prs_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(prs_init(&prs, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(prs_init(&prs, 0, ALLOC_STD), &prs);
	log_set_quiet(0, 0);

	EXPECT_NE(prs.nodes.data, NULL);

	prs_free(&prs);
	prs_free(NULL);

	EXPECT_EQ(prs.nodes.data, NULL);

	END;
}

TEST(prs_node_rule)
{
	START;

	prs_t prs = {0};
	log_set_quiet(0, 1);
	prs_init(&prs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	prs_node_t node;

	EXPECT_EQ(prs_node_rule(NULL, 0, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(prs_node_rule(&prs, 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(prs_node_rule(&prs, 0, &node), 0);
	EXPECT_EQ(node, 0);

	prs_free(&prs);

	END;
}

TEST(prs_node_tok)
{
	START;

	prs_t prs = {0};
	log_set_quiet(0, 1);
	prs_init(&prs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	prs_node_t node;

	EXPECT_EQ(prs_node_tok(NULL, (token_t){0}, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(prs_node_tok(&prs, (token_t){0}, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(prs_node_tok(&prs, (token_t){0}, &node), 0);
	EXPECT_EQ(node, 0);

	prs_free(&prs);

	END;
}

TEST(prs_node_lit)
{
	START;

	prs_t prs = {0};
	log_set_quiet(0, 1);
	prs_init(&prs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	prs_node_t node;

	EXPECT_EQ(prs_node_lit(NULL, 0, 0, NULL), 1);
	mem_oom(1);
	EXPECT_EQ(prs_node_lit(&prs, 0, 0, NULL), 1);
	mem_oom(0);
	EXPECT_EQ(prs_node_lit(&prs, 0, 0, &node), 0);
	EXPECT_EQ(node, 0);

	prs_free(&prs);

	END;
}

TEST(prs_add_node)
{
	START;

	prs_t prs = {0};
	log_set_quiet(0, 1);
	prs_init(&prs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	prs_node_t parent, child;

	prs_node_rule(&prs, 0, &parent);
	prs_node_rule(&prs, 0, &child);

	EXPECT_EQ(prs_add_node(NULL, prs.nodes.cnt, prs.nodes.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(prs_add_node(&prs, prs.nodes.cnt, prs.nodes.cnt), 1);
	EXPECT_EQ(prs_add_node(&prs, parent, prs.nodes.cnt), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(prs_add_node(&prs, parent, child), 0);

	prs_free(&prs);

	END;
}

TEST(prs_remove_node)
{
	START;

	prs_t prs = {0};
	log_set_quiet(0, 1);
	prs_init(&prs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	prs_node_t node;
	prs_node_rule(&prs, 0, &node);

	EXPECT_EQ(prs_remove_node(NULL, prs.nodes.cnt), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(prs_remove_node(&prs, prs.nodes.cnt), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(prs_remove_node(&prs, node), 0);

	prs_free(&prs);

	END;
}

TEST(prs_get_rule)
{
	START;

	prs_t prs = {0};
	prs_init(&prs, 1, ALLOC_STD);

	prs_node_t root, node, got;
	prs_node_lit(&prs, 0, 0, &root);

	prs_node_lit(&prs, 0, 0, &node);
	prs_add_node(&prs, root, node);
	prs_node_rule(&prs, 0, &node);
	prs_add_node(&prs, root, node);
	prs_node_rule(&prs, 1, &node);
	prs_add_node(&prs, root, node);

	EXPECT_EQ(prs_get_rule(NULL, prs.nodes.cnt, 0, NULL), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(prs_get_rule(&prs, prs.nodes.cnt, 0, NULL), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(prs_get_rule(&prs, root, prs.nodes.cnt, NULL), 1);
	EXPECT_EQ(prs_get_rule(&prs, root, 1, &got), 0);
	EXPECT_EQ(got, node);

	prs_free(&prs);

	END;
}

TEST(prs_get_str)
{
	START;

	prs_t prs = {0};
	prs_init(&prs, 1, ALLOC_STD);

	prs_node_t root, node;
	prs_node_rule(&prs, 0, &root);

	prs_node_lit(&prs, 0, 0, &node);
	prs_add_node(&prs, root, node);
	prs_node_tok(&prs, (token_t){0}, &node);
	prs_add_node(&prs, root, node);
	prs_node_rule(&prs, 1, &node);
	prs_add_node(&prs, root, node);
	prs_node_rule(&prs, 0, &node);
	prs_add_node(&prs, root, node);
	*(int *)tree_get(&prs.nodes, node) = 0;

	EXPECT_EQ(prs_get_str(NULL, prs.nodes.cnt, NULL), 1);
	EXPECT_EQ(prs_get_str(&prs, prs.nodes.cnt, NULL), 1);

	token_t str = {0};
	log_set_quiet(0, 1);
	EXPECT_EQ(prs_get_str(&prs, root, &str), 0);
	log_set_quiet(0, 0);

	EXPECT_EQ(str.start, 0);
	EXPECT_EQ(str.len, 0);

	prs_free(&prs);

	END;
}

TEST(prs_parse_gen)
{
	START;

	EXPECT_EQ(prs_parse(NULL, NULL, NULL, 0, NULL, DST_NONE()), 1);

	lex_t lex  = {0};
	strv_t src = STRV("<");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 1, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);
	stx_term_t term;
	stx_term_rule(&stx, -1, &term);
	stx_rule_add_term(&stx, rule, term);
	stx_term_data_t *data = stx_get_term_data(&stx, term);
	data->type	      = -1;

	log_set_quiet(0, 1);
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_NONE()), 1);
	log_set_quiet(0, 0);

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_rule_invalid)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);
	stx_term_t term;
	stx_term_rule(&stx, -1, &term);
	stx_rule_add_term(&stx, rule, term);

	log_set_quiet(0, 1);
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_NONE()), 1);
	log_set_quiet(0, 0);

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_rule)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV(" ");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t line, rule;
	stx_term_t term;

	stx_add_rule(&stx, &line);
	stx_term_lit(&stx, STRV(" "), &term);
	stx_rule_add_term(&stx, line, term);

	stx_add_rule(&stx, &rule);
	stx_term_rule(&stx, line, &term);
	stx_rule_add_term(&stx, rule, term);

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_NONE()), 0);

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_token_unexpected)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);
	stx_term_t term;
	stx_term_tok(&stx, TOKEN_ALPHA, &term);
	stx_rule_add_term(&stx, rule, term);

	char buf[256] = {0};
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:") - 1,
		   "0: error: expected ALPHA\n"
		   "1\n"
		   "^\n");

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_token)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("A");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);
	stx_term_t term;
	stx_term_tok(&stx, TOKEN_ALPHA, &term);
	stx_rule_add_term(&stx, rule, term);

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_NONE()), 0);

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_literal_unexpected_end)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);
	stx_term_t term;
	stx_term_lit(&stx, STRV("123"), &term);
	stx_rule_add_term(&stx, rule, term);

	char buf[256] = {0};
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:") - 1,
		   "1: error: expected '123'\n"
		   "1\n"
		   " ^\n");

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_literal_unexpected)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("13");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);
	stx_term_t term;
	stx_term_lit(&stx, STRV("123"), &term);
	stx_rule_add_term(&stx, rule, term);

	char buf[256] = {0};
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:") - 1,
		   "1: error: expected '123'\n"
		   "13\n"
		   " ^\n");

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_literal)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);
	stx_term_t term;
	stx_term_lit(&stx, STRV("1"), &term);
	stx_rule_add_term(&stx, rule, term);

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_NONE()), 0);

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_or_l)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);

	stx_term_t term, a, b;
	stx_term_lit(&stx, STRV("a"), &a);
	stx_term_lit(&stx, STRV("b"), &b);
	stx_term_or(&stx, a, b, &term);
	stx_rule_add_term(&stx, rule, term);

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_NONE()), 0);

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_or_r)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("b");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);

	stx_term_t term, a, b;
	stx_term_lit(&stx, STRV("a"), &a);
	stx_term_lit(&stx, STRV("b"), &b);
	stx_term_or(&stx, a, b, &term);
	stx_rule_add_term(&stx, rule, term);

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_NONE()), 0);

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_or_unexpected)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("c");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	stx_rule_t rule;
	stx_add_rule(&stx, &rule);

	stx_term_t term, a, b;
	stx_term_lit(&stx, STRV("a"), &a);
	stx_term_lit(&stx, STRV("b"), &b);
	stx_term_or(&stx, a, b, &term);
	stx_rule_add_term(&stx, rule, term);

	char buf[256] = {0};
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, DST_BUF(buf)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:") - 1,
		   "0: error: expected 'b'\n"
		   "c\n"
		   "^\n");

	stx_free(&stx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(prs_parse_cache)
{
	START;

	lex_t lex  = {0};
	strv_t src = STRV("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 10, 10, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);

	stx_rule_t file, line, ra;
	stx_add_rule(&stx, &file);
	stx_add_rule(&stx, &line);
	stx_add_rule(&stx, &ra);

	stx_term_t term;
	stx_term_rule(&stx, line, &term);
	stx_rule_add_term(&stx, file, term);
	stx_term_tok(&stx, TOKEN_EOF, &term);
	stx_rule_add_term(&stx, file, term);

	stx_term_rule(&stx, ra, &term);
	stx_rule_add_arr(&stx, line, term);

	stx_term_lit(&stx, STRV("a"), &term);
	stx_rule_add_term(&stx, ra, term);

	prs_node_t root;
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, file, &root, DST_NONE()), 0);

	char buf[64] = {0};
	EXPECT_EQ(prs_print(&prs, root, DST_BUF(buf)), 50);
	EXPECT_STR(buf,
		   "0\n"
		   "├─1\n"
		   "│ └─2\n"
		   "│   └─'a'\n"
		   "└─EOF()\n");

	prs_free(&prs);
	lex_free(&lex);
	stx_free(&stx);

	END;
}

TEST(prs_parse_bnf)
{
	START;

	bnf_t bnf = {0};
	bnf_init(&bnf, ALLOC_STD);
	bnf_get_stx(&bnf);

	lex_t lex = {0};
	lex_init(&lex, 0, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 256, ALLOC_STD);

	{
		strv_t src = STRV("<file> ::= <");
		lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 1);

		char buf[256] = {0};
		EXPECT_EQ(prs_parse(&prs, &lex, &bnf.stx, bnf.file, NULL, DST_BUF(buf)), 1);

		EXPECT_STR(buf + sizeof(__FILE__ ":000:") - 1,
			   "12: error: expected LOWER\n"
			   "<file> ::= <\n"
			   "            ^\n");
	}

	{
		strv_t src = STRV("<file> ::= ");
		lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 1);

		char buf[256] = {0};
		EXPECT_EQ(prs_parse(&prs, &lex, &bnf.stx, bnf.file, NULL, DST_BUF(buf)), 1);

		EXPECT_STR(buf + sizeof(__FILE__ ":000:") - 1,
			   "11: error: expected '<'\n"
			   "<file> ::= \n"
			   "           ^\n");
	}

	{
		uint line   = __LINE__ + 1;
		strv_t sbnf = STRV("<file>    ::= <bnf> EOF\n"
				   "<bnf>     ::= <rules>\n"
				   "<rules>   ::= <rule> <rules> | <rule>\n"
				   "<rule>    ::= '<' <rname> '>' <spaces> '::=' <space> <expr> NL\n"
				   "<rname>   ::= LOWER <rchars> | LOWER\n"
				   "<rchars>  ::= <rchar> <rchars> | <rchar>\n"
				   "<rchar>   ::= LOWER | '-'\n"
				   "<expr>    ::= <terms> <space> '|' <space> <expr> | <terms>\n"
				   "<terms>   ::= <term> <space> <terms> | <term>\n"
				   "<term>    ::= <literal> | <token> | '<' <rname> '>'\n"
				   "<literal> ::= \"'\" <tdouble> \"'\" | '\"' <tsingle> '\"'\n"
				   "<token>   ::= UPPER <token> | UPPER\n"
				   "<tdouble> ::= <cdouble> <tdouble> | <cdouble>\n"
				   "<tsingle> ::= <csingle> <tsingle> | <csingle>\n"
				   "<cdouble> ::= <char> | '\"'\n"
				   "<csingle> ::= <char> | \"'\"\n"
				   "<char>    ::= ALPHA | DIGIT | SYMBOL | <space>\n"
				   "<spaces>  ::= <space> <spaces> | <space>\n"
				   "<space>   ::= ' '\n");

		lex_tokenize(&lex, sbnf, STRV(__FILE__), line);

		prs_node_t root;
		prs_parse(&prs, &lex, &bnf.stx, bnf.file, &root, DST_STD());
		EXPECT_EQ(root, 0);
		char *buf = mem_alloc(160000);
		EXPECT_EQ(prs_print(&prs, root, DST_BUFN(buf, 160000)), 89752);
		mem_free(buf, 160000);
	}

	prs_free(&prs);
	lex_free(&lex);
	bnf_free(&bnf);

	END;
}

TEST(prs_parse)
{
	SSTART;

	RUN(prs_parse_gen);
	RUN(prs_parse_rule_invalid);
	RUN(prs_parse_rule);
	RUN(prs_parse_token_unexpected);
	RUN(prs_parse_token);
	RUN(prs_parse_literal_unexpected_end);
	RUN(prs_parse_literal_unexpected);
	RUN(prs_parse_literal);
	RUN(prs_parse_or_l);
	RUN(prs_parse_or_r);
	RUN(prs_parse_or_unexpected);
	RUN(prs_parse_cache);
	RUN(prs_parse_bnf);

	SEND;
}

TEST(prs_print)
{
	START;

	prs_t prs = {0};
	prs_init(&prs, 1, ALLOC_STD);

	prs_node_t root, node;
	prs_node_rule(&prs, 0, &root);
	prs_node_tok(&prs, (token_t){0}, &node);
	prs_add_node(&prs, root, node);
	prs_node_lit(&prs, 0, 0, &node);
	prs_add_node(&prs, root, node);
	prs_node_rule(&prs, 0, &node);
	prs_add_node(&prs, root, node);
	*(int *)tree_get(&prs.nodes, node) = 0;

	char buf[64] = {0};
	EXPECT_EQ(prs_print(NULL, prs.nodes.cnt, DST_BUF(buf)), 0);

	EXPECT_EQ(prs_print(&prs, root, DST_BUF(buf)), 33);
	EXPECT_STR(buf,
		   "0\n"
		   "├─UNKNOWN()\n"
		   "├─''\n"
		   "└─");

	prs_free(&prs);

	END;
}

STEST(parser)
{
	SSTART;

	RUN(prs_init_free);
	RUN(prs_node_rule);
	RUN(prs_node_tok);
	RUN(prs_node_lit);
	RUN(prs_add_node);
	RUN(prs_remove_node);
	RUN(prs_get_rule);
	RUN(prs_get_str);
	RUN(prs_parse);
	RUN(prs_print);

	SEND;
}
