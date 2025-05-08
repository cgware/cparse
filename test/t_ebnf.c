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

	EXPECT_EQ(ebnf_get_stx(NULL, ALLOC_STD, DST_NONE()), NULL);
	mem_oom(1);
	EXPECT_EQ(ebnf_get_stx(&ebnf, ALLOC_STD, DST_NONE()), NULL);
	mem_oom(0);
	EXPECT_NE(ebnf_get_stx(&ebnf, ALLOC_STD, DST_NONE()), NULL);

	char buf[1024] = {0};
	EXPECT_EQ(stx_print(&ebnf.stx, DST_BUF(buf)), 588);
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

	lex_t lex = {0};
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, sbnf, STRV(__FILE__), line);

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);

	prs_node_t prs_root;
	prs_parse(&prs, &lex, &ebnf.stx, ebnf.file, &prs_root, DST_NONE());

	strbuf_t names = {0};
	strbuf_init(&names, 16, 16, ALLOC_STD);

	estx_t new_stx = {0};
	estx_init(&new_stx, 10, 10, ALLOC_STD);
	estx_rule_t root;
	EXPECT_EQ(estx_from_ebnf(NULL, &prs, prs_root, &new_stx, &names, &root), 0);
	EXPECT_EQ(estx_from_ebnf(&ebnf, &prs, prs_root, &new_stx, &names, &root), 0);
	EXPECT_EQ(root, 0);

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
	ebnf_get_stx(&ebnf, ALLOC_STD, DST_NONE());

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);

	estx_t new_stx = {0};
	estx_init(&new_stx, 10, 10, ALLOC_STD);

	prs_node_t file, pebnf, rs_lit, r_lit, a_lit, c_lit, f_lit, t_lit, node;
	prs_node_rule(&prs, ebnf.file, &file);
	prs_node_rule(&prs, ebnf.ebnf, &pebnf);
	prs_add_node(&prs, file, pebnf);
	prs_node_rule(&prs, ebnf.rules, &rs_lit);
	prs_add_node(&prs, pebnf, rs_lit);
	prs_node_rule(&prs, ebnf.rule, &r_lit);
	prs_add_node(&prs, rs_lit, r_lit);
	prs_node_rule(&prs, ebnf.alt, &a_lit);
	prs_add_node(&prs, r_lit, a_lit);
	prs_node_rule(&prs, ebnf.concat, &c_lit);
	prs_add_node(&prs, a_lit, c_lit);
	prs_node_rule(&prs, ebnf.factor, &f_lit);
	prs_add_node(&prs, c_lit, f_lit);
	prs_node_rule(&prs, ebnf.term, &t_lit);
	prs_add_node(&prs, f_lit, t_lit);
	prs_node_rule(&prs, ebnf.literal, &node);
	prs_add_node(&prs, t_lit, node);

	prs_node_t rs_grp, r_grp, a_grp, c_grp, f_grp, t_grp;
	prs_node_rule(&prs, ebnf.rules, &rs_grp);
	prs_add_node(&prs, rs_lit, rs_grp);
	prs_node_rule(&prs, ebnf.rule, &r_grp);
	prs_add_node(&prs, rs_grp, r_grp);
	prs_node_rule(&prs, ebnf.alt, &a_grp);
	prs_add_node(&prs, r_grp, a_grp);
	prs_node_rule(&prs, ebnf.concat, &c_grp);
	prs_add_node(&prs, a_grp, c_grp);
	prs_node_rule(&prs, ebnf.factor, &f_grp);
	prs_add_node(&prs, c_grp, f_grp);
	prs_node_rule(&prs, ebnf.term, &t_grp);
	prs_add_node(&prs, f_grp, t_grp);
	prs_node_rule(&prs, ebnf.group, &node);
	prs_add_node(&prs, t_grp, node);

	prs_node_rule(&prs, ebnf.opt_rep, &node);
	prs_add_node(&prs, f_grp, node);

	strbuf_t names = {0};
	strbuf_init(&names, 16, 16, ALLOC_STD);

	estx_rule_t root;

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_from_ebnf(&ebnf, &prs, file, &new_stx, &names, &root), 0);
	EXPECT_EQ(root, 0);
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
