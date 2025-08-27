#include "bnf.h"

#include "log.h"
#include "strbuf.h"
#include "tok.h"

bnf_t *bnf_init(bnf_t *bnf, alloc_t alloc)
{
	if (bnf == NULL) {
		return NULL;
	}

	if (stx_init(&bnf->stx, 128, alloc) == NULL) {
		log_error("cparse", "bnf", NULL, "failed to intialize syntax");
		return NULL;
	}

	return bnf;
}

void bnf_free(bnf_t *bnf)
{
	if (bnf == NULL) {
		return;
	}

	stx_free(&bnf->stx);
}

const stx_t *bnf_get_stx(bnf_t *bnf)
{
	if (bnf == NULL) {
		return NULL;
	}

	stx_t *stx = &bnf->stx;

	stx_node_t rchars, rchar, cdouble, csingle, character, spaces, space;
	stx_rule(stx, STRV("file"), &bnf->file);
	stx_rule(stx, STRV("bnf"), &bnf->bnf);
	stx_rule(stx, STRV("rules"), &bnf->rules);
	stx_rule(stx, STRV("rule"), &bnf->rule);
	stx_rule(stx, STRV("rname"), &bnf->rname);
	stx_rule(stx, STRV("rchars"), &rchars);
	stx_rule(stx, STRV("rchar"), &rchar);
	stx_rule(stx, STRV("expr"), &bnf->expr);
	stx_rule(stx, STRV("terms"), &bnf->terms);
	stx_rule(stx, STRV("term"), &bnf->term);
	stx_rule(stx, STRV("literal"), &bnf->literal);
	stx_rule(stx, STRV("token"), &bnf->tok);
	stx_rule(stx, STRV("tdouble"), &bnf->tdouble);
	stx_rule(stx, STRV("tsingle"), &bnf->tsingle);
	stx_rule(stx, STRV("cdouble"), &cdouble);
	stx_rule(stx, STRV("csingle"), &csingle);
	stx_rule(stx, STRV("char"), &character);
	stx_rule(stx, STRV("spaces"), &spaces);
	stx_rule(stx, STRV("space"), &space);

	stx_node_t term;

	stx_term_rule(stx, bnf->bnf, &term);
	stx_add_term(stx, bnf->file, term);
	stx_term_tok(stx, TOK_EOF, &term);
	stx_add_term(stx, bnf->file, term);

	stx_term_rule(stx, bnf->rules, &term);
	stx_add_term(stx, bnf->bnf, term);

	stx_term_rule(stx, bnf->rule, &term);
	stx_rule_add_arr(stx, bnf->rules, term);

	stx_term_lit(stx, STRV("<"), &term);
	stx_add_term(stx, bnf->rule, term);
	stx_term_rule(stx, bnf->rname, &term);
	stx_add_term(stx, bnf->rule, term);
	stx_term_lit(stx, STRV(">"), &term);
	stx_add_term(stx, bnf->rule, term);
	stx_term_rule(stx, spaces, &term);
	stx_add_term(stx, bnf->rule, term);
	stx_term_lit(stx, STRV("::="), &term);
	stx_add_term(stx, bnf->rule, term);
	stx_term_rule(stx, space, &term);
	stx_add_term(stx, bnf->rule, term);
	stx_term_rule(stx, bnf->expr, &term);
	stx_add_term(stx, bnf->rule, term);
	stx_term_tok(stx, TOK_NL, &term);
	stx_add_term(stx, bnf->rule, term);

	stx_node_t l, r;
	stx_term_tok(stx, TOK_LOWER, &l);
	stx_term_rule(stx, rchars, &term);
	stx_add_term(stx, l, term);
	stx_term_tok(stx, TOK_LOWER, &r);
	stx_node_t orv;
	stx_term_or(stx, l, r, &orv);
	stx_add_term(stx, bnf->rname, orv);

	stx_term_rule(stx, rchar, &term);
	stx_rule_add_arr(stx, rchars, term);

	stx_term_tok(stx, TOK_LOWER, &l);
	stx_term_lit(stx, STRV("-"), &r);
	stx_term_or(stx, l, r, &orv);
	stx_add_term(stx, rchar, orv);

	stx_term_rule(stx, bnf->terms, &l);
	stx_term_rule(stx, space, &term);
	stx_add_term(stx, l, term);
	stx_term_lit(stx, STRV("|"), &term);
	stx_add_term(stx, l, term);
	stx_term_rule(stx, space, &term);
	stx_add_term(stx, l, term);
	stx_term_rule(stx, bnf->expr, &term);
	stx_add_term(stx, l, term);
	stx_term_rule(stx, bnf->terms, &r);
	stx_term_or(stx, l, r, &orv);
	stx_add_term(stx, bnf->expr, orv);

	stx_term_rule(stx, bnf->term, &l);
	stx_term_rule(stx, space, &r);
	stx_rule_add_arr_sep(stx, bnf->terms, l, r);

	stx_term_lit(stx, STRV("<"), &l);
	stx_term_rule(stx, bnf->rname, &term);
	stx_add_term(stx, l, term);
	stx_term_lit(stx, STRV(">"), &term);
	stx_add_term(stx, l, term);
	stx_term_rule(stx, bnf->tok, &r);
	stx_term_rule(stx, bnf->literal, &term);
	stx_rule_add_or(stx, bnf->term, 3, term, r, l);

	stx_term_lit(stx, STRV("'"), &l);
	stx_term_rule(stx, bnf->tdouble, &term);
	stx_add_term(stx, l, term);
	stx_term_lit(stx, STRV("'"), &term);
	stx_add_term(stx, l, term);
	stx_term_lit(stx, STRV("\""), &r);
	stx_term_rule(stx, bnf->tsingle, &term);
	stx_add_term(stx, r, term);
	stx_term_lit(stx, STRV("\""), &term);
	stx_add_term(stx, r, term);
	stx_term_or(stx, l, r, &orv);
	stx_add_term(stx, bnf->literal, orv);

	stx_term_tok(stx, TOK_UPPER, &term);
	stx_rule_add_arr(stx, bnf->tok, term);

	stx_term_rule(stx, cdouble, &term);
	stx_rule_add_arr(stx, bnf->tdouble, term);
	stx_term_rule(stx, csingle, &term);
	stx_rule_add_arr(stx, bnf->tsingle, term);

	stx_term_rule(stx, character, &l);
	stx_term_lit(stx, STRV("\""), &r);
	stx_term_or(stx, l, r, &orv);
	stx_add_term(stx, cdouble, orv);
	stx_term_rule(stx, character, &l);
	stx_term_lit(stx, STRV("'"), &r);
	stx_term_or(stx, l, r, &orv);
	stx_add_term(stx, csingle, orv);

	stx_node_t t0, t1, t2, t3;
	stx_term_tok(stx, TOK_ALPHA, &t0);
	stx_term_tok(stx, TOK_DIGIT, &t1);
	stx_term_tok(stx, TOK_SYMBOL, &t2);
	stx_term_rule(stx, space, &t3);
	stx_rule_add_or(stx, character, 4, t0, t1, t2, t3);

	stx_term_rule(stx, space, &term);
	stx_rule_add_arr(stx, spaces, term);

	stx_term_lit(stx, STRV(" "), &term);
	stx_add_term(stx, space, term);

	return stx;
}

static int term_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, stx_t *stx, stx_node_t *term)
{
	prs_node_t prs_rule_name;
	if (prs_get_rule(prs, parent, bnf->rname, &prs_rule_name) == 0) {
		tok_t str = {0};
		prs_get_str(prs, prs_rule_name, &str);
		strv_t name = lex_get_tok_val(prs->lex, str);

		stx_node_t term_rule;
		if (stx_find_rule(stx, name, &term_rule)) {
			stx_rule(stx, name, &term_rule);
		}

		return stx_term_rule(stx, term_rule, term);
	}

	prs_node_t prs_literal;
	if (prs_get_rule(prs, parent, bnf->literal, &prs_literal) == 0) {
		prs_node_t prs_text_double;
		if (prs_get_rule(prs, prs_literal, bnf->tdouble, &prs_text_double) == 0) {
			tok_t str = {0};
			prs_get_str(prs, prs_text_double, &str);
			return stx_term_lit(stx, lex_get_tok_val(prs->lex, str), term);
		}

		prs_node_t prs_text_single;
		if (prs_get_rule(prs, prs_literal, bnf->tsingle, &prs_text_single) == 0) {
			tok_t str = {0};
			prs_get_str(prs, prs_text_single, &str);
			return stx_term_lit(stx, lex_get_tok_val(prs->lex, str), term);
		}

		return 1;
	}

	prs_node_t prs_tok;
	if (prs_get_rule(prs, parent, bnf->tok, &prs_tok) == 0) {
		tok_t str = {0};
		prs_get_str(prs, prs_tok, &str);
		return stx_term_tok(stx, tok_type_enum(lex_get_tok_val(prs->lex, str)), term);
	}

	return 1;
}

static int terms_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, stx_t *stx, stx_node_t *term)
{
	prs_node_t prs_term;
	prs_get_rule(prs, parent, bnf->term, &prs_term);
	term_from_bnf(bnf, prs, prs_term, stx, term);

	prs_node_t prs_terms;
	if (prs_get_rule(prs, parent, bnf->terms, &prs_terms) == 0) {
		stx_node_t next;
		terms_from_bnf(bnf, prs, prs_terms, stx, &next);
		stx_add_term(stx, *term, next);
	}

	return 0;
}

static int exprs_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, stx_t *stx, stx_node_t *term)
{
	prs_node_t prs_expr;
	if (prs_get_rule(prs, parent, bnf->expr, &prs_expr) == 0) {
		prs_node_t prs_terms;
		prs_get_rule(prs, parent, bnf->terms, &prs_terms);

		stx_node_t l, r;
		terms_from_bnf(bnf, prs, prs_terms, stx, &l);
		exprs_from_bnf(bnf, prs, prs_expr, stx, &r);
		return stx_term_or(stx, l, r, term);
	}

	prs_node_t prs_terms;
	prs_get_rule(prs, parent, bnf->terms, &prs_terms);
	return terms_from_bnf(bnf, prs, prs_terms, stx, term);
}

static int rules_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, stx_t *stx, stx_node_t *root)
{
	prs_node_t prs_rule, prs_rname;
	prs_get_rule(prs, parent, bnf->rule, &prs_rule);
	prs_get_rule(prs, prs_rule, bnf->rname, &prs_rname);

	tok_t str = {0};
	prs_get_str(prs, prs_rname, &str);
	strv_t name = lex_get_tok_val(prs->lex, str);

	stx_node_t rule;
	if (stx_find_rule(stx, name, &rule)) {
		stx_rule(stx, name, &rule);
	}

	prs_node_t prs_expr;
	prs_get_rule(prs, prs_rule, bnf->expr, &prs_expr);
	stx_node_t term;
	exprs_from_bnf(bnf, prs, prs_expr, stx, &term);
	stx_add_term(stx, rule, term);

	if (root) {
		*root = rule;
	}

	prs_node_t rules;
	if (prs_get_rule(prs, parent, bnf->rules, &rules)) {
		return 0;
	}

	return rules_from_bnf(bnf, prs, rules, stx, NULL);
}

int stx_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t root, stx_t *stx, stx_node_t *rule)
{
	if (bnf == NULL) {
		return 1;
	}

	prs_node_t fbnf, rules;
	prs_get_rule(prs, root, bnf->bnf, &fbnf);
	prs_get_rule(prs, fbnf, bnf->rules, &rules);

	return rules_from_bnf(bnf, prs, rules, stx, rule);
}
