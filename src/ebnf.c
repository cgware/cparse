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

const stx_t *ebnf_get_stx(ebnf_t *ebnf, alloc_t alloc, print_dst_t dst)
{
	if (ebnf == NULL) {
		return NULL;
	}

	uint line  = __LINE__ + 1;
	str_t sbnf = STR("<file>    ::= <ebnf> EOF\n"
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

	lex_tokenize(&lex, &sbnf, STR(__FILE__), line);

	bnf_t bnf = {0};
	bnf_init(&bnf, alloc);
	bnf_get_stx(&bnf);

	prs_t prs = {0};
	prs_init(&prs, 100, alloc);

	prs_node_t prs_root;
	prs_parse(&prs, &lex, &bnf.stx, bnf.file, &prs_root, dst);

	bnf_free(&bnf);

	strbuf_t names = {0};
	strbuf_init(&names, 16 * sizeof(char), ALLOC_STD);

	stx_from_bnf(&bnf, &prs, prs_root, &ebnf->stx, &names);

#define PSTRV(_str) _str, sizeof(_str) - 1

	strbuf_get_index(&names, PSTRV("file"), &ebnf->file);
	strbuf_get_index(&names, PSTRV("ebnf"), &ebnf->ebnf);
	strbuf_get_index(&names, PSTRV("rules"), &ebnf->rules);
	strbuf_get_index(&names, PSTRV("rule"), &ebnf->rule);
	strbuf_get_index(&names, PSTRV("rname"), &ebnf->rname);
	strbuf_get_index(&names, PSTRV("alt"), &ebnf->alt);
	strbuf_get_index(&names, PSTRV("concat"), &ebnf->concat);
	strbuf_get_index(&names, PSTRV("factor"), &ebnf->factor);
	strbuf_get_index(&names, PSTRV("term"), &ebnf->term);
	strbuf_get_index(&names, PSTRV("literal"), &ebnf->literal);
	strbuf_get_index(&names, PSTRV("tdouble"), &ebnf->tdouble);
	strbuf_get_index(&names, PSTRV("tsingle"), &ebnf->tsingle);
	strbuf_get_index(&names, PSTRV("token"), &ebnf->token);
	strbuf_get_index(&names, PSTRV("group"), &ebnf->group);
	strbuf_get_index(&names, PSTRV("opt"), &ebnf->opt);
	strbuf_get_index(&names, PSTRV("rep"), &ebnf->rep);
	strbuf_get_index(&names, PSTRV("opt-rep"), &ebnf->opt_rep);

	strbuf_free(&names);

	prs_free(&prs);
	lex_free(&lex);

	return &ebnf->stx;
}

static void alt_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			  estx_rule_t rule, int recursive);

static void term_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, estx_term_occ_t occ,
			   strbuf_t *names, estx_rule_t rule)
{
	const prs_node_t prs_rname = prs_get_rule(prs, node, ebnf->rname);
	if (prs_rname < prs->nodes.cnt) {
		token_t str = {0};
		prs_get_str(prs, prs_rname, &str);
		strv_t name = lex_get_token_val(prs->lex, str);

		estx_rule_t new_rule;
		if (strbuf_get_index(names, name.data, name.len, &new_rule)) {
			strbuf_add(names, name.data, name.len, &new_rule);
			estx_add_rule(estx);
		}

		if (rule != ESTX_RULE_END) {
			estx_rule_set_term(estx, rule, ESTX_TERM_RULE(estx, new_rule, occ));
		} else {
			estx_term_add_term(estx, parent, ESTX_TERM_RULE(estx, new_rule, occ));
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

		if (rule != ESTX_RULE_END) {
			estx_rule_set_term(estx, rule, ESTX_TERM_LITERAL(estx, literal, occ));
		} else {
			estx_term_add_term(estx, parent, ESTX_TERM_LITERAL(estx, literal, occ));
		}
		return;
	}

	const prs_node_t prs_token = prs_get_rule(prs, node, ebnf->token);
	if (prs_token < prs->nodes.cnt) {
		token_t str = {0};
		prs_get_str(prs, prs_token, &str);
		strv_t token = lex_get_token_val(prs->lex, str);
		if (rule != ESTX_RULE_END) {
			estx_rule_set_term(estx, rule, ESTX_TERM_TOKEN(estx, token_type_enum(token), occ));
		} else {
			estx_term_t tok = ESTX_TERM_TOKEN(estx, token_type_enum(token), occ);
			estx_term_add_term(estx, parent, tok);
		}
		return;
	}

	const prs_node_t prs_group = prs_get_rule(prs, node, ebnf->group);
	if (prs_group < prs->nodes.cnt) {
		const estx_term_t group	 = rule == ESTX_RULE_END ? estx_term_add_term(estx, parent, ESTX_TERM_GROUP(estx, occ))
								 : estx_rule_set_term(estx, rule, ESTX_TERM_GROUP(estx, occ));
		const prs_node_t prs_alt = prs_get_rule(prs, prs_group, ebnf->alt);
		alt_from_ebnf(ebnf, prs, prs_alt, estx, group, names, ESTX_RULE_END, 0);
	}
}

static void factor_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			     estx_rule_t rule)
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

	term_from_ebnf(ebnf, prs, prs_term, estx, parent, occ, names, rule);
}

static void concat_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			     estx_rule_t rule, int recursive)
{
	const prs_node_t prs_factor = prs_get_rule(prs, node, ebnf->factor);

	const prs_node_t prs_concat = prs_get_rule(prs, node, ebnf->concat);
	if (prs_concat < prs->nodes.cnt) {
		estx_term_t concat = parent;
		if (rule != ESTX_RULE_END) {
			concat = estx_rule_set_term(estx, rule, ESTX_TERM_CON(estx));
		} else if (!recursive) {
			concat = estx_term_add_term(estx, parent, ESTX_TERM_CON(estx));
		}
		factor_from_ebnf(ebnf, prs, prs_factor, estx, concat, names, STX_RULE_END);
		concat_from_ebnf(ebnf, prs, prs_concat, estx, concat, names, STX_RULE_END, 1);
	} else {
		factor_from_ebnf(ebnf, prs, prs_factor, estx, parent, names, rule);
	}
}

static void alt_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, estx_t *estx, estx_term_t parent, strbuf_t *names,
			  estx_rule_t rule, int recursive)
{
	const prs_node_t prs_concat = prs_get_rule(prs, node, ebnf->concat);

	const prs_node_t prs_alt = prs_get_rule(prs, node, ebnf->alt);
	if (prs_alt < prs->nodes.cnt) {
		estx_term_t alt = parent;
		if (rule != ESTX_RULE_END) {
			alt = estx_rule_set_term(estx, rule, ESTX_TERM_ALT(estx));
		} else if (!recursive) {
			alt = estx_term_add_term(estx, parent, ESTX_TERM_ALT(estx));
		}
		concat_from_ebnf(ebnf, prs, prs_concat, estx, alt, names, STX_RULE_END, 0);
		alt_from_ebnf(ebnf, prs, prs_alt, estx, alt, names, STX_RULE_END, 1);
	} else {
		concat_from_ebnf(ebnf, prs, prs_concat, estx, parent, names, rule, 0);
	}
}

static estx_rule_t rules_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t node, strbuf_t *names, estx_t *estx)
{
	const prs_node_t prs_rule  = prs_get_rule(prs, node, ebnf->rule);
	const prs_node_t prs_rname = prs_get_rule(prs, prs_rule, ebnf->rname);

	token_t str = {0};
	prs_get_str(prs, prs_rname, &str);
	strv_t name = lex_get_token_val(prs->lex, str);

	estx_rule_t rule;
	if (strbuf_get_index(names, name.data, name.len, &rule)) {
		strbuf_add(names, name.data, name.len, &rule);
		estx_add_rule(estx);
	}

	const prs_node_t prs_alt = prs_get_rule(prs, prs_rule, ebnf->alt);
	alt_from_ebnf(ebnf, prs, prs_alt, estx, STX_TERM_END, names, rule, 0);

	const prs_node_t rules = prs_get_rule(prs, node, ebnf->rules);
	if (rules >= prs->nodes.cnt) {
		return rule;
	}

	rules_from_ebnf(ebnf, prs, rules, names, estx);

	return rule;
}

estx_rule_t estx_from_ebnf(const ebnf_t *ebnf, const prs_t *prs, prs_node_t root, estx_t *estx, strbuf_t *names)
{
	prs_node_t febnf = prs_get_rule(prs, root, ebnf->ebnf);
	prs_node_t rules = prs_get_rule(prs, febnf, ebnf->rules);

	estx_rule_t estx_root = rules_from_ebnf(ebnf, prs, rules, names, estx);

	return estx_root;
}
