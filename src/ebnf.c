#include "ebnf.h"

#include "bnf.h"
#include "log.h"
#include "token.h"

ebnf_t *ebnf_init(ebnf_t *ebnf, alloc_t alloc)
{
	if (ebnf == NULL) {
		return NULL;
	}

	if (stx_init(&ebnf->stx, 10, 10, alloc) == NULL) {
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

	strbuf_t names = {0};
	strbuf_init(&names, 16, 16, ALLOC_STD);

	stx_rule_t root;
	stx_from_bnf(&bnf, &prs, prs_root, &ebnf->stx, &names, &root);

	strbuf_find(&names, STRV("file"), &ebnf->file);
	strbuf_find(&names, STRV("ebnf"), &ebnf->ebnf);
	strbuf_find(&names, STRV("rules"), &ebnf->rules);
	strbuf_find(&names, STRV("rule"), &ebnf->rule);
	strbuf_find(&names, STRV("rname"), &ebnf->rname);
	strbuf_find(&names, STRV("alt"), &ebnf->alt);
	strbuf_find(&names, STRV("concat"), &ebnf->concat);
	strbuf_find(&names, STRV("factor"), &ebnf->factor);
	strbuf_find(&names, STRV("term"), &ebnf->term);
	strbuf_find(&names, STRV("literal"), &ebnf->literal);
	strbuf_find(&names, STRV("tdouble"), &ebnf->tdouble);
	strbuf_find(&names, STRV("tsingle"), &ebnf->tsingle);
	strbuf_find(&names, STRV("token"), &ebnf->token);
	strbuf_find(&names, STRV("group"), &ebnf->group);
	strbuf_find(&names, STRV("opt"), &ebnf->opt);
	strbuf_find(&names, STRV("rep"), &ebnf->rep);
	strbuf_find(&names, STRV("opt-rep"), &ebnf->opt_rep);

	strbuf_free(&names);

	prs_free(&prs);
	lex_free(&lex);

	return &ebnf->stx;
}

static void alt_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			  estx_rule_t rule, int is_rule, int recursive);

static void term_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, estx_term_occ_t occ,
			   strbuf_t *names, estx_rule_t rule, int is_rule)
{
	const prs_node_t prs_rname = prs_get_rule(prs, node, ebnf->rname);
	if (prs_rname < prs->nodes.cnt) {
		token_t str = {0};
		prs_get_str(prs, prs_rname, &str);
		strv_t name = lex_get_token_val(prs->lex, str);

		estx_rule_t new_rule;
		if (strbuf_find(names, name, &new_rule)) {
			strbuf_add(names, name, &new_rule);
			estx_add_rule(estx);
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

	const prs_node_t prs_literal = prs_get_rule(prs, node, ebnf->literal);
	if (prs_literal < prs->nodes.cnt) {
		const prs_node_t prs_text_double = prs_get_rule(prs, prs_literal, ebnf->tdouble);
		const prs_node_t prs_text_single = prs_get_rule(prs, prs_literal, ebnf->tsingle);

		token_t str = {0};

		if (prs_text_double < prs->nodes.cnt) {
			prs_get_str(prs, prs_text_double, &str);
		} else if (prs_text_single < prs->nodes.cnt) {
			prs_get_str(prs, prs_text_single, &str);
		} else {
			return;
		}

		strv_t literal = lex_get_token_val(prs->lex, str);

		estx_term_t term;
		estx_term_lit(estx, literal, occ, &term);

		if (is_rule) {
			estx_rule_set_term(estx, rule, term);
		} else {
			estx_term_add_term(estx, parent, term);
		}
		return;
	}

	const prs_node_t prs_token = prs_get_rule(prs, node, ebnf->token);
	if (prs_token < prs->nodes.cnt) {
		token_t str = {0};
		prs_get_str(prs, prs_token, &str);
		strv_t token = lex_get_token_val(prs->lex, str);

		estx_term_t term;
		estx_term_tok(estx, token_type_enum(token), occ, &term);

		if (is_rule) {
			estx_rule_set_term(estx, rule, term);
		} else {
			estx_term_add_term(estx, parent, term);
		}
		return;
	}

	const prs_node_t prs_group = prs_get_rule(prs, node, ebnf->group);
	if (prs_group < prs->nodes.cnt) {
		estx_term_t group;

		estx_term_group(estx, occ, &group);

		if (is_rule) {
			estx_rule_set_term(estx, rule, group);
		} else {
			estx_term_add_term(estx, parent, group);
		}

		const prs_node_t prs_alt = prs_get_rule(prs, prs_group, ebnf->alt);
		alt_from_ebnf(ebnf, prs, prs_alt, estx, group, names, 0, 0, 0);
	}
}

static void factor_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			     estx_rule_t rule, int is_rule)
{
	const prs_node_t prs_term = prs_get_rule(prs, node, ebnf->term);
	estx_term_occ_t occ	  = ESTX_TERM_OCC_ONE;

	const prs_node_t prs_opt = prs_get_rule(prs, node, ebnf->opt);
	if (prs_opt < prs->nodes.cnt) {
		occ = ESTX_TERM_OCC_OPT;
	}

	const prs_node_t prs_rep = prs_get_rule(prs, node, ebnf->rep);
	if (prs_rep < prs->nodes.cnt) {
		occ = ESTX_TERM_OCC_REP;
	}

	const prs_node_t prs_opt_rep = prs_get_rule(prs, node, ebnf->opt_rep);
	if (prs_opt_rep < prs->nodes.cnt) {
		occ = ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP;
	}

	term_from_ebnf(ebnf, prs, prs_term, estx, parent, occ, names, rule, is_rule);
}

static void concat_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			     estx_rule_t rule, int is_rule, int recursive)
{
	const prs_node_t prs_factor = prs_get_rule(prs, node, ebnf->factor);

	const prs_node_t prs_concat = prs_get_rule(prs, node, ebnf->concat);
	if (prs_concat < prs->nodes.cnt) {
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
	} else {
		factor_from_ebnf(ebnf, prs, prs_factor, estx, parent, names, rule, is_rule);
	}
}

static void alt_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			  estx_rule_t rule, int is_rule, int recursive)
{
	const prs_node_t prs_concat = prs_get_rule(prs, node, ebnf->concat);

	const prs_node_t prs_alt = prs_get_rule(prs, node, ebnf->alt);
	if (prs_alt < prs->nodes.cnt) {
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
	} else {
		concat_from_ebnf(ebnf, prs, prs_concat, estx, parent, names, rule, is_rule, 0);
	}
}

static int rules_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, strbuf_t *names, estx_t *estx, estx_rule_t *root)
{
	const prs_node_t prs_rule  = prs_get_rule(prs, node, ebnf->rule);
	const prs_node_t prs_rname = prs_get_rule(prs, prs_rule, ebnf->rname);

	token_t str = {0};
	prs_get_str(prs, prs_rname, &str);
	strv_t name = lex_get_token_val(prs->lex, str);

	estx_rule_t rule;
	if (strbuf_find(names, name, &rule)) {
		strbuf_add(names, name, &rule);
		estx_add_rule(estx);
	}

	const prs_node_t prs_alt = prs_get_rule(prs, prs_rule, ebnf->alt);
	alt_from_ebnf(ebnf, prs, prs_alt, estx, (uint)-1, names, rule, 1, 0);

	const prs_node_t rules = prs_get_rule(prs, node, ebnf->rules);

	if (root) {
		*root = rule;
	}

	if (rules >= prs->nodes.cnt) {
		return 0;
	}

	return rules_from_ebnf(ebnf, prs, rules, names, estx, NULL);
}

int estx_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t root, estx_t *estx, strbuf_t *names, estx_rule_t *rule)
{
	if (ebnf == NULL) {
		return 0;
	}

	prs_node_t febnf = prs_get_rule(prs, root, ebnf->ebnf);
	prs_node_t rules = prs_get_rule(prs, febnf, ebnf->rules);

	return rules_from_ebnf(ebnf, prs, rules, names, estx, rule);
}
