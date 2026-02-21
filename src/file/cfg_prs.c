#include "file/cfg_prs.h"

#include "ebnf.h"
#include "eprs.h"
#include "log.h"
#include "str.h"

cfg_prs_t *cfg_prs_init(cfg_prs_t *cfg_prs, alloc_t alloc)
{
	if (cfg_prs == NULL) {
		return NULL;
	}

	uint line      = __LINE__ + 1;
	strv_t cfg_bnf = STRV("file = cfg EOF\n"
			      "cfg  = (tv NL)* tbl? (NL tbl)*\n"
			      "tv   = kv | (key ':' NL vals? NL) | val\n"
			      "vals = val (NL val)*\n"
			      "key  = (ALPHA | '.')+\n"
			      "val  = int | '\"' str '\"' | lit | '[' arr? ']' | '{' obj? '}'\n"
			      "int  = DIGIT+\n"
			      "str  = c*\n"
			      "lit  = (ALPHA | DIGIT | '_') (ALPHA | DIGIT | '_' | ':')*\n"
			      "arr  = val (', ' val)*\n"
			      "obj  = kv (', ' kv)*\n"
			      "kv   = key ' ' mode? '= ' val\n"
			      "mode = '+' | '-' | '?'\n"
			      "c    = ALPHA | DIGIT | SYMBOL | ' ' | \"'\"\n"
			      "tbl  = '[' name ']' NL ent\n"
			      "name = (ALPHA | DIGIT | '_' | ':' | '.' | '=' | '+' | '?' | '-')+\n"
			      "ent  = (tv NL)*\n");

	lex_t lex = {0};
	if (lex_init(&lex, 0, 512, alloc) == NULL) {
		log_error("cparse", "bnf", NULL, "failed to intialize lexer");
		return NULL;
	}

	lex_tokenize(&lex, cfg_bnf, STRV(__FILE__), line);

	ebnf_t ebnf = {0};
	ebnf_init(&ebnf, alloc);
	ebnf_get_stx(&ebnf, alloc, DST_NONE());

	prs_t prs = {0};
	prs_init(&prs, 1024, ALLOC_STD);

	prs_node_t prs_root;
	prs_parse(&prs, &lex, &ebnf.stx, ebnf.file, &prs_root, DST_NONE());

	ebnf_free(&ebnf);

	estx_init(&cfg_prs->estx, 128, ALLOC_STD);
	estx_from_ebnf(&ebnf, &prs, prs_root, &cfg_prs->estx, NULL);

	estx_find_rule(&cfg_prs->estx, STRV("file"), &cfg_prs->file);
	estx_find_rule(&cfg_prs->estx, STRV("cfg"), &cfg_prs->cfg);
	estx_find_rule(&cfg_prs->estx, STRV("tv"), &cfg_prs->tv);
	estx_find_rule(&cfg_prs->estx, STRV("vals"), &cfg_prs->vals);
	estx_find_rule(&cfg_prs->estx, STRV("key"), &cfg_prs->key);
	estx_find_rule(&cfg_prs->estx, STRV("val"), &cfg_prs->val);
	estx_find_rule(&cfg_prs->estx, STRV("int"), &cfg_prs->i);
	estx_find_rule(&cfg_prs->estx, STRV("str"), &cfg_prs->str);
	estx_find_rule(&cfg_prs->estx, STRV("lit"), &cfg_prs->lit);
	estx_find_rule(&cfg_prs->estx, STRV("arr"), &cfg_prs->arr);
	estx_find_rule(&cfg_prs->estx, STRV("obj"), &cfg_prs->obj);
	estx_find_rule(&cfg_prs->estx, STRV("kv"), &cfg_prs->kv);
	estx_find_rule(&cfg_prs->estx, STRV("mode"), &cfg_prs->mode);
	estx_find_rule(&cfg_prs->estx, STRV("tbl"), &cfg_prs->tbl);
	estx_find_rule(&cfg_prs->estx, STRV("name"), &cfg_prs->name);
	estx_find_rule(&cfg_prs->estx, STRV("ent"), &cfg_prs->ent);

	prs_free(&prs);
	lex_free(&lex);

	return cfg_prs;
}

void cfg_prs_free(cfg_prs_t *cfg_prs)
{
	estx_free(&cfg_prs->estx);
}

static int cfg_parse_value(const cfg_prs_t *cfg_prs, eprs_t *eprs, strv_t key, cfg_mode_t mode, eprs_node_t value, cfg_t *cfg,
			   cfg_var_t *var);

static cfg_var_t cfg_parse_kv(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t kv, cfg_t *cfg, cfg_var_t *var)
{
	eprs_node_t node;
	tok_t key   = {0};
	tok_t tmode = {0};

	if (eprs_get_rule(eprs, kv, cfg_prs->key, &node) == 0) {
		eprs_get_str(eprs, node, &key);
	}

	cfg_mode_t mode = CFG_MODE_SET;
	if (eprs_get_rule(eprs, kv, cfg_prs->mode, &node) == 0) {
		eprs_get_str(eprs, node, &tmode);
		strv_t val = lex_get_tok_val(eprs->lex, tmode);
		if (strv_eq(val, STRV("+"))) {
			mode = CFG_MODE_ADD;
		} else if (strv_eq(val, STRV("-"))) {
			mode = CFG_MODE_SUB;
		} else if (strv_eq(val, STRV("?"))) {
			mode = CFG_MODE_ENS;
		}
	}

	eprs_node_t prs_val;
	eprs_get_rule(eprs, kv, cfg_prs->val, &prs_val);
	return cfg_parse_value(cfg_prs, eprs, lex_get_tok_val(eprs->lex, key), mode, prs_val, cfg, var);
}

static int cfg_parse_value(const cfg_prs_t *cfg_prs, eprs_t *eprs, strv_t key, cfg_mode_t mode, eprs_node_t value, cfg_t *cfg,
			   cfg_var_t *var)
{
	int ret = 1;
	eprs_node_t node;
	if (eprs_get_rule(eprs, value, cfg_prs->i, &node) == 0) {
		tok_t val = {0};
		eprs_get_str(eprs, node, &val);

		strv_t val_str = lex_get_tok_val(eprs->lex, val);
		int val_int;
		strv_to_int(val_str, &val_int);
		ret = cfg_int(cfg, key, mode, val_int, var);
	} else if (eprs_get_rule(eprs, value, cfg_prs->str, &node) == 0) {
		tok_t val = {0};
		eprs_get_str(eprs, node, &val);
		ret = cfg_str(cfg, key, mode, lex_get_tok_val(eprs->lex, val), var);
	} else if (eprs_get_rule(eprs, value, cfg_prs->lit, &node) == 0) {
		tok_t val = {0};
		eprs_get_str(eprs, node, &val);
		ret = cfg_lit(cfg, key, mode, lex_get_tok_val(eprs->lex, val), var);
	} else if (eprs_get_rule(eprs, value, cfg_prs->arr, &node) == 0) {
		eprs_node_t child;
		void *data;
		ret = cfg_arr(cfg, key, mode, 0, var);
		eprs_node_foreach(&eprs->nodes, node, child, data)
		{
			eprs_node_t val;
			if (eprs_get_rule(eprs, child, cfg_prs->val, &val)) {
				continue;
			}

			cfg_var_t el;
			ret |= cfg_parse_value(cfg_prs, eprs, STRV_NULL, CFG_MODE_UNKNOWN, val, cfg, &el);
			ret |= cfg_add_var(cfg, *var, el);
		}
	} else if (eprs_get_rule(eprs, value, cfg_prs->obj, &node) == 0) {
		eprs_node_t child;
		void *data;
		ret = cfg_obj(cfg, key, var);
		eprs_node_foreach(&eprs->nodes, node, child, data)
		{
			eprs_node_t kv;
			if (eprs_get_rule(eprs, child, cfg_prs->kv, &kv)) {
				continue;
			}

			cfg_var_t el;
			ret |= cfg_parse_kv(cfg_prs, eprs, kv, cfg, &el);
			ret |= cfg_add_var(cfg, *var, el);
		}
	}

	return ret;
}

static cfg_var_t cfg_parse_tv(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t kv, cfg_t *cfg, cfg_var_t *var)
{
	eprs_node_t node;
	tok_t key = {0};

	if (eprs_get_rule(eprs, kv, cfg_prs->kv, &node) == 0) {
		return cfg_parse_kv(cfg_prs, eprs, node, cfg, var);
	}

	int ret = 0;

	if (eprs_get_rule(eprs, kv, cfg_prs->key, &node) == 0) {
		ret |= eprs_get_str(eprs, node, &key);
	}

	if (eprs_get_rule(eprs, kv, cfg_prs->val, &node) == 0) {
		ret |= cfg_parse_value(cfg_prs, eprs, lex_get_tok_val(eprs->lex, key), CFG_MODE_UNKNOWN, node, cfg, var);
	} else if (eprs_get_rule(eprs, kv, cfg_prs->vals, &node) == 0) {
		eprs_node_t child;
		void *data;
		ret = cfg_arr(cfg, lex_get_tok_val(eprs->lex, key), CFG_MODE_ADD, 1, var);
		eprs_node_foreach(&eprs->nodes, node, child, data)
		{
			eprs_node_t val;
			if (eprs_get_rule(eprs, child, cfg_prs->val, &val)) {
				continue;
			}

			cfg_var_t el;
			ret |= cfg_parse_value(cfg_prs, eprs, STRV_NULL, CFG_MODE_UNKNOWN, val, cfg, &el);
			ret |= cfg_add_var(cfg, *var, el);
		}
	}

	return ret;
}

static int cfg_parse_ent(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t ent, cfg_var_t parent, cfg_t *cfg);

static int cfg_parse_tbl(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t kv, cfg_t *cfg, cfg_var_t *var)
{
	eprs_node_t prs_name;
	eprs_get_rule(eprs, kv, cfg_prs->name, &prs_name);

	tok_t name = {0};
	eprs_get_str(eprs, prs_name, &name);

	cfg_tbl(cfg, lex_get_tok_val(eprs->lex, name), var);

	eprs_node_t prs_ent;
	eprs_get_rule(eprs, kv, cfg_prs->ent, &prs_ent);

	return cfg_parse_ent(cfg_prs, eprs, prs_ent, *var, cfg);
}

static int cfg_parse_ent(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t ent, cfg_var_t parent, cfg_t *cfg)
{
	eprs_node_t child;
	void *data;
	eprs_node_foreach(&eprs->nodes, ent, child, data)
	{
		eprs_node_t prs_kv;
		if (eprs_get_rule(eprs, child, cfg_prs->tv, &prs_kv) == 0) {
			cfg_var_t var;
			cfg_parse_tv(cfg_prs, eprs, prs_kv, cfg, &var);
			cfg_add_var(cfg, parent, var);
			continue;
		}

		eprs_node_t prs_tbl;
		if (eprs_get_rule(eprs, child, cfg_prs->tbl, &prs_tbl) == 0) {
			cfg_var_t var;
			cfg_parse_tbl(cfg_prs, eprs, prs_tbl, cfg, &var);
			cfg_add_var(cfg, parent, var);
			continue;
		}
	}

	return 0;
}

static int cfg_parse_file(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t file, cfg_t *cfg, cfg_var_t *var)
{
	cfg_root(cfg, var);

	eprs_node_t prs_cfg;
	eprs_get_rule(eprs, file, cfg_prs->cfg, &prs_cfg);

	return cfg_parse_ent(cfg_prs, eprs, prs_cfg, *var, cfg);
}

int cfg_prs_parse(const cfg_prs_t *cfg_prs, strv_t str, cfg_t *cfg, alloc_t alloc, cfg_var_t *root, dst_t dst)
{
	if (cfg_prs == NULL || cfg == NULL || str.data == NULL) {
		return 1;
	}

	lex_t lex = {0};
	if (lex_init(&lex, 0, 100, alloc) == NULL) {
		log_error("cparse", "bnf", NULL, "failed to intialize lexer");
		return 1;
	}

	strv_t sstr = STRVN(str.data, str.len);
	lex_tokenize(&lex, sstr, STRV(__FILE__), __LINE__ - 1);

	eprs_t eprs = {0};
	eprs_init(&eprs, 100, alloc);

	eprs_node_t prs_root;
	if (eprs_parse(&eprs, &lex, &cfg_prs->estx, cfg_prs->file, &prs_root, dst)) {
		eprs_free(&eprs);
		lex_free(&lex);
		return 1;
	}

	cfg_var_t tmp;
	int ret = cfg_parse_file(cfg_prs, &eprs, prs_root, cfg, &tmp);

	if (root) {
		*root = tmp;
	}

	eprs_free(&eprs);
	lex_free(&lex);

	return ret;
}
