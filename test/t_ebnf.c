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
	EXPECT_EQ(stx_print(&ebnf.stx, DST_BUF(buf)), 851);
	EXPECT_STR(buf,
		   "<file> ::= <ebnf> EOF\n"
		   "<ebnf> ::= <rules>\n"
		   "<rules> ::= <rule> <rules> | <rule>\n"
		   "<rule> ::= <rname> <spaces> '=' <space> <alt> NL\n"
		   "<rname> ::= LOWER <rchars> | LOWER\n"
		   "<spaces> ::= <space> <spaces> | <space>\n"
		   "<space> ::= ' '\n"
		   "<alt> ::= <concat> <space> '|' <space> <alt> | <concat>\n"
		   "<rchars> ::= <rchar> <rchars> | <rchar>\n"
		   "<rchar> ::= LOWER | '_'\n"
		   "<concat> ::= <factor> <space> <concat> | <factor>\n"
		   "<factor> ::= <term> <opt> | <term> <rep> | <term> <opt-rep> | <term>\n"
		   "<term> ::= <literal> | <token> | <rname> | <group>\n"
		   "<opt> ::= '?'\n"
		   "<rep> ::= '+'\n"
		   "<opt-rep> ::= '*'\n"
		   "<literal> ::= \"'\" <tdouble> \"'\" | '\"' <tsingle> '\"'\n"
		   "<token> ::= UPPER <token> | UPPER\n"
		   "<group> ::= '(' <alt> ')'\n"
		   "<tdouble> ::= <cdouble> <tdouble> | <cdouble>\n"
		   "<tsingle> ::= <csingle> <tsingle> | <csingle>\n"
		   "<cdouble> ::= <char> | '\"'\n"
		   "<csingle> ::= <char> | \"'\"\n"
		   "<char> ::= ALPHA | DIGIT | SYMBOL | ' '\n");

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

	estx_t new_stx = {0};
	estx_init(&new_stx, 10, ALLOC_STD);
	estx_node_t root;
	EXPECT_EQ(estx_from_ebnf(NULL, &prs, prs_root, &new_stx, &root), 0);
	EXPECT_EQ(estx_from_ebnf(&ebnf, &prs, prs_root, &new_stx, &root), 0);
	EXPECT_EQ(root, 0);

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
	estx_init(&new_stx, 10, ALLOC_STD);

	prs.stx = &ebnf.stx;

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

	estx_node_t root;

	log_set_quiet(0, 1);
	EXPECT_EQ(estx_from_ebnf(&ebnf, &prs, file, &new_stx, &root), 0);
	EXPECT_EQ(root, 0);
	log_set_quiet(0, 0);

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
