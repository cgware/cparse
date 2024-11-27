#include "bnf.h"

#include "log.h"
#include "token.h"

bnf_t *bnf_init(bnf_t *bnf)
{
	if (bnf == NULL) {
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

const stx_t *bnf_get_stx(bnf_t *bnf, alloc_t alloc)
{
	if (bnf == NULL) {
		return NULL;
	}

	stx_t *stx = &bnf->stx;

	if (stx_init(stx, 20, 90, alloc) == NULL) {
		log_error("cutils", "bnf", NULL, "failed to intialize syntax");
		return NULL;
	}

	bnf->file		   = stx_add_rule(stx, STR("file"));
	bnf->bnf		   = stx_add_rule(stx, STR("bnf"));
	bnf->rules		   = stx_add_rule(stx, STR("rules"));
	bnf->rule		   = stx_add_rule(stx, STR("rule"));
	bnf->rname		   = stx_add_rule(stx, STR("rname"));
	const stx_rule_t rchars	   = stx_add_rule(stx, STR("rchars"));
	const stx_rule_t rchar	   = stx_add_rule(stx, STR("rchar"));
	bnf->expr		   = stx_add_rule(stx, STR("expr"));
	bnf->terms		   = stx_add_rule(stx, STR("terms"));
	bnf->term		   = stx_add_rule(stx, STR("term"));
	bnf->literal		   = stx_add_rule(stx, STR("literal"));
	bnf->token		   = stx_add_rule(stx, STR("token"));
	bnf->tdouble		   = stx_add_rule(stx, STR("tdouble"));
	bnf->tsingle		   = stx_add_rule(stx, STR("tsingle"));
	const stx_rule_t cdouble   = stx_add_rule(stx, STR("cdouble"));
	const stx_rule_t csingle   = stx_add_rule(stx, STR("csingle"));
	const stx_rule_t character = stx_add_rule(stx, STR("char"));
	const stx_rule_t spaces	   = stx_add_rule(stx, STR("spaces"));
	const stx_rule_t space	   = stx_add_rule(stx, STR("space"));

	stx_rule_add_term(stx, bnf->file, STX_TERM_RULE(stx, bnf->bnf));
	stx_rule_add_term(stx, bnf->file, STX_TERM_TOKEN(stx, TOKEN_EOF));

	stx_rule_add_term(stx, bnf->bnf, STX_TERM_RULE(stx, bnf->rules));

	stx_rule_add_arr(stx, bnf->rules, STX_TERM_RULE(stx, bnf->rule), STX_TERM_NONE(stx));

	stx_rule_add_term(stx, bnf->rule, STX_TERM_LITERAL(stx, STR("<")));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_RULE(stx, bnf->rname));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_LITERAL(stx, STR(">")));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_RULE(stx, spaces));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_LITERAL(stx, STR("::=")));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_RULE(stx, space));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_RULE(stx, bnf->expr));
	stx_rule_add_term(stx, bnf->rule, STX_TERM_TOKEN(stx, TOKEN_NL));

	const stx_term_t rname_l = STX_TERM_TOKEN(stx, TOKEN_LOWER);
	stx_term_add_term(stx, rname_l, STX_TERM_RULE(stx, rchars));
	stx_rule_add_term(stx, bnf->rname, STX_TERM_OR(stx, rname_l, STX_TERM_TOKEN(stx, TOKEN_LOWER)));

	stx_rule_add_arr(stx, rchars, STX_TERM_RULE(stx, rchar), STX_TERM_NONE(stx));

	stx_rule_add_term(stx, rchar, STX_TERM_OR(stx, STX_TERM_TOKEN(stx, TOKEN_LOWER), STX_TERM_LITERAL(stx, STR("-"))));

	const stx_term_t expr_l = STX_TERM_RULE(stx, bnf->terms);
	stx_term_add_term(stx, expr_l, STX_TERM_RULE(stx, space));
	stx_term_add_term(stx, expr_l, STX_TERM_LITERAL(stx, STR("|")));
	stx_term_add_term(stx, expr_l, STX_TERM_RULE(stx, space));
	stx_term_add_term(stx, expr_l, STX_TERM_RULE(stx, bnf->expr));
	stx_rule_add_term(stx, bnf->expr, STX_TERM_OR(stx, expr_l, STX_TERM_RULE(stx, bnf->terms)));

	stx_rule_add_arr(stx, bnf->terms, STX_TERM_RULE(stx, bnf->term), STX_TERM_RULE(stx, space));

	const stx_term_t term_rule = STX_TERM_LITERAL(stx, STR("<"));
	stx_term_add_term(stx, term_rule, STX_TERM_RULE(stx, bnf->rname));
	stx_term_add_term(stx, term_rule, STX_TERM_LITERAL(stx, STR(">")));
	stx_rule_add_or(stx, bnf->term, 3, STX_TERM_RULE(stx, bnf->literal), STX_TERM_RULE(stx, bnf->token), term_rule);

	const stx_term_t lsingle = STX_TERM_LITERAL(stx, STR("'"));
	stx_term_add_term(stx, lsingle, STX_TERM_RULE(stx, bnf->tdouble));
	stx_term_add_term(stx, lsingle, STX_TERM_LITERAL(stx, STR("'")));
	const stx_term_t ldouble = STX_TERM_LITERAL(stx, STR("\""));
	stx_term_add_term(stx, ldouble, STX_TERM_RULE(stx, bnf->tsingle));
	stx_term_add_term(stx, ldouble, STX_TERM_LITERAL(stx, STR("\"")));
	stx_rule_add_term(stx, bnf->literal, STX_TERM_OR(stx, lsingle, ldouble));

	stx_rule_add_arr(stx, bnf->token, STX_TERM_TOKEN(stx, TOKEN_UPPER), STX_TERM_NONE(stx));

	stx_rule_add_arr(stx, bnf->tdouble, STX_TERM_RULE(stx, cdouble), STX_TERM_NONE(stx));
	stx_rule_add_arr(stx, bnf->tsingle, STX_TERM_RULE(stx, csingle), STX_TERM_NONE(stx));

	stx_rule_add_term(stx, cdouble, STX_TERM_OR(stx, STX_TERM_RULE(stx, character), STX_TERM_LITERAL(stx, STR("\""))));
	stx_rule_add_term(stx, csingle, STX_TERM_OR(stx, STX_TERM_RULE(stx, character), STX_TERM_LITERAL(stx, STR("'"))));

	stx_rule_add_or(stx,
			character,
			4,
			STX_TERM_TOKEN(stx, TOKEN_ALPHA),
			STX_TERM_TOKEN(stx, TOKEN_DIGIT),
			STX_TERM_TOKEN(stx, TOKEN_SYMBOL),
			STX_TERM_RULE(stx, space));

	stx_rule_add_arr(stx, spaces, STX_TERM_RULE(stx, space), STX_TERM_NONE(stx));

	stx_rule_add_term(stx, space, STX_TERM_LITERAL(stx, STR(" ")));

	stx_compile(stx);

	return stx;
}

static stx_term_t term_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, stx_t *stx)
{
	const prs_node_t prs_rule_name = prs_get_rule(prs, parent, bnf->rname);
	if (prs_rule_name < prs->nodes.cnt) {
		str_t rule_name = strz(16);
		prs_get_str(prs, prs_rule_name, &rule_name);

		stx_rule_t term_rule = stx_get_rule(stx, rule_name);
		if (term_rule >= stx->rules.cnt) {
			term_rule = stx_add_rule(stx, rule_name);
		} else {
			str_free(&rule_name);
		}

		return STX_TERM_RULE(stx, term_rule);
	}

	const prs_node_t prs_literal = prs_get_rule(prs, parent, bnf->literal);
	if (prs_literal < prs->nodes.cnt) {
		const prs_node_t prs_text_double = prs_get_rule(prs, prs_literal, bnf->tdouble);
		if (prs_text_double < prs->nodes.cnt) {
			str_t literal = strz(16);
			prs_get_str(prs, prs_text_double, &literal);
			return STX_TERM_LITERAL(stx, literal);
		}

		const prs_node_t prs_text_single = prs_get_rule(prs, prs_literal, bnf->tsingle);
		if (prs_text_single < prs->nodes.cnt) {
			str_t literal = strz(16);
			prs_get_str(prs, prs_text_single, &literal);
			return STX_TERM_LITERAL(stx, literal);
		}

		return STX_TERM_END;
	}

	const prs_node_t prs_token = prs_get_rule(prs, parent, bnf->token);
	if (prs_token < prs->nodes.cnt) {
		str_t token = strz(16);
		prs_get_str(prs, prs_token, &token);
		const stx_term_t term = STX_TERM_TOKEN(stx, token_type_enum(token));
		str_free(&token);
		return term;
	}

	return STX_TERM_END;
}

static stx_term_t terms_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, stx_t *stx)
{
	const prs_node_t prs_term = prs_get_rule(prs, parent, bnf->term);
	const stx_term_t term	  = term_from_bnf(bnf, prs, prs_term, stx);

	const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);
	if (prs_terms < prs->nodes.cnt) {
		stx_term_add_term(stx, term, terms_from_bnf(bnf, prs, prs_terms, stx));
	}

	return term;
}

static stx_term_t exprs_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, stx_t *stx)
{
	const prs_node_t prs_expr = prs_get_rule(prs, parent, bnf->expr);
	if (prs_expr < prs->nodes.cnt) {
		const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);

		const stx_term_t left  = terms_from_bnf(bnf, prs, prs_terms, stx);
		const stx_term_t right = exprs_from_bnf(bnf, prs, prs_expr, stx);

		return STX_TERM_OR(stx, left, right);
	}

	const prs_node_t prs_terms = prs_get_rule(prs, parent, bnf->terms);
	return terms_from_bnf(bnf, prs, prs_terms, stx);
}

static stx_rule_t rules_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t parent, stx_t *stx)
{
	const prs_node_t prs_rule  = prs_get_rule(prs, parent, bnf->rule);
	const prs_node_t prs_rname = prs_get_rule(prs, prs_rule, bnf->rname);

	str_t rname = strz(16);
	prs_get_str(prs, prs_rname, &rname);
	stx_rule_t rule = stx_get_rule(stx, rname);
	if (rule >= stx->rules.cnt) {
		rule = stx_add_rule(stx, rname);
	} else {
		str_free(&rname);
	}

	const prs_node_t prs_expr = prs_get_rule(prs, prs_rule, bnf->expr);
	const stx_term_t term	  = exprs_from_bnf(bnf, prs, prs_expr, stx);
	stx_rule_set_term(stx, rule, term);

	const prs_node_t rules = prs_get_rule(prs, parent, bnf->rules);
	if (rules >= prs->nodes.cnt) {
		return rule;
	}

	rules_from_bnf(bnf, prs, rules, stx);

	return rule;
}

stx_rule_t stx_from_bnf(const bnf_t *bnf, const prs_t *prs, prs_node_t root, stx_t *stx)
{
	prs_node_t fbnf	 = prs_get_rule(prs, root, bnf->bnf);
	prs_node_t rules = prs_get_rule(prs, fbnf, bnf->rules);

	stx_rule_t stx_root = rules_from_bnf(bnf, prs, rules, stx);

	stx_compile(stx);

	return stx_root;
}
