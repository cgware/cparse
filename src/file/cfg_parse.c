#include "file/cfg_parse.h"

#include "ebnf.h"
#include "eparser.h"
#include "log.h"
#include "str.h"

#include <stdlib.h>

cfg_prs_t *cfg_prs_init(cfg_prs_t *cfg_prs, alloc_t alloc)
{
	if (cfg_prs == NULL) {
		return NULL;
	}

	uint line      = __LINE__ + 1;
	strv_t cfg_bnf = STRV("file = cfg EOF\n"
			      "cfg  = (kv NL)* tbl? (NL tbl)*\n"
			      "kv   = key ' = ' val\n"
			      "key  = (ALPHA | '.')+\n"
			      "val  = int | '\"' str '\"' | lit | '[' arr? ']' | '{' obj? '}'\n"
			      "int  = DIGIT+\n"
			      "str  = c*\n"
			      "lit  = (ALPHA | DIGIT | '_')+\n"
			      "arr  = val (', ' val)*\n"
			      "obj  = kv (', ' kv)*\n"
			      "c    = ALPHA | DIGIT | SYMBOL | ' '\n"
			      "tbl  = '[' key ']' NL ent\n"
			      "ent  = (kv NL)*\n");

	lex_t lex = {0};
	if (lex_init(&lex, 0, 100, alloc) == NULL) {
		log_error("cparse", "bnf", NULL, "failed to intialize lexer");
		return NULL;
	}

	lex_tokenize(&lex, cfg_bnf, STRV(__FILE__), line);

	ebnf_t ebnf = {0};
	ebnf_init(&ebnf, alloc);
	ebnf_get_stx(&ebnf, alloc, PRINT_DST_NONE());

	prs_t prs = {0};
	prs_init(&prs, 100, ALLOC_STD);

	prs_node_t prs_root;
	prs_parse(&prs, &lex, &ebnf.stx, ebnf.file, &prs_root, PRINT_DST_NONE());

	ebnf_free(&ebnf);

	strbuf_t names = {0};
	strbuf_init(&names, 16, 16, ALLOC_STD);

	estx_init(&cfg_prs->estx, 10, 10, ALLOC_STD);
	estx_rule_t estx_root = estx_from_ebnf(&ebnf, &prs, prs_root, &cfg_prs->estx, &names);
	(void)estx_root;

	strbuf_find(&names, STRV("file"), &cfg_prs->file);
	strbuf_find(&names, STRV("cfg"), &cfg_prs->cfg);
	strbuf_find(&names, STRV("kv"), &cfg_prs->kv);
	strbuf_find(&names, STRV("key"), &cfg_prs->key);
	strbuf_find(&names, STRV("val"), &cfg_prs->val);
	strbuf_find(&names, STRV("int"), &cfg_prs->i);
	strbuf_find(&names, STRV("str"), &cfg_prs->str);
	strbuf_find(&names, STRV("lit"), &cfg_prs->lit);
	strbuf_find(&names, STRV("arr"), &cfg_prs->arr);
	strbuf_find(&names, STRV("obj"), &cfg_prs->obj);
	strbuf_find(&names, STRV("tbl"), &cfg_prs->tbl);
	strbuf_find(&names, STRV("ent"), &cfg_prs->ent);

	strbuf_free(&names);
	prs_free(&prs);
	lex_free(&lex);

	return cfg_prs;
}

void cfg_prs_free(cfg_prs_t *cfg_prs)
{
	estx_free(&cfg_prs->estx);
}

static cfg_var_t cfg_parse_kv(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t kv, cfg_t *cfg);

static cfg_var_t cfg_parse_value(const cfg_prs_t *cfg_prs, eprs_t *eprs, strv_t key, eprs_node_t value, cfg_t *cfg)
{
	eprs_node_t node;
	cfg_var_t ret = CFG_VAR_END;
	if ((node = eprs_get_rule(eprs, value, cfg_prs->i)) < eprs->nodes.cnt) {
		token_t val = {0};
		eprs_get_str(eprs, node, &val);

		strv_t val_str = lex_get_token_val(eprs->lex, val);
		int val_int    = strtol(val_str.data, NULL, 10);

		ret = CFG_INT(cfg, key, val_int);
	} else if ((node = eprs_get_rule(eprs, value, cfg_prs->str)) < eprs->nodes.cnt) {
		token_t val = {0};
		eprs_get_str(eprs, node, &val);

		ret = CFG_STR(cfg, key, lex_get_token_val(eprs->lex, val));
	} else if ((node = eprs_get_rule(eprs, value, cfg_prs->lit)) < eprs->nodes.cnt) {
		token_t val = {0};
		eprs_get_str(eprs, node, &val);

		ret = CFG_LIT(cfg, key, lex_get_token_val(eprs->lex, val));
	} else if ((node = eprs_get_rule(eprs, value, cfg_prs->arr)) < eprs->nodes.cnt) {
		eprs_node_t child;
		ret = CFG_ARR(cfg, key);
		eprs_node_foreach(&eprs->nodes, node, child)
		{
			eprs_node_t val = eprs_get_rule(eprs, child, cfg_prs->val);
			if (val >= eprs->nodes.cnt) {
				continue;
			}

			cfg_add_var(cfg, ret, cfg_parse_value(cfg_prs, eprs, STRV_NULL, val, cfg));
		}
	} else if ((node = eprs_get_rule(eprs, value, cfg_prs->obj)) < eprs->nodes.cnt) {
		eprs_node_t child;
		ret = CFG_OBJ(cfg, key);
		eprs_node_foreach(&eprs->nodes, node, child)
		{
			eprs_node_t kv = eprs_get_rule(eprs, child, cfg_prs->kv);
			if (kv >= eprs->nodes.cnt) {
				continue;
			}

			cfg_add_var(cfg, ret, cfg_parse_kv(cfg_prs, eprs, kv, cfg));
		}
	}

	return ret;
}

static cfg_var_t cfg_parse_kv(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t kv, cfg_t *cfg)
{
	eprs_node_t prs_key = eprs_get_rule(eprs, kv, cfg_prs->key);

	token_t key = {0};
	eprs_get_str(eprs, prs_key, &key);

	eprs_node_t prs_val = eprs_get_rule(eprs, kv, cfg_prs->val);
	cfg_var_t s	    = cfg_parse_value(cfg_prs, eprs, lex_get_token_val(eprs->lex, key), prs_val, cfg);
	return s;
}

void cfg_parse_ent(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t ent, cfg_var_t parent, cfg_t *cfg);

cfg_var_t cfg_parse_tbl(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t kv, cfg_t *cfg)
{
	eprs_node_t prs_key = eprs_get_rule(eprs, kv, cfg_prs->key);

	token_t key = {0};
	eprs_get_str(eprs, prs_key, &key);

	cfg_var_t tbl = CFG_TBL(cfg, lex_get_token_val(eprs->lex, key));

	eprs_node_t prs_ent = eprs_get_rule(eprs, kv, cfg_prs->ent);

	cfg_parse_ent(cfg_prs, eprs, prs_ent, tbl, cfg);

	return tbl;
}

void cfg_parse_ent(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t ent, cfg_var_t parent, cfg_t *cfg)
{
	eprs_node_t child;
	eprs_node_foreach(&eprs->nodes, ent, child)
	{
		eprs_node_t prs_kv = eprs_get_rule(eprs, child, cfg_prs->kv);
		if (prs_kv < eprs->nodes.cnt) {
			cfg_add_var(cfg, parent, cfg_parse_kv(cfg_prs, eprs, prs_kv, cfg));
			continue;
		}

		eprs_node_t prs_tbl = eprs_get_rule(eprs, child, cfg_prs->tbl);
		if (prs_tbl < eprs->nodes.cnt) {
			cfg_add_var(cfg, parent, cfg_parse_tbl(cfg_prs, eprs, prs_tbl, cfg));
			continue;
		}
	}
}

cfg_var_t cfg_parse_file(const cfg_prs_t *cfg_prs, eprs_t *eprs, eprs_node_t file, cfg_t *cfg)
{
	cfg_var_t root = CFG_ROOT(cfg);

	eprs_node_t prs_cfg = eprs_get_rule(eprs, file, cfg_prs->cfg);

	cfg_parse_ent(cfg_prs, eprs, prs_cfg, root, cfg);

	return root;
}

cfg_var_t cfg_prs_parse(const cfg_prs_t *cfg_prs, strv_t str, cfg_t *cfg, alloc_t alloc, print_dst_t dst)
{
	if (cfg_prs == NULL || cfg == NULL || str.data == NULL) {
		return CFG_VAR_END;
	}

	lex_t lex = {0};
	if (lex_init(&lex, 0, 100, alloc) == NULL) {
		log_error("cparse", "bnf", NULL, "failed to intialize lexer");
		return CFG_VAR_END;
	}

	strv_t sstr = STRVN(str.data, str.len);
	lex_tokenize(&lex, sstr, STRV(__FILE__), __LINE__ - 1);

	eprs_t eprs = {0};
	eprs_init(&eprs, 100, alloc);

	eprs_node_t prs_root;
	if (eprs_parse(&eprs, &lex, &cfg_prs->estx, cfg_prs->file, &prs_root, dst)) {
		eprs_free(&eprs);
		lex_free(&lex);
		return CFG_VAR_END;
	}

	cfg_var_t root = cfg_parse_file(cfg_prs, &eprs, prs_root, cfg);

	eprs_free(&eprs);
	lex_free(&lex);

	return root;
}
