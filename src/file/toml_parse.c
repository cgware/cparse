#include "file/toml_parse.h"

#include "ebnf.h"
#include "eparser.h"
#include "log.h"
#include "str.h"

#include <stdlib.h>

toml_prs_t *toml_prs_init(toml_prs_t *toml_prs, alloc_t alloc)
{
	if (toml_prs == NULL) {
		return NULL;
	}

	uint line      = __LINE__ + 1;
	str_t toml_bnf = STR("file = toml EOF\n"
			     "toml = (kv NL)* (tbl | tbla)? (NL tbl | NL tbla)*\n"
			     "kv   = key ' = ' val\n"
			     "key  = (ALPHA | '.')+\n"
			     "val  = int | \"'\" strl \"'\" | '[' arr? ']' | '{' inl? '}'\n"
			     "int  = DIGIT+\n"
			     "strl = c*\n"
			     "arr  = val (', ' val)*\n"
			     "inl  = kv (', ' kv)*\n"
			     "c    = ALPHA | DIGIT | SYMBOL | ' '\n"
			     "tbl  = '[' key ']' NL ent\n"
			     "tbla = '[[' key ']]' NL ent\n"
			     "ent  = (kv NL | tbl NL | tbla NL)*\n");

	lex_t lex = {0};
	if (lex_init(&lex, 0, 100, alloc) == NULL) {
		log_error("cparse", "bnf", NULL, "failed to intialize lexer");
		return NULL;
	}

	lex_tokenize(&lex, &toml_bnf, STR(__FILE__), line);

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

	estx_init(&toml_prs->estx, 10, 10, ALLOC_STD);
	estx_rule_t estx_root = estx_from_ebnf(&ebnf, &prs, prs_root, &toml_prs->estx, &names);
	(void)estx_root;

	strbuf_get_index(&names, STRV("file"), &toml_prs->file);
	strbuf_get_index(&names, STRV("toml"), &toml_prs->toml);
	strbuf_get_index(&names, STRV("kv"), &toml_prs->kv);
	strbuf_get_index(&names, STRV("key"), &toml_prs->key);
	strbuf_get_index(&names, STRV("val"), &toml_prs->val);
	strbuf_get_index(&names, STRV("int"), &toml_prs->i);
	strbuf_get_index(&names, STRV("strl"), &toml_prs->strl);
	strbuf_get_index(&names, STRV("arr"), &toml_prs->arr);
	strbuf_get_index(&names, STRV("inl"), &toml_prs->inl);
	strbuf_get_index(&names, STRV("tbl"), &toml_prs->tbl);
	strbuf_get_index(&names, STRV("tbla"), &toml_prs->tbla);
	strbuf_get_index(&names, STRV("ent"), &toml_prs->ent);

	strbuf_free(&names);
	prs_free(&prs);
	lex_free(&lex);

	return toml_prs;
}

void toml_prs_free(toml_prs_t *toml_prs)
{
	estx_free(&toml_prs->estx);
}

static toml_var_t toml_parse_kv(const toml_prs_t *toml_prs, eprs_t *eprs, eprs_node_t kv, toml_t *toml);

static toml_var_t toml_parse_value(const toml_prs_t *toml_prs, eprs_t *eprs, strv_t key, eprs_node_t value, toml_t *toml)
{
	eprs_node_t node;
	toml_var_t ret = TOML_VAL_END;
	if ((node = eprs_get_rule(eprs, value, toml_prs->i)) < eprs->nodes.cnt) {
		token_t val = {0};
		eprs_get_str(eprs, node, &val);

		strv_t val_str = lex_get_token_val(eprs->lex, val);
		int val_int    = strtol(val_str.data, NULL, 10);

		ret = TOML_INT(toml, key, val_int);
	} else if ((node = eprs_get_rule(eprs, value, toml_prs->strl)) < eprs->nodes.cnt) {
		token_t val = {0};
		eprs_get_str(eprs, node, &val);

		ret = TOML_STRL(toml, key, lex_get_token_val(eprs->lex, val));
	} else if ((node = eprs_get_rule(eprs, value, toml_prs->arr)) < eprs->nodes.cnt) {
		eprs_node_t child;
		ret = TOML_ARR(toml, key);
		eprs_node_foreach(&eprs->nodes, node, child)
		{
			eprs_node_t val = eprs_get_rule(eprs, child, toml_prs->val);
			if (val >= eprs->nodes.cnt) {
				continue;
			}

			toml_add_var(toml, ret, toml_parse_value(toml_prs, eprs, STRV_NULL, val, toml));
		}
	} else if ((node = eprs_get_rule(eprs, value, toml_prs->inl)) < eprs->nodes.cnt) {
		eprs_node_t child;
		ret = TOML_INL(toml, key);
		eprs_node_foreach(&eprs->nodes, node, child)
		{
			eprs_node_t kv = eprs_get_rule(eprs, child, toml_prs->kv);
			if (kv >= eprs->nodes.cnt) {
				continue;
			}

			toml_add_var(toml, ret, toml_parse_kv(toml_prs, eprs, kv, toml));
		}
	}

	return ret;
}

static toml_var_t toml_parse_kv(const toml_prs_t *toml_prs, eprs_t *eprs, eprs_node_t kv, toml_t *toml)
{
	eprs_node_t prs_key = eprs_get_rule(eprs, kv, toml_prs->key);

	token_t key = {0};
	eprs_get_str(eprs, prs_key, &key);

	eprs_node_t prs_val = eprs_get_rule(eprs, kv, toml_prs->val);
	toml_var_t s	    = toml_parse_value(toml_prs, eprs, lex_get_token_val(eprs->lex, key), prs_val, toml);
	return s;
}

void toml_parse_ent(const toml_prs_t *toml_prs, eprs_t *eprs, eprs_node_t ent, toml_var_t parent, toml_t *toml);

toml_var_t toml_parse_tbl(const toml_prs_t *toml_prs, eprs_t *eprs, eprs_node_t kv, toml_t *toml)
{
	eprs_node_t prs_key = eprs_get_rule(eprs, kv, toml_prs->key);

	token_t key = {0};
	eprs_get_str(eprs, prs_key, &key);

	toml_var_t tbl = TOML_TBL(toml, lex_get_token_val(eprs->lex, key));

	eprs_node_t prs_ent = eprs_get_rule(eprs, kv, toml_prs->ent);

	toml_parse_ent(toml_prs, eprs, prs_ent, tbl, toml);

	return tbl;
}

toml_var_t toml_parse_tbla(const toml_prs_t *toml_prs, eprs_t *eprs, eprs_node_t kv, toml_t *toml)
{
	eprs_node_t prs_key = eprs_get_rule(eprs, kv, toml_prs->key);

	token_t key = {0};
	eprs_get_str(eprs, prs_key, &key);

	toml_var_t tbl = TOML_TBL_ARR(toml, lex_get_token_val(eprs->lex, key));

	eprs_node_t prs_ent = eprs_get_rule(eprs, kv, toml_prs->ent);

	toml_parse_ent(toml_prs, eprs, prs_ent, tbl, toml);

	return tbl;
}

void toml_parse_ent(const toml_prs_t *toml_prs, eprs_t *eprs, eprs_node_t ent, toml_var_t parent, toml_t *toml)
{
	eprs_node_t child;
	eprs_node_foreach(&eprs->nodes, ent, child)
	{
		eprs_node_t prs_kv = eprs_get_rule(eprs, child, toml_prs->kv);
		if (prs_kv < eprs->nodes.cnt) {
			toml_add_var(toml, parent, toml_parse_kv(toml_prs, eprs, prs_kv, toml));
			continue;
		}

		eprs_node_t prs_tbl = eprs_get_rule(eprs, child, toml_prs->tbl);
		if (prs_tbl < eprs->nodes.cnt) {
			toml_add_var(toml, parent, toml_parse_tbl(toml_prs, eprs, prs_tbl, toml));
			continue;
		}

		eprs_node_t prs_tbla = eprs_get_rule(eprs, child, toml_prs->tbla);
		if (prs_tbla < eprs->nodes.cnt) {
			toml_add_var(toml, parent, toml_parse_tbla(toml_prs, eprs, prs_tbla, toml));
			continue;
		}
	}
}

toml_var_t toml_parse_file(const toml_prs_t *toml_prs, eprs_t *eprs, eprs_node_t file, toml_t *toml)
{
	toml_var_t root = TOML_ROOT(toml);

	eprs_node_t prs_toml = eprs_get_rule(eprs, file, toml_prs->toml);

	toml_parse_ent(toml_prs, eprs, prs_toml, root, toml);

	return root;
}

toml_var_t toml_prs_parse(const toml_prs_t *toml_prs, strv_t str, toml_t *toml, alloc_t alloc, print_dst_t dst)
{
	if (toml_prs == NULL || toml == NULL || str.data == NULL) {
		return TOML_VAL_END;
	}

	lex_t lex = {0};
	if (lex_init(&lex, 0, 100, alloc) == NULL) {
		log_error("cparse", "bnf", NULL, "failed to intialize lexer");
		return TOML_VAL_END;
	}

	str_t sstr = strc(str.data, str.len);
	lex_tokenize(&lex, &sstr, STR(__FILE__), __LINE__ - 1);

	eprs_t eprs = {0};
	eprs_init(&eprs, 100, alloc);

	eprs_node_t prs_root;
	if (eprs_parse(&eprs, &lex, &toml_prs->estx, toml_prs->file, &prs_root, dst)) {
		eprs_free(&eprs);
		lex_free(&lex);
		return TOML_VAL_END;
	}

	toml_var_t root = toml_parse_file(toml_prs, &eprs, prs_root, toml);

	eprs_free(&eprs);
	lex_free(&lex);

	return root;
}
