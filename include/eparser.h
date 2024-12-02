#ifndef EPARSER_H
#define EPARSER_H

#include "esyntax.h"
#include "lexer.h"
#include "tree.h"

#define EPRS_NODE_END TREE_END

typedef tnode_t eprs_node_t;

typedef enum eprs_node_type_e {
	EPRS_NODE_UNKNOWN,
	EPRS_NODE_RULE,
	EPRS_NODE_TOKEN,
	EPRS_NODE_LITERAL,
} eprs_node_type_t;

typedef struct eprs_node_data_s {
	eprs_node_type_t type;
	union {
		estx_rule_t rule;
		token_t literal;
		token_t token;
		int alt;
	} val;
} eprs_node_data_t;

typedef struct eprs_s {
	const estx_t *estx;
	const lex_t *lex;
	tree_t nodes;
} eprs_t;

eprs_t *eprs_init(eprs_t *eprs, const lex_t *lex, const estx_t *estx, uint nodes_cap, alloc_t alloc);
void eprs_free(eprs_t *eprs);

eprs_node_t eprs_add(eprs_t *eprs, eprs_node_data_t node);

eprs_node_t eprs_add_node(eprs_t *eprs, eprs_node_t parent, eprs_node_t node);
int eprs_remove_node(eprs_t *eprs, eprs_node_t node);

eprs_node_t eprs_get_rule(const eprs_t *eprs, eprs_node_t parent, estx_rule_t rule);
int eprs_get_str(const eprs_t *eprs, eprs_node_t parent, str_t *out);

eprs_node_t eprs_parse(eprs_t *eprs, estx_rule_t rule, print_dst_t dst);

int eprs_print(const eprs_t *eprs, eprs_node_t node, print_dst_t dst);

#define EPRS_NODE_RULE(_prs, _rule)   eprs_add(_prs, (eprs_node_data_t){.type = EPRS_NODE_RULE, .val.rule = _rule})
#define EPRS_NODE_TOKEN(_prs, _token) eprs_add(_prs, (eprs_node_data_t){.type = EPRS_NODE_TOKEN, .val.token = _token})
#define EPRS_NODE_LITERAL(_prs, _start, _len)                                                                                              \
	eprs_add(_prs, (eprs_node_data_t){.type = EPRS_NODE_LITERAL, .val.literal = {.start = _start, .len = _len}})

#define eprs_node_foreach tree_foreach_child

#endif
