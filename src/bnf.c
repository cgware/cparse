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

	bnf->file		   = stx_add_rule(stx);
	bnf->bnf		   = stx_add_rule(stx);
	bnf->rules		   = stx_add_rule(stx);
	bnf->rule		   = stx_add_rule(stx);
	bnf->rname		   = stx_add_rule(stx);
	const stx_rule_t rchars	   = stx_add_rule(stx);
	const stx_rule_t rchar	   = stx_add_rule(stx);
	bnf->expr		   = stx_add_rule(stx);
	bnf->terms		   = stx_add_rule(stx);
	bnf->term		   = stx_add_rule(stx);
	bnf->literal		   = stx_add_rule(stx);
	bnf->token		   = stx_add_rule(stx);
	bnf->tdouble		   = stx_add_rule(stx);
	bnf->tsingle		   = stx_add_rule(stx);
	const stx_rule_t cdouble   = stx_add_rule(stx);
	const stx_rule_t csingle   = stx_add_rule(stx);
	const stx_rule_t character = stx_add_rule(stx);
	const stx_rule_t spaces	   = stx_add_rule(stx);
	const stx_rule_t space	   = stx_add_rule(stx);

	stx_rule_add_term(stx, bnf->file, STX_TERM_RULE(stx, bnf->bnf));
	stx_rule_add_term(stx, bnf->file, STX_TERM_TOKEN(stx, TOKEN_EOF));

	stx_rule_add_term(stx, bnf->bnf, STX_TERM_RULE(stx, bnf->rules));

	stx_rule_add_arr(stx, bnf->rules, STX_TERM_RULE(stx, bnf->rule), STX_TERM_NONE(stx));

	stx_rule_add_term(stx, bnf->rule, STX_TERM_LITERAL(stx, STRV("<")));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_RULE(stx, bnf->rname));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_LITERAL(stx, STRV(">")));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_RULE(stx, spaces));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_LITERAL(stx, STRV("::=")));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_RULE(stx, space));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_RULE(stx, bnf->expr));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_TOKEN(stx, TOKEN_NL));

	const stx_term_t rname_l = STX_TERM_TOKEN(stx, TOKEN_LOWER);
	stx_term_add_term(stx, rname_l, STX_TERM_RULE(stx, rchars));
	stx_rule_add_term(stx, bnf->rname, STX_TERM_OR(stx, rname_l, STX_TERM_TOKEN(stx, TOKEN_LOWER)));

	stx_rule_add_arr(stx, rchars, STX_TERM_RULE(stx, rchar), STX_TERM_NONE(stx));

	stx_rule_add_term(stx, rchar, STX_TERM_OR(stx, STX_TERM_TOKEN(stx, TOKEN_LOWER), STX_TERM_LITERAL(stx, STRV("-"))));

	const stx_term_t expr_l = STX_TERM_RULE(stx, bnf->terms);
	stx_term_add_term(stx, expr_l, STX_TERM_RULE(stx, space));
	stx_term_add_term(stx, expr_l, STX_TERM_LITERAL(stx, STRV("|")));
	stx_term_add_term(stx, expr_l, STX_TERM_RULE(stx, space));
	stx_term_add_term(stx, expr_l, STX_TERM_RULE(stx, bnf->expr));
	stx_rule_add_term(stx, bnf->expr, STX_TERM_OR(stx, expr_l, STX_TERM_RULE(stx, bnf->terms)));

	stx_rule_add_arr(stx, bnf->terms, STX_TERM_RULE(stx, bnf->term), STX_TERM_RULE(stx, space));

	const stx_term_t term_rule = STX_TERM_LITERAL(stx, STRV("<"));
	stx_term_add_term(stx, term_rule, STX_TERM_RULE(stx, bnf->rname));
	stx_term_add_term(stx, term_rule, STX_TERM_LITERAL(stx, STRV(">")));
	stx_rule_add_or(stx, bnf->term, 3, STX_TERM_RULE(stx, bnf->literal), STX_TERM_RULE(stx, bnf->token), term_rule);

	const stx_term_t lsingle = STX_TERM_LITERAL(stx, STRV("'"));
	stx_term_add_term(stx, lsingle, STX_TERM_RULE(stx, bnf->tdouble));
	stx_term_add_term(stx, lsingle, STX_TERM_LITERAL(stx, STRV("'")));
	const stx_term_t ldouble = STX_TERM_LITERAL(stx, STRV("\""));
	stx_term_add_term(stx, ldouble, STX_TERM_RULE(stx, bnf->tsingle));
	stx_term_add_term(stx, ldouble, STX_TERM_LITERAL(stx, STRV("\"")));
	stx_rule_add_term(stx, bnf->literal, STX_TERM_OR(stx, lsingle, ldouble));

	stx_rule_add_arr(stx, bnf->token, STX_TERM_TOKEN(stx, TOKEN_UPPER), STX_TERM_NONE(stx));

	stx_rule_add_arr(stx, bnf->tdouble, STX_TERM_RULE(stx, cdouble), STX_TERM_NONE(stx));
	stx_rule_add_arr(stx, bnf->tsingle, STX_TERM_RULE(stx, csingle), STX_TERM_NONE(stx));

	stx_rule_add_term(stx, cdouble, STX_TERM_OR(stx, STX_TERM_RULE(stx, character), STX_TERM_LITERAL(stx, STRV("\""))));
	stx_rule_add_term(stx, csingle, STX_TERM_OR(stx, STX_TERM_RULE(stx, character), STX_TERM_LITERAL(stx, STRV("'"))));

	stx_rule_add_or(stx,
			character,
			4,
			STX_TERM_TOKEN(stx, TOKEN_ALPHA),
			STX_TERM_TOKEN(stx, TOKEN_DIGIT),
			STX_TERM_TOKEN(stx, TOKEN_SYMBOL),
			STX_TERM_RULE(stx, space));

	stx_rule_add_arr(stx, spaces, STX_TERM_RULE(stx, space), STX_TERM_NONE(stx));

	stx_rule_add_term(stx, space, STX_TERM_LITERAL(stx, STRV(" ")));

	return stx;
}

static stx_term_t term_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, strbuf_t *names, stx_t *stx)
{
	const prs_node_t prs_rule_name = prs_get_rule(prs, parent, bnf->rname);
	if (prs_rule_name < prs->nodes.cnt) {
		token_t str = {0};
		prs_get_str(prs, prs_rule_name, &str);
		strv_t name = lex_get_token_val(prs->lex, str);

		stx_rule_t term_rule;
		if (strbuf_find(names, name, &term_rule)) {
			strbuf_add(names, name, &term_rule);
			stx_add_rule(stx);
		}

		return STX_TERM_RULE(stx, term_rule);
	}

	const prs_node_t prs_literal = prs_get_rule(prs, parent, bnf->literal);
	if (prs_literal < prs->nodes.cnt) {
		const prs_node_t prs_text_double = prs_get_rule(prs, prs_literal, bnf->tdouble);
		if (prs_text_double < prs->nodes.cnt) {
			token_t str = {0};
			prs_get_str(prs, prs_text_double, &str);
			return STX_TERM_LITERAL(stx, lex_get_token_val(prs->lex, str));
		}

		const prs_node_t prs_text_single = prs_get_rule(prs, prs_literal, bnf->tsingle);
		if (prs_text_single < prs->nodes.cnt) {
			token_t str = {0};
			prs_get_str(prs, prs_text_single, &str);
			return STX_TERM_LITERAL(stx, lex_get_token_val(prs->lex, str));
		}

		return STX_TERM_END;
	}

	const prs_node_t prs_token = prs_get_rule(prs, parent, bnf->token);
	if (prs_token < prs->nodes.cnt) {
		token_t str = {0};
		prs_get_str(prs, prs_token, &str);
		const stx_term_t term = STX_TERM_TOKEN(stx, token_type_enum(lex_get_token_val(prs->lex, str)));
		return term;
	}

	return STX_TERM_END;
}

static stx_term_t terms_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, strbuf_t *names, stx_t *stx)
{
	const prs_node_t prs_term = prs_get_rule(prs, parent, bnf->term);
	const stx_term_t term	  = term_from_bnf(bnf, prs, prs_term, names, stx);

	const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);
	if (prs_terms < prs->nodes.cnt) {
		stx_term_add_term(stx, term, terms_from_bnf(bnf, prs, prs_terms, names, stx));
	}

	return term;
}

static stx_term_t exprs_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, strbuf_t *names, stx_t *stx)
{
	const prs_node_t prs_expr = prs_get_rule(prs, parent, bnf->expr);
	if (prs_expr < prs->nodes.cnt) {
		const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);

		const stx_term_t left  = terms_from_bnf(bnf, prs, prs_terms, names, stx);
		const stx_term_t right = exprs_from_bnf(bnf, prs, prs_expr, names, stx);

		return STX_TERM_OR(stx, left, right);
	}

	const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);
	return terms_from_bnf(bnf, prs, prs_terms, names, stx);
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
		stx_add_rule(stx);
	}

	const prs_node_t prs_expr = prs_get_rule(prs, prs_rule, bnf->expr);
	const stx_term_t term	  = exprs_from_bnf(bnf, prs, prs_expr, names, stx);
	stx_rule_set_term(stx, rule, term);

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
