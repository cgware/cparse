#include "bnf.h"

#include "log.h"
#include "strbuf.h"
#include "token.h"

bnf_t *bnf_init(bnf_t *bnf, alloc_t alloc)
{
	if (bnf == NULL) {
		return NULL;
	}

	if (stx_init(&bnf->stx, 20, 90, alloc) == NULL) {
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

	stx_rule_t rchars, rchar, cdouble, csingle, character, spaces, space;
	stx_add_rule(stx, &bnf->file);
	stx_add_rule(stx, &bnf->bnf);
	stx_add_rule(stx, &bnf->rules);
	stx_add_rule(stx, &bnf->rule);
	stx_add_rule(stx, &bnf->rname);
	stx_add_rule(stx, &rchars);
	stx_add_rule(stx, &rchar);
	stx_add_rule(stx, &bnf->expr);
	stx_add_rule(stx, &bnf->terms);
	stx_add_rule(stx, &bnf->term);
	stx_add_rule(stx, &bnf->literal);
	stx_add_rule(stx, &bnf->token);
	stx_add_rule(stx, &bnf->tdouble);
	stx_add_rule(stx, &bnf->tsingle);
	stx_add_rule(stx, &cdouble);
	stx_add_rule(stx, &csingle);
	stx_add_rule(stx, &character);
	stx_add_rule(stx, &spaces);
	stx_add_rule(stx, &space);

	stx_term_t term;

	stx_term_rule(stx, bnf->bnf, &term);
	stx_rule_add_term(stx, bnf->file, term);
	stx_term_tok(stx, TOKEN_EOF, &term);
	stx_rule_add_term(stx, bnf->file, term);

	stx_term_rule(stx, bnf->rules, &term);
	stx_rule_add_term(stx, bnf->bnf, term);

	stx_term_rule(stx, bnf->rule, &term);
	stx_rule_add_arr(stx, bnf->rules, term);

	stx_term_lit(stx, STRV("<"), &term);
	stx_rule_add_term(stx, bnf->rule, term);
	stx_term_rule(stx, bnf->rname, &term);
	stx_rule_add_term(stx, bnf->rule, term);
	stx_term_lit(stx, STRV(">"), &term);
	stx_rule_add_term(stx, bnf->rule, term);
	stx_term_rule(stx, spaces, &term);
	stx_rule_add_term(stx, bnf->rule, term);
	stx_term_lit(stx, STRV("::="), &term);
	stx_rule_add_term(stx, bnf->rule, term);
	stx_term_rule(stx, space, &term);
	stx_rule_add_term(stx, bnf->rule, term);
	stx_term_rule(stx, bnf->expr, &term);
	stx_rule_add_term(stx, bnf->rule, term);
	stx_term_tok(stx, TOKEN_NL, &term);
	stx_rule_add_term(stx, bnf->rule, term);

	stx_term_t l, r;
	stx_term_tok(stx, TOKEN_LOWER, &l);
	stx_term_rule(stx, rchars, &term);
	stx_term_add_term(stx, l, term);
	stx_term_tok(stx, TOKEN_LOWER, &r);
	stx_term_t orv;
	stx_term_or(stx, l, r, &orv);
	stx_rule_add_term(stx, bnf->rname, orv);

	stx_term_rule(stx, rchar, &term);
	stx_rule_add_arr(stx, rchars, term);

	stx_term_tok(stx, TOKEN_LOWER, &l);
	stx_term_lit(stx, STRV("-"), &r);
	stx_term_or(stx, l, r, &orv);
	stx_rule_add_term(stx, rchar, orv);

	stx_term_rule(stx, bnf->terms, &l);
	stx_term_rule(stx, space, &term);
	stx_term_add_term(stx, l, term);
	stx_term_lit(stx, STRV("|"), &term);
	stx_term_add_term(stx, l, term);
	stx_term_rule(stx, space, &term);
	stx_term_add_term(stx, l, term);
	stx_term_rule(stx, bnf->expr, &term);
	stx_term_add_term(stx, l, term);
	stx_term_rule(stx, bnf->terms, &r);
	stx_term_or(stx, l, r, &orv);
	stx_rule_add_term(stx, bnf->expr, orv);

	stx_term_rule(stx, bnf->term, &l);
	stx_term_rule(stx, space, &r);
	stx_rule_add_arr_sep(stx, bnf->terms, l, r);

	stx_term_lit(stx, STRV("<"), &l);
	stx_term_rule(stx, bnf->rname, &term);
	stx_term_add_term(stx, l, term);
	stx_term_lit(stx, STRV(">"), &term);
	stx_term_add_term(stx, l, term);
	stx_term_rule(stx, bnf->token, &r);
	stx_term_rule(stx, bnf->literal, &term);
	stx_rule_add_or(stx, bnf->term, 3, term, r, l);

	stx_term_lit(stx, STRV("'"), &l);
	stx_term_rule(stx, bnf->tdouble, &term);
	stx_term_add_term(stx, l, term);
	stx_term_lit(stx, STRV("'"), &term);
	stx_term_add_term(stx, l, term);
	stx_term_lit(stx, STRV("\""), &r);
	stx_term_rule(stx, bnf->tsingle, &term);
	stx_term_add_term(stx, r, term);
	stx_term_lit(stx, STRV("\""), &term);
	stx_term_add_term(stx, r, term);
	stx_term_or(stx, l, r, &orv);
	stx_rule_add_term(stx, bnf->literal, orv);

	stx_term_tok(stx, TOKEN_UPPER, &term);
	stx_rule_add_arr(stx, bnf->token, term);

	stx_term_rule(stx, cdouble, &term);
	stx_rule_add_arr(stx, bnf->tdouble, term);
	stx_term_rule(stx, csingle, &term);
	stx_rule_add_arr(stx, bnf->tsingle, term);

	stx_term_rule(stx, character, &l);
	stx_term_lit(stx, STRV("\""), &r);
	stx_term_or(stx, l, r, &orv);
	stx_rule_add_term(stx, cdouble, orv);
	stx_term_rule(stx, character, &l);
	stx_term_lit(stx, STRV("'"), &r);
	stx_term_or(stx, l, r, &orv);
	stx_rule_add_term(stx, csingle, orv);

	stx_term_t t0, t1, t2, t3;
	stx_term_tok(stx, TOKEN_ALPHA, &t0);
	stx_term_tok(stx, TOKEN_DIGIT, &t1);
	stx_term_tok(stx, TOKEN_SYMBOL, &t2);
	stx_term_rule(stx, space, &t3);
	stx_rule_add_or(stx, character, 4, t0, t1, t2, t3);

	stx_term_rule(stx, space, &term);
	stx_rule_add_arr(stx, spaces, term);

	stx_term_lit(stx, STRV(" "), &term);
	stx_rule_add_term(stx, space, term);

	return stx;
}

static int term_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, strbuf_t *names, stx_t *stx, stx_term_t *term)
{
	const prs_node_t prs_rule_name = prs_get_rule(prs, parent, bnf->rname);
	if (prs_rule_name < prs->nodes.cnt) {
		token_t str = {0};
		prs_get_str(prs, prs_rule_name, &str);
		strv_t name = lex_get_token_val(prs->lex, str);

		stx_rule_t term_rule;
		if (strbuf_find(names, name, &term_rule)) {
			strbuf_add(names, name, &term_rule);
			stx_add_rule(stx, NULL);
		}

		return stx_term_rule(stx, term_rule, term);
	}

	const prs_node_t prs_literal = prs_get_rule(prs, parent, bnf->literal);
	if (prs_literal < prs->nodes.cnt) {
		const prs_node_t prs_text_double = prs_get_rule(prs, prs_literal, bnf->tdouble);
		if (prs_text_double < prs->nodes.cnt) {
			token_t str = {0};
			prs_get_str(prs, prs_text_double, &str);
			return stx_term_lit(stx, lex_get_token_val(prs->lex, str), term);
		}

		const prs_node_t prs_text_single = prs_get_rule(prs, prs_literal, bnf->tsingle);
		if (prs_text_single < prs->nodes.cnt) {
			token_t str = {0};
			prs_get_str(prs, prs_text_single, &str);
			return stx_term_lit(stx, lex_get_token_val(prs->lex, str), term);
		}

		return 1;
	}

	const prs_node_t prs_token = prs_get_rule(prs, parent, bnf->token);
	if (prs_token < prs->nodes.cnt) {
		token_t str = {0};
		prs_get_str(prs, prs_token, &str);
		return stx_term_tok(stx, token_type_enum(lex_get_token_val(prs->lex, str)), term);
	}

	return 1;
}

static int terms_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, strbuf_t *names, stx_t *stx, stx_term_t *term)
{
	const prs_node_t prs_term = prs_get_rule(prs, parent, bnf->term);
	term_from_bnf(bnf, prs, prs_term, names, stx, term);

	const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);
	if (prs_terms < prs->nodes.cnt) {
		stx_term_t next;
		terms_from_bnf(bnf, prs, prs_terms, names, stx, &next);
		stx_term_add_term(stx, *term, next);
	}

	return 0;
}

static int exprs_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, strbuf_t *names, stx_t *stx, stx_term_t *term)
{
	const prs_node_t prs_expr = prs_get_rule(prs, parent, bnf->expr);
	if (prs_expr < prs->nodes.cnt) {
		const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);

		stx_term_t l, r;
		terms_from_bnf(bnf, prs, prs_terms, names, stx, &l);
		exprs_from_bnf(bnf, prs, prs_expr, names, stx, &r);
		return stx_term_or(stx, l, r, term);
	}

	const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);
	return terms_from_bnf(bnf, prs, prs_terms, names, stx, term);
}

static stx_rule_t rules_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, strbuf_t *names, stx_t *stx)
{
	const prs_node_t prs_rule  = prs_get_rule(prs, parent, bnf->rule);
	const prs_node_t prs_rname = prs_get_rule(prs, prs_rule, bnf->rname);

	token_t str = {0};
	prs_get_str(prs, prs_rname, &str);
	strv_t name = lex_get_token_val(prs->lex, str);

	stx_rule_t rule;
	if (strbuf_find(names, name, &rule)) {
		strbuf_add(names, name, &rule);
		stx_add_rule(stx, NULL);
	}

	const prs_node_t prs_expr = prs_get_rule(prs, prs_rule, bnf->expr);
	stx_term_t term;
	exprs_from_bnf(bnf, prs, prs_expr, names, stx, &term);
	stx_rule_add_term(stx, rule, term);

	const prs_node_t rules = prs_get_rule(prs, parent, bnf->rules);
	if (rules >= prs->nodes.cnt) {
		return rule;
	}

	rules_from_bnf(bnf, prs, rules, names, stx);

	return rule;
}

stx_rule_t stx_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t root, stx_t *stx, strbuf_t *names)
{
	prs_node_t fbnf	 = prs_get_rule(prs, root, bnf->bnf);
	prs_node_t rules = prs_get_rule(prs, fbnf, bnf->rules);

	stx_rule_t stx_root = rules_from_bnf(bnf, prs, rules, names, stx);

	return stx_root;
}
