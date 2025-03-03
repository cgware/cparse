#include "ebnf.h"

#include "log.h"
#include "mem.h"
#include "test.h"

TEST(ebnf_init_free)
{
	START;

	ebnf_t ebnf = {0};

	EXPECT_EQ(ebnf_init(NULL, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(ebnf_init(&ebnf, ALLOC_STD), NULL);
	mem_oom(0);
	EXPECT_EQ(ebnf_init(&ebnf, ALLOC_STD), &ebnf);

	ebnf_free(&ebnf);
	ebnf_free(NULL);

	END;
}

TEST(ebnf_get_stx)
{
	START;

	ebnf_t ebnf = {0};
	ebnf_init(&ebnf, ALLOC_STD);

	EXPECT_EQ(ebnf_get_stx(NULL, ALLOC_STD, PRINT_DST_NONE()), NULL);
	mem_oom(1);
	EXPECT_EQ(ebnf_get_stx(&ebnf, ALLOC_STD, PRINT_DST_NONE()), NULL);
	mem_oom(0);
	EXPECT_NE(ebnf_get_stx(&ebnf, ALLOC_STD, PRINT_DST_NONE()), NULL);

	char buf[1024] = {0};
	EXPECT_EQ(stx_print(&ebnf.stx, PRINT_DST_BUF(buf, sizeof(buf), 0)), 588);
	EXPECT_STR(buf,
		   "<0> ::= <1> EOF\n"
		   "<1> ::= <2>\n"
		   "<2> ::= <3> <2> | <3>\n"
		   "<3> ::= <4> <5> '=' <6> <7> NL\n"
		   "<4> ::= LOWER <8> | LOWER\n"
		   "<5> ::= <6> <5> | <6>\n"
		   "<6> ::= ' '\n"
		   "<7> ::= <10> <6> '|' <6> <7> | <10>\n"
		   "<8> ::= <9> <8> | <9>\n"
		   "<9> ::= LOWER | '_'\n"
		   "<10> ::= <11> <6> <10> | <11>\n"
		   "<11> ::= <12> <13> | <12> <14> | <12> <15> | <12>\n"
		   "<12> ::= <16> | <17> | <4> | <18>\n"
		   "<13> ::= '?'\n"
		   "<14> ::= '+'\n"
		   "<15> ::= '*'\n"
		   "<16> ::= \"'\" <19> \"'\" | '\"' <20> '\"'\n"
		   "<17> ::= UPPER <17> | UPPER\n"
		   "<18> ::= '(' <7> ')'\n"
		   "<19> ::= <21> <19> | <21>\n"
		   "<20> ::= <22> <20> | <22>\n"
		   "<21> ::= <23> | '\"'\n"
		   "<22> ::= <23> | \"'\"\n"
		   "<23> ::= ALPHA | DIGIT | SYMBOL | ' '\n");

	ebnf_free(&ebnf);

	END;
}

TEST(stx_from_ebnf)
{
	START;

	ebnf_t ebnf = {0};
	ebnf_init(&ebnf, ALLOC_STD);
	ebnf_get_stx(&ebnf, ALLOC_STD, PRINT_DST_NONE());

	uint line  = __LINE__ + 1;
	str_t sbnf = STR("file    = ebnf EOF\n"
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

	lex_t lex = {0};
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &sbnf, STR(__FILE__), line);

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);

	prs_node_t prs_root;
	prs_parse(&prs, &lex, &ebnf.stx, ebnf.file, &prs_root, PRINT_DST_NONE());

	strbuf_t names = {0};
	strbuf_init(&names, 16, 16, ALLOC_STD);

	estx_t new_stx = {0};
	estx_init(&new_stx, 10, 10, ALLOC_STD);
	EXPECT_EQ(estx_from_ebnf(&ebnf, &prs, prs_root, &new_stx, &names), 0);

	strbuf_free(&names);
	estx_free(&new_stx);
	lex_free(&lex);
	prs_free(&prs);
	ebnf_free(&ebnf);

	END;
}

TEST(stx_from_ebnf_custom)
{
	START;

	ebnf_t ebnf = {0};
	ebnf_init(&ebnf, ALLOC_STD);
	ebnf_get_stx(&ebnf, ALLOC_STD, PRINT_DST_NONE());

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);

	estx_t new_stx = {0};
	estx_init(&new_stx, 10, 10, ALLOC_STD);

	prs_node_t file	  = prs_add_node(&prs, PRS_NODE_END, PRS_NODE_RULE(&prs, ebnf.file));
	prs_node_t pebnf  = prs_add_node(&prs, file, PRS_NODE_RULE(&prs, ebnf.ebnf));
	prs_node_t rs_lit = prs_add_node(&prs, pebnf, PRS_NODE_RULE(&prs, ebnf.rules));
	prs_node_t r_lit  = prs_add_node(&prs, rs_lit, PRS_NODE_RULE(&prs, ebnf.rule));
	prs_node_t a_lit  = prs_add_node(&prs, r_lit, PRS_NODE_RULE(&prs, ebnf.alt));
	prs_node_t c_lit  = prs_add_node(&prs, a_lit, PRS_NODE_RULE(&prs, ebnf.concat));
	prs_node_t f_lit  = prs_add_node(&prs, c_lit, PRS_NODE_RULE(&prs, ebnf.factor));
	prs_node_t t_lit  = prs_add_node(&prs, f_lit, PRS_NODE_RULE(&prs, ebnf.term));
	prs_add_node(&prs, t_lit, PRS_NODE_RULE(&prs, ebnf.literal));

	prs_node_t rs_grp = prs_add_node(&prs, rs_lit, PRS_NODE_RULE(&prs, ebnf.rules));
	prs_node_t r_grp  = prs_add_node(&prs, rs_grp, PRS_NODE_RULE(&prs, ebnf.rule));
	prs_node_t a_grp  = prs_add_node(&prs, r_grp, PRS_NODE_RULE(&prs, ebnf.alt));
	prs_node_t c_grp  = prs_add_node(&prs, a_grp, PRS_NODE_RULE(&prs, ebnf.concat));
	prs_node_t f_grp  = prs_add_node(&prs, c_grp, PRS_NODE_RULE(&prs, ebnf.factor));
	prs_node_t t_grp  = prs_add_node(&prs, f_grp, PRS_NODE_RULE(&prs, ebnf.term));
	prs_add_node(&prs, t_grp, PRS_NODE_RULE(&prs, ebnf.group));

	prs_add_node(&prs, f_grp, PRS_NODE_RULE(&prs, ebnf.opt_rep));

	strbuf_t names = {0};
	strbuf_init(&names, 16, 16, ALLOC_STD);

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_from_ebnf(&ebnf, &prs, file, &new_stx, &names), 0);
	log_set_quiet(0, 0);

	strbuf_free(&names);
	estx_free(&new_stx);
	prs_free(&prs);
	ebnf_free(&ebnf);

	END;
}

STEST(ebnf)
{
	SSTART;

	RUN(ebnf_init_free);
	RUN(ebnf_get_stx);
	RUN(stx_from_ebnf);
	RUN(stx_from_ebnf_custom);

	SEND;
}
