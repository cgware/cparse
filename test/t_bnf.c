#include "bnf.h"

#include "cstr.h"
#include "log.h"
#include "mem.h"
#include "test.h"

TEST(bnf_init_free)
{
	START;

	bnf_t bnf = {0};

	EXPECT_EQ(bnf_init(NULL), NULL);
	EXPECT_EQ(bnf_init(&bnf), &bnf);

	// bnf_free(&bnf);
	bnf_free(NULL);

	END;
}

TEST(bnf_get_stx)
{
	START;

	bnf_t bnf = {0};

	EXPECT_EQ(bnf_get_stx(NULL, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(bnf_get_stx(&bnf, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_NE(bnf_get_stx(&bnf, ALLOC_STD), NULL);

	char buf[512] = {0};
	EXPECT_EQ(stx_print(&bnf.stx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 493);
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
	bnf_get_stx(&bnf, ALLOC_STD);

	uint line  = __LINE__ + 1;
	str_t sbnf = STR("<file>        ::= <bnf> EOF\n"
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
			 "<character>   ::= ALPHA | DIGIT | SYMBOL | COMMA\n"
			 "<spaces>      ::= <space> <spaces> | <space>\n"
			 "<space>       ::= ' '\n");

	lex_t lex = {0};
	lex_init(&lex, STR(__FILE__), &sbnf, line, 1, 1, ALLOC_STD);
	lex_tokenize(&lex);

	prs_t prs = {0};
	prs_init(&prs, &lex, &bnf.stx, 100, ALLOC_STD);
	prs_node_t prs_root = prs_parse(&prs, bnf.file, PRINT_DST_STD());

	stx_t new_stx = {0};
	stx_init(&new_stx, 10, 10, ALLOC_STD);
	EXPECT_EQ(stx_from_bnf(&bnf, &prs, prs_root, &new_stx), 0);

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
	bnf_get_stx(&bnf, ALLOC_STD);

	stx_t new_stx = {0};
	stx_init(&new_stx, 10, 10, ALLOC_STD);

	prs_t prs = {0};
	prs_init(&prs, NULL, &new_stx, 100, ALLOC_STD);

	prs_node_t file	  = prs_add_node(&prs, PRS_NODE_END, PRS_NODE_RULE(&prs, bnf.file));
	prs_node_t pbnf	  = prs_add_node(&prs, file, PRS_NODE_RULE(&prs, bnf.bnf));
	prs_node_t rules  = prs_add_node(&prs, pbnf, PRS_NODE_RULE(&prs, bnf.rules));
	prs_node_t rule	  = prs_add_node(&prs, rules, PRS_NODE_RULE(&prs, bnf.rule));
	prs_node_t expr	  = prs_add_node(&prs, rule, PRS_NODE_RULE(&prs, bnf.expr));
	prs_node_t terms0 = prs_add_node(&prs, expr, PRS_NODE_RULE(&prs, bnf.terms));
	prs_node_t term0  = prs_add_node(&prs, terms0, PRS_NODE_RULE(&prs, bnf.term));
	prs_add_node(&prs, term0, PRS_NODE_RULE(&prs, bnf.literal));
	prs_node_t terms1 = prs_add_node(&prs, terms0, PRS_NODE_RULE(&prs, bnf.terms));
	prs_add_node(&prs, terms1, PRS_NODE_RULE(&prs, bnf.term));

	log_set_quiet(0, 1);
	EXPECT_EQ(stx_from_bnf(&bnf, &prs, file, &new_stx), 0);
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
