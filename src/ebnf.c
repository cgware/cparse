#include "ebnf.h"

#include "bnf.h"
#include "log.h"
#include "tok.h"

ebnf_t *ebnf_init(ebnf_t *ebnf, alloc_t alloc)
{
	if (ebnf == NULL) {
		return NULL;
	}

	if (stx_init(&ebnf->stx, 64, alloc) == NULL) {
		log_error("cparse", "ebnf", NULL, "failed to intialize syntax");
		return NULL;
	}

	return ebnf;
}

void ebnf_free(ebnf_t *ebnf)
{
	if (ebnf == NULL) {
		return;
	}

	stx_free(&ebnf->stx);
}

const stx_t *ebnf_get_stx(ebnf_t *ebnf, alloc_t alloc, dst_t dst)
{
	if (ebnf == NULL) {
		return NULL;
	}

	uint line   = __LINE__ + 1;
	strv_t sbnf = STRV("<file>    ::= <ebnf> EOF\n"
			   "<ebnf>    ::= <rules>\n"
			   "<rules>   ::= <rule> <rules> | <rule>\n"
			   "<rule>    ::= <rname> <spaces> '=' <space> <alt> NL\n"
			   "<rname>   ::= LOWER <rchars> | LOWER\n"
			   "<rchars>  ::= <rchar> <rchars> | <rchar>\n"
			   "<rchar>   ::= LOWER | '_'\n"
			   "<alt>     ::= <concat> <space> '|' <space> <alt> | <concat>\n"
			   "<concat>  ::= <factor> <space> <concat> | <factor>\n"
			   "<factor>  ::= <term> <opt> | <term> <rep> | <term> <opt-rep> | <term>\n"
			   "<opt>     ::= '?'\n"
			   "<rep>     ::= '+'\n"
			   "<opt-rep> ::= '*'\n"
			   "<term>    ::= <literal> | <token> | <rname> | <group>\n"
			   "<literal> ::= \"'\" <tdouble> \"'\" | '\"' <tsingle> '\"'\n"
			   "<token>   ::= UPPER <token> | UPPER\n"
			   "<group>   ::= '(' <alt> ')'\n"
			   "<tdouble> ::= <cdouble> <tdouble> | <cdouble>\n"
			   "<tsingle> ::= <csingle> <tsingle> | <csingle>\n"
			   "<cdouble> ::= <char> | '\"'\n"
			   "<csingle> ::= <char> | \"'\"\n"
			   "<char>    ::= ALPHA | DIGIT | SYMBOL | ' '\n"
			   "<spaces>  ::= <space> <spaces> | <space>\n"
			   "<space>   ::= ' '\n");

	lex_t lex = {0};
	if (lex_init(&lex, 0, 100, alloc) == NULL) {
		log_error("cparse", "bnf", NULL, "failed to intialize lexer");
		return NULL;
	}

	lex_tokenize(&lex, sbnf, STRV(__FILE__), line);

	bnf_t bnf = {0};
	bnf_init(&bnf, alloc);
	bnf_get_stx(&bnf);

	prs_t prs = {0};
	prs_init(&prs, 100, alloc);

	prs_node_t prs_root;
	prs_parse(&prs, &lex, &bnf.stx, bnf.file, &prs_root, dst);

	bnf_free(&bnf);

	stx_node_t root;
	stx_from_bnf(&bnf, &prs, prs_root, &ebnf->stx, &root);

	stx_find_rule(&ebnf->stx, STRV("file"), &ebnf->file);
	stx_find_rule(&ebnf->stx, STRV("ebnf"), &ebnf->ebnf);
	stx_find_rule(&ebnf->stx, STRV("rules"), &ebnf->rules);
	stx_find_rule(&ebnf->stx, STRV("rule"), &ebnf->rule);
	stx_find_rule(&ebnf->stx, STRV("rname"), &ebnf->rname);
	stx_find_rule(&ebnf->stx, STRV("alt"), &ebnf->alt);
	stx_find_rule(&ebnf->stx, STRV("concat"), &ebnf->concat);
	stx_find_rule(&ebnf->stx, STRV("factor"), &ebnf->factor);
	stx_find_rule(&ebnf->stx, STRV("term"), &ebnf->term);
	stx_find_rule(&ebnf->stx, STRV("literal"), &ebnf->literal);
	stx_find_rule(&ebnf->stx, STRV("tdouble"), &ebnf->tdouble);
	stx_find_rule(&ebnf->stx, STRV("tsingle"), &ebnf->tsingle);
	stx_find_rule(&ebnf->stx, STRV("token"), &ebnf->tok);
	stx_find_rule(&ebnf->stx, STRV("group"), &ebnf->group);
	stx_find_rule(&ebnf->stx, STRV("opt"), &ebnf->opt);
	stx_find_rule(&ebnf->stx, STRV("rep"), &ebnf->rep);
	stx_find_rule(&ebnf->stx, STRV("opt-rep"), &ebnf->opt_rep);

	prs_free(&prs);
	lex_free(&lex);

	return &ebnf->stx;
}

static void alt_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			  estx_rule_t rule, int is_rule, int recursive);

static void term_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, estx_term_occ_t occ,
			   strbuf_t *names, estx_rule_t rule, int is_rule)
{
	prs_node_t prs_rname;
	if (prs_get_rule(prs, node, ebnf->rname, &prs_rname) == 0) {
		tok_t str = {0};
		prs_get_str(prs, prs_rname, &str);
		strv_t name = lex_get_tok_val(prs->lex, str);

		estx_rule_t new_rule;
		if (strbuf_find(names, name, &new_rule)) {
			strbuf_add(names, name, &new_rule);
			estx_add_rule(estx, NULL);
		}

		estx_term_t term;
		estx_term_rule(estx, new_rule, occ, &term);

		if (is_rule) {
			estx_rule_set_term(estx, rule, term);
		} else {
			estx_term_add_term(estx, parent, term);
		}
		return;
	}

	prs_node_t prs_literal;
	if (prs_get_rule(prs, node, ebnf->literal, &prs_literal) == 0) {
		prs_node_t prs_text_double, prs_text_single;
		tok_t str = {0};

		if (prs_get_rule(prs, prs_literal, ebnf->tdouble, &prs_text_double) == 0) {
			prs_get_str(prs, prs_text_double, &str);
		} else if (prs_get_rule(prs, prs_literal, ebnf->tsingle, &prs_text_single) == 0) {
			prs_get_str(prs, prs_text_single, &str);
		} else {
			return;
		}

		strv_t literal = lex_get_tok_val(prs->lex, str);

		estx_term_t term;
		estx_term_lit(estx, literal, occ, &term);

		if (is_rule) {
			estx_rule_set_term(estx, rule, term);
		} else {
			estx_term_add_term(estx, parent, term);
		}
		return;
	}

	prs_node_t prs_tok;
	if (prs_get_rule(prs, node, ebnf->tok, &prs_tok) == 0) {
		tok_t str = {0};
		prs_get_str(prs, prs_tok, &str);
		strv_t tok = lex_get_tok_val(prs->lex, str);

		estx_term_t term;
		estx_term_tok(estx, tok_type_enum(tok), occ, &term);

		if (is_rule) {
			estx_rule_set_term(estx, rule, term);
		} else {
			estx_term_add_term(estx, parent, term);
		}
		return;
	}

	prs_node_t prs_group;
	if (prs_get_rule(prs, node, ebnf->group, &prs_group) == 0) {
		estx_term_t group;

		estx_term_group(estx, occ, &group);

		if (is_rule) {
			estx_rule_set_term(estx, rule, group);
		} else {
			estx_term_add_term(estx, parent, group);
		}

		prs_node_t prs_alt;
		prs_get_rule(prs, prs_group, ebnf->alt, &prs_alt);
		alt_from_ebnf(ebnf, prs, prs_alt, estx, group, names, 0, 0, 0);
	}
}

static void factor_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			     estx_rule_t rule, int is_rule)
{
	prs_node_t prs_term;
	prs_get_rule(prs, node, ebnf->term, &prs_term);
	estx_term_occ_t occ = ESTX_TERM_OCC_ONE;

	if (prs_get_rule(prs, node, ebnf->opt, NULL) == 0) {
		occ = ESTX_TERM_OCC_OPT;
	}

	if (prs_get_rule(prs, node, ebnf->rep, NULL) == 0) {
		occ = ESTX_TERM_OCC_REP;
	}

	if (prs_get_rule(prs, node, ebnf->opt_rep, NULL) == 0) {
		occ = ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP;
	}

	term_from_ebnf(ebnf, prs, prs_term, estx, parent, occ, names, rule, is_rule);
}

static void concat_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			     estx_rule_t rule, int is_rule, int recursive)
{
	prs_node_t prs_factor;
	prs_get_rule(prs, node, ebnf->factor, &prs_factor);

	prs_node_t prs_concat;
	if (prs_get_rule(prs, node, ebnf->concat, &prs_concat)) {
		factor_from_ebnf(ebnf, prs, prs_factor, estx, parent, names, rule, is_rule);
	} else {
		estx_term_t concat = parent;

		if (is_rule) {
			estx_term_con(estx, &concat);
			estx_rule_set_term(estx, rule, concat);
		} else if (!recursive) {
			estx_term_con(estx, &concat);
			estx_term_add_term(estx, parent, concat);
		}
		factor_from_ebnf(ebnf, prs, prs_factor, estx, concat, names, 0, 0);
		concat_from_ebnf(ebnf, prs, prs_concat, estx, concat, names, 0, 0, 1);
	}
}

static void alt_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			  estx_rule_t rule, int is_rule, int recursive)
{
	prs_node_t prs_concat;
	prs_get_rule(prs, node, ebnf->concat, &prs_concat);

	prs_node_t prs_alt;
	if (prs_get_rule(prs, node, ebnf->alt, &prs_alt)) {
		concat_from_ebnf(ebnf, prs, prs_concat, estx, parent, names, rule, is_rule, 0);
	} else {
		estx_term_t alt = parent;

		if (is_rule) {
			estx_term_alt(estx, &alt);
			estx_rule_set_term(estx, rule, alt);
		} else if (!recursive) {
			estx_term_alt(estx, &alt);
			estx_term_add_term(estx, parent, alt);
		}

		concat_from_ebnf(ebnf, prs, prs_concat, estx, alt, names, 0, 0, 0);
		alt_from_ebnf(ebnf, prs, prs_alt, estx, alt, names, 0, 0, 1);
	}
}

static int rules_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, strbuf_t *names, estx_t *estx, estx_rule_t *root)
{
	prs_node_t prs_rule, prs_rname;
	prs_get_rule(prs, node, ebnf->rule, &prs_rule);
	prs_get_rule(prs, prs_rule, ebnf->rname, &prs_rname);

	tok_t str = {0};
	prs_get_str(prs, prs_rname, &str);
	strv_t name = lex_get_tok_val(prs->lex, str);

	estx_rule_t rule;
	if (strbuf_find(names, name, &rule)) {
		strbuf_add(names, name, &rule);
		estx_add_rule(estx, NULL);
	}

	prs_node_t prs_alt;
	prs_get_rule(prs, prs_rule, ebnf->alt, &prs_alt);
	alt_from_ebnf(ebnf, prs, prs_alt, estx, (uint)-1, names, rule, 1, 0);

	if (root) {
		*root = rule;
	}

	prs_node_t rules;
	if (prs_get_rule(prs, node, ebnf->rules, &rules)) {
		return 0;
	}

	return rules_from_ebnf(ebnf, prs, rules, names, estx, NULL);
}

int estx_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t root, estx_t *estx, strbuf_t *names, estx_rule_t *rule)
{
	if (ebnf == NULL) {
		return 0;
	}

	prs_node_t febnf, rules;
	prs_get_rule(prs, root, ebnf->ebnf, &febnf);
	prs_get_rule(prs, febnf, ebnf->rules, &rules);

	return rules_from_ebnf(ebnf, prs, rules, names, estx, rule);
}
