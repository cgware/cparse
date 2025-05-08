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

	char buf[512] = {0};
	EXPECT_EQ(stx_print(&bnf.stx, DST_BUF(buf)), 493);
	EXPECT_STR(buf,
		   "<0> ::= <1> EOF\n"
		   "<1> ::= <2>\n"
		   "<2> ::= <3> <2> | <3>\n"
		   "<3> ::= '<' <4> '>' <17> '::=' <18> <7> NL\n"
		   "<4> ::= LOWER <5> | LOWER\n"
		   "<5> ::= <6> <5> | <6>\n"
		   "<6> ::= LOWER | '-'\n"
		   "<7> ::= <8> <18> '|' <18> <7> | <8>\n"
		   "<8> ::= <9> <18> <8> | <9>\n"
		   "<9> ::= <10> | <11> | '<' <4> '>'\n"
		   "<10> ::= \"'\" <12> \"'\" | '\"' <13> '\"'\n"
		   "<11> ::= UPPER <11> | UPPER\n"
		   "<12> ::= <14> <12> | <14>\n"
		   "<13> ::= <15> <13> | <15>\n"
		   "<14> ::= <16> | '\"'\n"
		   "<15> ::= <16> | \"'\"\n"
		   "<16> ::= ALPHA | DIGIT | SYMBOL | <18>\n"
		   "<17> ::= <18> <17> | <18>\n"
		   "<18> ::= ' '\n");

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
			   "<rule-chars>  ::= <rule-char> <rule-chars> | <rule-char>\n"
			   "<rule-char>   ::= LOWER | '-'\n"
			   "<expression>  ::= <terms> <space> '|' <space> <expression> | <terms>\n"
			   "<terms>       ::= <term> <terms> | <term>\n"
			   "<term>        ::= <literal> | <token> | '<' <rule-name> '>'\n"
			   "<literal>     ::= \"'\" <text-double> \"'\" | '\"' <text-single> '\"'\n"
			   "<token>       ::= UPPER <token> | UPPER\n"
			   "<text-double> ::= <char-double> <text-double> | <char-double>\n"
			   "<text-single> ::= <char-single> <text-single> | <char-single>\n"
			   "<char-double> ::= <character> | '\"'\n"
			   "<char-single> ::= <character> | \"'\"\n"
			   "<character>   ::= ALPHA | DIGIT | SYMBOL | ' '\n"
			   "<spaces>      ::= <space> <spaces> | <space>\n"
			   "<space>       ::= ' '\n");

	lex_t lex = {0};
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, sbnf, STRV(__FILE__), line);

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);
	prs_node_t prs_root;
	prs_parse(&prs, &lex, &bnf.stx, bnf.file, &prs_root, DST_STD());

	strbuf_t names = {0};
	strbuf_init(&names, 16, 16, ALLOC_STD);

	stx_t new_stx = {0};
	stx_init(&new_stx, 10, 10, ALLOC_STD);
	stx_rule_t root;
	EXPECT_EQ(stx_from_bnf(NULL, &prs, prs_root, &new_stx, &names, &root), 0);
	EXPECT_EQ(stx_from_bnf(&bnf, &prs, prs_root, &new_stx, &names, &root), 0);
	EXPECT_EQ(root, 0);

	uint file, bnfr, rules;
	strbuf_find(&names, STRV("file"), &file);
	strbuf_find(&names, STRV("bnf"), &bnfr);
	strbuf_find(&names, STRV("rules"), &rules);

	EXPECT_EQ(file, 0);
	EXPECT_EQ(bnfr, 1);
	EXPECT_EQ(rules, 2);

	strbuf_free(&names);

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
	stx_init(&new_stx, 10, 10, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);

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

	strbuf_t names = {0};
	strbuf_init(&names, 16, 16, ALLOC_STD);

	stx_rule_t root;

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_from_bnf(&bnf, &prs, file, &new_stx, &names, &root), 0);
	EXPECT_EQ(root, 0);
	log_set_quiet(0, 0);

	strbuf_free(&names);

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
