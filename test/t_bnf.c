#include "bnf.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(bnf_init_free)
{
	START;

	bnf_t bnf = {0};

	EXPECT_EQ(bnf_init(NULL, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(bnf_init(&bnf, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(bnf_init(&bnf, ALLOC_STD), &bnf);

	bnf_free(NULL);
	bnf_free(&bnf);

	END;
}

TEST(bnf_get_stx)
{
	START;

	bnf_t bnf = {0};
	bnf_init(&bnf, ALLOC_STD);

	EXPECT_EQ(bnf_get_stx(NULL), NULL);
	EXPECT_NE(bnf_get_stx(&bnf), NULL);

	char buf[1024] = {0};
	EXPECT_EQ(stx_print(&bnf.stx, DST_BUF(buf)), 715);
	EXPECT_STR(buf,
		   "<file> ::= <bnf> EOF\n"
		   "<bnf> ::= <rules>\n"
		   "<rules> ::= <rule> <rules> | <rule>\n"
		   "<rule> ::= '<' <rname> '>' <spaces> '::=' <space> <expr> NL\n"
		   "<rname> ::= LOWER <rchars> | LOWER\n"
		   "<rchars> ::= <rchar> <rchars> | <rchar>\n"
		   "<rchar> ::= LOWER | '-'\n"
		   "<expr> ::= <terms> <space> '|' <space> <expr> | <terms>\n"
		   "<terms> ::= <term> <space> <terms> | <term>\n"
		   "<term> ::= <literal> | <token> | '<' <rname> '>'\n"
		   "<literal> ::= \"'\" <tdouble> \"'\" | '\"' <tsingle> '\"'\n"
		   "<token> ::= UPPER <token> | UPPER\n"
		   "<tdouble> ::= <cdouble> <tdouble> | <cdouble>\n"
		   "<tsingle> ::= <csingle> <tsingle> | <csingle>\n"
		   "<cdouble> ::= <char> | '\"'\n"
		   "<csingle> ::= <char> | \"'\"\n"
		   "<char> ::= ALPHA | DIGIT | SYMBOL | <space>\n"
		   "<spaces> ::= <space> <spaces> | <space>\n"
		   "<space> ::= ' '\n");

	bnf_free(&bnf);

	END;
}

TEST(stx_from_bnf)
{
	START;

	bnf_t bnf = {0};
	bnf_init(&bnf, ALLOC_STD);
	bnf_get_stx(&bnf);

	uint line   = __LINE__ + 1;
	strv_t sbnf = STRV("<file>        ::= <bnf> EOF\n"
			   "<bnf>         ::= <rules>\n"
			   "<rules>       ::= <rule> <rules> | <rule>\n"
			   "<rule>        ::= '<' <rule-name> '>' <spaces> '::=' <space> <expression> NL\n"
			   "<rule-name>   ::= LOWER <rule-chars> | LOWER\n"
			   "<spaces>      ::= <space> <spaces> | <space>\n"
			   "<space>       ::= ' '\n"
			   "<expression>  ::= <terms> <space> '|' <space> <expression> | <terms>\n"
			   "<rule-chars>  ::= <rule-char> <rule-chars> | <rule-char>\n"
			   "<terms>       ::= <term> <terms> | <term>\n"
			   "<rule-char>   ::= LOWER | '-'\n"
			   "<term>        ::= <literal> | <token> | '<' <rule-name> '>'\n"
			   "<literal>     ::= \"'\" <text-double> \"'\" | '\"' <text-single> '\"'\n"
			   "<token>       ::= UPPER <token> | UPPER\n"
			   "<text-double> ::= <char-double> <text-double> | <char-double>\n"
			   "<text-single> ::= <char-single> <text-single> | <char-single>\n"
			   "<char-double> ::= <character> | '\"'\n"
			   "<char-single> ::= <character> | \"'\"\n"
			   "<character>   ::= ALPHA | DIGIT | SYMBOL | ' '\n");

	lex_t lex = {0};
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, sbnf, STRV(__FILE__), line);

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);
	prs_node_t prs_root;
	prs_parse(&prs, &lex, &bnf.stx, bnf.file, &prs_root, DST_STD());

	stx_t new_stx = {0};
	stx_init(&new_stx, 10, ALLOC_STD);
	stx_node_t root;
	EXPECT_EQ(stx_from_bnf(NULL, &prs, prs_root, &new_stx, &root), 1);
	EXPECT_EQ(stx_from_bnf(&bnf, &prs, prs_root, &new_stx, &root), 0);
	EXPECT_EQ(root, 0);

	char buf[1024] = {0};
	stx_print(&new_stx, DST_BUF(buf));
	EXPECT_STR(buf,
		   "<file> ::= <bnf> EOF\n"
		   "<bnf> ::= <rules>\n"
		   "<rules> ::= <rule> <rules> | <rule>\n"
		   "<rule> ::= '<' <rule-name> '>' <spaces> '::=' <space> <expression> NL\n"
		   "<rule-name> ::= LOWER <rule-chars> | LOWER\n"
		   "<spaces> ::= <space> <spaces> | <space>\n"
		   "<space> ::= ' '\n"
		   "<expression> ::= <terms> <space> '|' <space> <expression> | <terms>\n"
		   "<rule-chars> ::= <rule-char> <rule-chars> | <rule-char>\n"
		   "<terms> ::= <term> <terms> | <term>\n"
		   "<rule-char> ::= LOWER | '-'\n"
		   "<term> ::= <literal> | <token> | '<' <rule-name> '>'\n"
		   "<literal> ::= \"'\" <text-double> \"'\" | '\"' <text-single> '\"'\n"
		   "<token> ::= UPPER <token> | UPPER\n"
		   "<text-double> ::= <char-double> <text-double> | <char-double>\n"
		   "<text-single> ::= <char-single> <text-single> | <char-single>\n"
		   "<char-double> ::= <character> | '\"'\n"
		   "<char-single> ::= <character> | \"'\"\n"
		   "<character> ::= ALPHA | DIGIT | SYMBOL | ' '\n");

	uint file, bnfr, rules;
	stx_find_rule(&new_stx, STRV("file"), &file);
	stx_find_rule(&new_stx, STRV("bnf"), &bnfr);
	stx_find_rule(&new_stx, STRV("rules"), &rules);

	EXPECT_EQ(file, 0);
	EXPECT_EQ(bnfr, 1);
	EXPECT_EQ(rules, 4);

	lex_free(&lex);
	prs_free(&prs);
	stx_free(&new_stx);
	bnf_free(&bnf);

	END;
}

TEST(stx_from_bnf_custom)
{
	START;

	bnf_t bnf = {0};
	bnf_init(&bnf, ALLOC_STD);
	bnf_get_stx(&bnf);

	stx_t new_stx = {0};
	stx_init(&new_stx, 10, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);
	prs.stx = &bnf.stx;

	prs_node_t file, pbnf, rules, rule, expr, terms0, term0, terms1, node;
	prs_node_rule(&prs, bnf.file, &file);
	prs_node_rule(&prs, bnf.bnf, &pbnf);
	prs_add_node(&prs, file, pbnf);
	prs_node_rule(&prs, bnf.rules, &rules);
	prs_add_node(&prs, pbnf, rules);
	prs_node_rule(&prs, bnf.rule, &rule);
	prs_add_node(&prs, rules, rule);
	prs_node_rule(&prs, bnf.expr, &expr);
	prs_add_node(&prs, rule, expr);
	prs_node_rule(&prs, bnf.terms, &terms0);
	prs_add_node(&prs, expr, terms0);
	prs_node_rule(&prs, bnf.term, &term0);
	prs_add_node(&prs, terms0, term0);
	prs_node_rule(&prs, bnf.literal, &node);
	prs_add_node(&prs, term0, node);
	prs_node_rule(&prs, bnf.terms, &terms1);
	prs_add_node(&prs, terms0, terms1);
	prs_node_rule(&prs, bnf.term, &node);
	prs_add_node(&prs, terms1, node);

	stx_node_t root;

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_from_bnf(&bnf, &prs, file, &new_stx, &root), 0);
	EXPECT_EQ(root, 0);
	log_set_quiet(0, 0);

	prs_free(&prs);
	stx_free(&new_stx);
	bnf_free(&bnf);

	END;
}

STEST(bnf)
{
	SSTART;

	RUN(bnf_init_free);
	RUN(bnf_get_stx);
	RUN(stx_from_bnf);
	RUN(stx_from_bnf_custom);

	SEND;
}
