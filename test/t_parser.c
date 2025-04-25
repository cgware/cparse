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

TEST(prs_add_node)
{
	START;

	prs_t prs = {0};
	log_set_quiet(0, 1);
	prs_init(&prs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(prs_add(NULL, (prs_node_data_t){.type = PRS_NODE_UNKNOWN}), PRS_NODE_END);
	mem_oom(1);
	EXPECT_EQ(prs_add(&prs, (prs_node_data_t){.type = PRS_NODE_UNKNOWN}), PRS_NODE_END);
	mem_oom(0);

	prs_node_t parent = prs_add(&prs, (prs_node_data_t){.type = PRS_NODE_UNKNOWN});
	prs_node_t child  = prs_add(&prs, (prs_node_data_t){.type = PRS_NODE_UNKNOWN});

	EXPECT_EQ(prs_add_node(NULL, PRS_NODE_END, PRS_NODE_END), PRS_NODE_END);
	EXPECT_EQ(prs_add_node(&prs, PRS_NODE_END, PRS_NODE_END), PRS_NODE_END);
	EXPECT_EQ(prs_add_node(&prs, parent, PRS_NODE_END), PRS_NODE_END);
	EXPECT_EQ(prs_add_node(&prs, parent, child), child);

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

	prs_node_t node = prs_add_node(&prs, PRS_NODE_END, prs_add(&prs, (prs_node_data_t){.type = PRS_NODE_UNKNOWN}));

	EXPECT_EQ(prs_remove_node(NULL, PRS_NODE_END), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(prs_remove_node(&prs, PRS_NODE_END), 1);
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

	prs_node_t root = prs_add_node(&prs, PRS_NODE_END, PRS_NODE_LITERAL(&prs, 0, 0));

	prs_add_node(&prs, root, PRS_NODE_LITERAL(&prs, 0, 0));
	prs_add_node(&prs, root, PRS_NODE_RULE(&prs, 0));
	prs_node_t node = prs_add_node(&prs, root, PRS_NODE_RULE(&prs, 1));

	EXPECT_EQ(prs_get_rule(NULL, PRS_NODE_END, STX_RULE_END), PRS_NODE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(prs_get_rule(&prs, PRS_NODE_END, STX_RULE_END), PRS_NODE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(prs_get_rule(&prs, root, STX_RULE_END), PRS_NODE_END);
	EXPECT_EQ(prs_get_rule(&prs, root, 1), node);

	prs_free(&prs);

	END;
}

TEST(prs_get_str)
{
	START;

	prs_t prs = {0};
	prs_init(&prs, 1, ALLOC_STD);

	prs_node_t root = prs_add_node(&prs, PRS_NODE_END, PRS_NODE_RULE(&prs, 0));

	prs_add_node(&prs, root, PRS_NODE_LITERAL(&prs, 0, 0));
	prs_add_node(&prs, root, PRS_NODE_TOKEN(&prs, (token_t){0}));
	prs_add_node(&prs, root, PRS_NODE_RULE(&prs, 1));
	prs_add_node(&prs, root, prs_add(&prs, (prs_node_data_t){.type = PRS_NODE_UNKNOWN}));

	EXPECT_EQ(prs_get_str(NULL, PRS_NODE_END, NULL), 1);
	EXPECT_EQ(prs_get_str(&prs, PRS_NODE_END, NULL), 1);

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

	EXPECT_EQ(prs_parse(NULL, NULL, NULL, STX_RULE_END, NULL, PRINT_DST_NONE()), 1);

	lex_t lex  = {0};
	strv_t src = STRV("<");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 2);

	stx_t stx = {0};
	stx_init(&stx, 1, 1, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 1, ALLOC_STD);

	stx_rule_t rule = stx_add_rule(&stx);
	stx_rule_add_term(&stx, rule, stx_create_term(&stx, (stx_term_data_t){.type = -1}));

	log_set_quiet(0, 1);
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_NONE()), 1);
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

	stx_rule_t rule = stx_add_rule(&stx);
	stx_rule_add_term(&stx, rule, STX_TERM_RULE(&stx, -1));

	log_set_quiet(0, 1);
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_NONE()), 1);
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

	stx_rule_t line = stx_add_rule(&stx);
	stx_rule_add_term(&stx, line, STX_TERM_LITERAL(&stx, STRV(" ")));

	stx_rule_t rule = stx_add_rule(&stx);
	stx_rule_add_term(&stx, rule, STX_TERM_RULE(&stx, line));

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_NONE()), 0);

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

	stx_rule_t rule = stx_add_rule(&stx);
	stx_rule_add_term(&stx, rule, STX_TERM_TOKEN(&stx, TOKEN_ALPHA));

	char buf[256] = {0};
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

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

	stx_rule_t rule = stx_add_rule(&stx);
	stx_rule_add_term(&stx, rule, STX_TERM_TOKEN(&stx, TOKEN_ALPHA));

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_NONE()), 0);

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

	stx_rule_t rule = stx_add_rule(&stx);
	stx_rule_add_term(&stx, rule, STX_TERM_LITERAL(&stx, STRV("123")));

	char buf[256] = {0};
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

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

	stx_rule_t rule = stx_add_rule(&stx);
	stx_rule_add_term(&stx, rule, STX_TERM_LITERAL(&stx, STRV("123")));

	char buf[256] = {0};
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

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

	stx_rule_t rule = stx_add_rule(&stx);
	stx_rule_add_term(&stx, rule, STX_TERM_LITERAL(&stx, STRV("1")));

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_NONE()), 0);

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

	stx_rule_t rule = stx_add_rule(&stx);

	const stx_term_t a = STX_TERM_LITERAL(&stx, STRV("a"));
	const stx_term_t b = STX_TERM_LITERAL(&stx, STRV("b"));
	stx_rule_add_term(&stx, rule, STX_TERM_OR(&stx, a, b));

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_NONE()), 0);

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

	stx_rule_t rule = stx_add_rule(&stx);

	const stx_term_t a = STX_TERM_LITERAL(&stx, STRV("a"));
	const stx_term_t b = STX_TERM_LITERAL(&stx, STRV("b"));
	stx_rule_add_term(&stx, rule, STX_TERM_OR(&stx, a, b));

	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_NONE()), 0);

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

	stx_rule_t rule = stx_add_rule(&stx);

	const stx_term_t a = STX_TERM_LITERAL(&stx, STRV("a"));
	const stx_term_t b = STX_TERM_LITERAL(&stx, STRV("b"));
	stx_rule_add_term(&stx, rule, STX_TERM_OR(&stx, a, b));

	char buf[256] = {0};
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

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

	const stx_rule_t file = stx_add_rule(&stx);
	const stx_rule_t line = stx_add_rule(&stx);
	const stx_rule_t ra   = stx_add_rule(&stx);

	stx_rule_add_term(&stx, file, STX_TERM_RULE(&stx, line));
	stx_rule_add_term(&stx, file, STX_TERM_TOKEN(&stx, TOKEN_EOF));

	stx_rule_add_arr(&stx, line, STX_TERM_RULE(&stx, ra), STX_TERM_NONE(&stx));

	stx_rule_add_term(&stx, ra, STX_TERM_LITERAL(&stx, STRV("a")));

	prs_node_t root;
	EXPECT_EQ(prs_parse(&prs, &lex, &stx, file, &root, PRINT_DST_NONE()), 0);

	char buf[64] = {0};
	EXPECT_EQ(prs_print(&prs, root, PRINT_DST_BUF(buf, sizeof(buf), 0)), 50);
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
		EXPECT_EQ(prs_parse(&prs, &lex, &bnf.stx, bnf.file, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

		EXPECT_STR(buf + sizeof(__FILE__ ":000:") - 1,
			   "12: error: expected LOWER\n"
			   "<file> ::= <\n"
			   "            ^\n");
	}

	{
		strv_t src = STRV("<file> ::= ");
		lex_tokenize(&lex, src, STRV(__FILE__), __LINE__ - 1);

		char buf[256] = {0};
		EXPECT_EQ(prs_parse(&prs, &lex, &bnf.stx, bnf.file, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

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
		prs_parse(&prs, &lex, &bnf.stx, bnf.file, &root, PRINT_DST_STD());
		EXPECT_EQ(root, 0);
		char *buf = mem_alloc(160000);
		EXPECT_EQ(prs_print(&prs, root, PRINT_DST_BUF(buf, 160000, 0)), 89752);
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

	prs_node_t root = prs_add_node(&prs, PRS_NODE_END, PRS_NODE_RULE(&prs, 0));
	prs_add_node(&prs, root, PRS_NODE_TOKEN(&prs, (token_t){0}));
	prs_add_node(&prs, root, PRS_NODE_LITERAL(&prs, 0, 0));
	prs_add_node(&prs, root, prs_add(&prs, (prs_node_data_t){.type = PRS_NODE_UNKNOWN}));

	char buf[64] = {0};
	EXPECT_EQ(prs_print(NULL, PRS_NODE_END, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	EXPECT_EQ(prs_print(&prs, root, PRINT_DST_BUF(buf, sizeof(buf), 0)), 33);
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
	RUN(prs_add_node);
	RUN(prs_remove_node);
	RUN(prs_get_rule);
	RUN(prs_get_str);
	RUN(prs_parse);
	RUN(prs_print);

	SEND;
}
