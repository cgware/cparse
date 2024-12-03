#include "eparser.h"

#include "ebnf.h"
#include "log.h"
#include "mem.h"
#include "test.h"

#include <memory.h>
#include <stdlib.h>

TEST(eprs_init_free)
{
	START;

	eprs_t eprs = {0};

	EXPECT_EQ(eprs_init(NULL, 0, ALLOC_STD), NULL);
	mem_oom(1);
	EXPECT_EQ(eprs_init(&eprs, 1, ALLOC_STD), NULL);
	mem_oom(0);
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_init(&eprs, 0, ALLOC_STD), &eprs);
	log_set_quiet(0, 0);

	EXPECT_NE(eprs.nodes.data, NULL);

	eprs_free(&eprs);
	eprs_free(NULL);

	EXPECT_EQ(eprs.nodes.data, NULL);

	END;
}

TEST(eprs_add_node)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	EXPECT_EQ(eprs_add(NULL, (eprs_node_data_t){.type = EPRS_NODE_UNKNOWN}), EPRS_NODE_END);
	mem_oom(1);
	EXPECT_EQ(eprs_add(&eprs, (eprs_node_data_t){.type = EPRS_NODE_UNKNOWN}), EPRS_NODE_END);
	mem_oom(0);

	eprs_node_t parent = eprs_add(&eprs, (eprs_node_data_t){.type = EPRS_NODE_UNKNOWN});
	eprs_node_t child  = eprs_add(&eprs, (eprs_node_data_t){.type = EPRS_NODE_UNKNOWN});

	EXPECT_EQ(eprs_add_node(NULL, EPRS_NODE_END, EPRS_NODE_END), EPRS_NODE_END);
	EXPECT_EQ(eprs_add_node(&eprs, EPRS_NODE_END, EPRS_NODE_END), EPRS_NODE_END);
	EXPECT_EQ(eprs_add_node(&eprs, parent, EPRS_NODE_END), EPRS_NODE_END);
	EXPECT_EQ(eprs_add_node(&eprs, parent, child), child);

	eprs_free(&eprs);

	END;
}

TEST(eprs_remove_node)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	eprs_node_t node = eprs_add_node(&eprs, EPRS_NODE_END, eprs_add(&eprs, (eprs_node_data_t){.type = EPRS_NODE_UNKNOWN}));

	EXPECT_EQ(eprs_remove_node(NULL, EPRS_NODE_END), 1);
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_remove_node(&eprs, EPRS_NODE_END), 1);
	log_set_quiet(0, 0);
	EXPECT_EQ(eprs_remove_node(&eprs, node), 0);

	eprs_free(&eprs);

	END;
}

TEST(eprs_get_rule)
{
	START;

	eprs_t eprs = {0};
	log_set_quiet(0, 1);
	eprs_init(&eprs, 0, ALLOC_STD);
	log_set_quiet(0, 0);

	eprs_node_t root = eprs_add_node(&eprs, EPRS_NODE_END, EPRS_NODE_LITERAL(&eprs, 0, 0));

	eprs_add_node(&eprs, root, EPRS_NODE_LITERAL(&eprs, 0, 0));
	eprs_node_t node = eprs_add_node(&eprs, root, EPRS_NODE_RULE(&eprs, 0));

	EXPECT_EQ(eprs_get_rule(NULL, EPRS_NODE_END, ESTX_RULE_END), EPRS_NODE_END);
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_get_rule(&eprs, EPRS_NODE_END, ESTX_RULE_END), EPRS_NODE_END);
	log_set_quiet(0, 0);
	EXPECT_EQ(eprs_get_rule(&eprs, root, ESTX_RULE_END), EPRS_NODE_END);
	EXPECT_EQ(eprs_get_rule(&eprs, root, 0), node);
	EXPECT_EQ(eprs_get_rule(&eprs, node, 0), node);

	eprs_free(&eprs);

	END;
}

TEST(eprs_get_str)
{
	START;

	eprs_t eprs = {0};
	eprs_init(&eprs, 1, ALLOC_STD);

	eprs_node_t root = eprs_add_node(&eprs, EPRS_NODE_END, EPRS_NODE_RULE(&eprs, 0));

	eprs_add_node(&eprs, root, EPRS_NODE_LITERAL(&eprs, 0, 0));
	eprs_add_node(&eprs, root, EPRS_NODE_TOKEN(&eprs, (token_t){0}));
	eprs_add_node(&eprs, root, EPRS_NODE_RULE(&eprs, 0));
	eprs_add_node(&eprs, root, eprs_add(&eprs, (eprs_node_data_t){.type = EPRS_NODE_UNKNOWN}));

	str_t str = strz(16);

	EXPECT_EQ(eprs_get_str(NULL, EPRS_NODE_END, NULL), 1);
	EXPECT_EQ(eprs_get_str(&eprs, EPRS_NODE_END, NULL), 1);

	str.len = 0;
	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_get_str(&eprs, root, &str), 0);
	log_set_quiet(0, 0);

	EXPECT_STR(str.data, "");

	str_free(&str);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_gen)
{
	START;

	EXPECT_EQ(eprs_parse(NULL, NULL, NULL, ESTX_RULE_END, NULL, PRINT_DST_NONE()), 1);

	lex_t lex = {0};
	str_t src = STR("<");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 1, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, estx_create_term(&estx, (estx_term_data_t){.type = -1}));

	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 1);
	log_set_quiet(0, 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_rule_invalid)
{
	START;

	lex_t lex = {0};
	str_t src = STR("");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_RULE(&estx, -1, ESTX_TERM_OCC_ONE));

	log_set_quiet(0, 1);
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 1);
	log_set_quiet(0, 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_rule)
{
	START;

	lex_t lex = {0};
	str_t src = STR(" ");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t line = estx_add_rule(&estx);
	estx_rule_set_term(&estx, line, ESTX_TERM_LITERAL(&estx, STR(" "), ESTX_TERM_OCC_ONE));

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_RULE(&estx, line, ESTX_TERM_OCC_ONE));

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_token_unexpected)
{
	START;

	lex_t lex = {0};
	str_t src = STR("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_TOKEN(&estx, TOKEN_ALPHA, ESTX_TERM_OCC_ONE));

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected ALPHA\n"
		   "1\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_token)
{
	START;

	lex_t lex = {0};
	str_t src = STR("A");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_TOKEN(&estx, TOKEN_ALPHA, ESTX_TERM_OCC_ONE));

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_literal_unexpected_end)
{
	START;

	lex_t lex = {0};
	str_t src = STR("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_LITERAL(&estx, STR("123"), ESTX_TERM_OCC_ONE));

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected '123'\n"
		   "1\n"
		   " ^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_literal_unexpected)
{
	START;

	lex_t lex = {0};
	str_t src = STR("13");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_LITERAL(&estx, STR("123"), ESTX_TERM_OCC_ONE));

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected '123'\n"
		   "13\n"
		   " ^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_literal)
{
	START;

	lex_t lex = {0};
	str_t src = STR("1");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_LITERAL(&estx, STR("1"), ESTX_TERM_OCC_ONE));

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_alt_failed)
{
	START;

	lex_t lex = {0};
	str_t src = STR("c");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_term_t alt	 = estx_rule_set_term(&estx, rule, ESTX_TERM_ALT(&estx));
	estx_term_add_term(&estx, alt, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, alt, ESTX_TERM_LITERAL(&estx, STR("b"), ESTX_TERM_OCC_ONE));

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected 'b'\n"
		   "c\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_alt)
{
	START;

	lex_t lex = {0};
	str_t src = STR("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_term_t alt	 = estx_rule_set_term(&estx, rule, ESTX_TERM_ALT(&estx));
	estx_term_add_term(&estx, alt, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, alt, ESTX_TERM_LITERAL(&estx, STR("b"), ESTX_TERM_OCC_ONE));

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_con_failed)
{
	START;

	lex_t lex = {0};
	str_t src = STR("c");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_term_t con	 = estx_rule_set_term(&estx, rule, ESTX_TERM_CON(&estx));
	estx_term_add_term(&estx, con, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, con, ESTX_TERM_LITERAL(&estx, STR("b"), ESTX_TERM_OCC_ONE));

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected 'a'\n"
		   "c\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_con)
{
	START;

	lex_t lex = {0};
	str_t src = STR("ab");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_term_t con	 = estx_rule_set_term(&estx, rule, ESTX_TERM_CON(&estx));
	estx_term_add_term(&estx, con, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, con, ESTX_TERM_LITERAL(&estx, STR("b"), ESTX_TERM_OCC_ONE));

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_group_failed)
{
	START;

	lex_t lex = {0};
	str_t src = STR("c");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule  = estx_add_rule(&estx);
	estx_term_t group = estx_rule_set_term(&estx, rule, ESTX_TERM_GROUP(&estx, ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, group, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, group, ESTX_TERM_LITERAL(&estx, STR("b"), ESTX_TERM_OCC_ONE));

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected 'a'\n"
		   "c\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_group)
{
	START;

	lex_t lex = {0};
	str_t src = STR("ab");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule  = estx_add_rule(&estx);
	estx_term_t group = estx_rule_set_term(&estx, rule, ESTX_TERM_GROUP(&estx, ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, group, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, group, ESTX_TERM_LITERAL(&estx, STR("b"), ESTX_TERM_OCC_ONE));

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_opt)
{
	START;

	lex_t lex = {0};
	str_t src = STR("");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_OPT));

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_rep_failed)
{
	START;

	lex_t lex = {0};
	str_t src = STR("");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_REP));

	char buf[256] = {0};
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	EXPECT_STR(buf + sizeof(__FILE__ ":000:0: ") - 1,
		   "error: expected 'a'\n"
		   "\n"
		   "^\n");

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_rep_loop)
{
	START;

	lex_t lex = {0};
	str_t bnf = STR("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &bnf, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 10, 10, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 100, ALLOC_STD);

	const estx_rule_t file = estx_add_rule(&estx);
	const estx_rule_t line = estx_add_rule(&estx);
	const estx_rule_t val  = estx_add_rule(&estx);

	estx_rule_set_term(&estx, file, ESTX_TERM_RULE(&estx, line, ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP));
	estx_term_add_term(&estx, file, ESTX_TERM_TOKEN(&estx, TOKEN_EOF, ESTX_TERM_OCC_ONE));
	estx_rule_set_term(&estx, line, ESTX_TERM_RULE(&estx, val, ESTX_TERM_OCC_OPT));
	estx_rule_set_term(&estx, val, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));

	log_set_quiet(0, 1);
	eprs_node_t root;
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, file, &root, PRINT_DST_NONE()), 0);
	log_set_quiet(0, 0);
	EXPECT_EQ(root, 0);

	eprs_free(&eprs);
	lex_free(&lex);
	estx_free(&estx);

	END;
}

TEST(eprs_parse_rep)
{
	START;

	lex_t lex = {0};
	str_t src = STR("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &src, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 1, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 256, ALLOC_STD);

	estx_rule_t rule = estx_add_rule(&estx);
	estx_rule_set_term(&estx, rule, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_REP));

	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 0);

	estx_free(&estx);
	lex_free(&lex);
	eprs_free(&eprs);

	END;
}

TEST(eprs_parse_name)
{
	START;

	lex_t lex = {0};
	str_t bnf = STR("b");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &bnf, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 10, 10, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 100, ALLOC_STD);

	const estx_rule_t file = estx_add_rule(&estx);
	const estx_rule_t vala = estx_add_rule(&estx);
	const estx_rule_t valb = estx_add_rule(&estx);

	const estx_term_t file_alt = estx_rule_set_term(&estx, file, ESTX_TERM_ALT(&estx));
	estx_term_add_term(&estx, file_alt, ESTX_TERM_RULE(&estx, vala, ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, file_alt, ESTX_TERM_RULE(&estx, valb, ESTX_TERM_OCC_ONE));

	estx_rule_set_term(&estx, valb, ESTX_TERM_LITERAL(&estx, STR("b"), ESTX_TERM_OCC_ONE));
	estx_rule_set_term(&estx, vala, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));

	eprs_node_t root;
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, file, &root, PRINT_DST_NONE()), 0);

	char buf[64] = {0};
	EXPECT_EQ(eprs_print(&eprs, root, PRINT_DST_BUF(buf, sizeof(buf), 0)), 22);
	EXPECT_STR(buf,
		   "0\n"
		   "└─2\n"
		   "  └─'b'\n");

	eprs_free(&eprs);
	lex_free(&lex);
	estx_free(&estx);

	END;
}

TEST(eprs_parse_cache)
{
	START;

	lex_t lex = {0};
	str_t bnf = STR("a");
	lex_init(&lex, 0, 1, ALLOC_STD);
	lex_tokenize(&lex, &bnf, STR(__FILE__), __LINE__ - 2);

	estx_t estx = {0};
	estx_init(&estx, 10, 10, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 100, ALLOC_STD);

	const estx_rule_t file = estx_add_rule(&estx);
	const estx_rule_t line = estx_add_rule(&estx);
	const estx_rule_t ra   = estx_add_rule(&estx);

	estx_rule_set_term(&estx, file, ESTX_TERM_RULE(&estx, line, ESTX_TERM_OCC_ONE));
	estx_term_add_term(&estx, file, ESTX_TERM_TOKEN(&estx, TOKEN_EOF, ESTX_TERM_OCC_ONE));
	estx_rule_set_term(&estx, line, ESTX_TERM_RULE(&estx, ra, ESTX_TERM_OCC_OPT | ESTX_TERM_OCC_REP));
	estx_rule_set_term(&estx, ra, ESTX_TERM_LITERAL(&estx, STR("a"), ESTX_TERM_OCC_ONE));

	eprs_node_t root;
	EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, file, &root, PRINT_DST_NONE()), 0);

	char buf[64] = {0};
	EXPECT_EQ(eprs_print(&eprs, root, PRINT_DST_BUF(buf, sizeof(buf), 0)), 34);
	EXPECT_STR(buf,
		   "0\n"
		   "└─1\n"
		   "  └─2\n"
		   "    └─'a'\n");

	eprs_free(&eprs);
	lex_free(&lex);
	estx_free(&estx);

	END;
}

TEST(eprs_parse_ebnf)
{
	START;

	lex_t lex = {0};
	lex_init(&lex, 0, 1, ALLOC_STD);

	eprs_t eprs = {0};
	eprs_init(&eprs, 1, ALLOC_STD);

	{
		str_t bnf = STR("<file> ::= <");
		lex_tokenize(&lex, &bnf, STR(__FILE__), __LINE__ - 1);

		estx_t estx = {0};
		estx_init(&estx, 1, 1, ALLOC_STD);

		estx_rule_t rule = estx_add_rule(&estx);
		estx_rule_set_term(&estx, rule, estx_create_term(&estx, (estx_term_data_t){.type = -1}));

		log_set_quiet(0, 1);
		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 1);
		log_set_quiet(0, 0);

		estx_free(&estx);
	}

	{
		str_t bnf = STR("<file> ::= <");
		lex_tokenize(&lex, &bnf, STR(__FILE__), __LINE__ - 1);

		estx_t estx = {0};
		estx_init(&estx, 1, 1, ALLOC_STD);

		estx_rule_t rule = estx_add_rule(&estx);
		EXPECT_EQ(estx_rule_set_term(&estx, rule, ESTX_TERM_RULE(&estx, -1, 0)), 0);

		log_set_quiet(0, 1);
		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 1);
		log_set_quiet(0, 0);

		estx_free(&estx);
	}

	{
		str_t bnf = STR("::");
		lex_tokenize(&lex, &bnf, STR(__FILE__), __LINE__ - 1);

		estx_t estx = {0};
		estx_init(&estx, 1, 1, ALLOC_STD);

		estx_rule_t rule = estx_add_rule(&estx);
		estx_rule_set_term(&estx, rule, ESTX_TERM_LITERAL(&estx, STRH("::="), ESTX_TERM_OCC_ONE));

		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, rule, NULL, PRINT_DST_NONE()), 1);

		estx_free(&estx);
	}

	estx_t estx = {0};
	prs_t prs   = {0};
	estx_rule_t estx_root;

	{
		ebnf_t ebnf = {0};
		ebnf_init(&ebnf, ALLOC_STD);
		ebnf_get_stx(&ebnf, ALLOC_STD, PRINT_DST_NONE());

		uint line  = __LINE__ + 1;
		str_t sbnf = STR("file    = ebnf EOF\n"
				 "ebnf    = rule+\n"
				 "rule    = rname spaces '= ' alt NL\n"
				 "rname   = LOWER (LOWER | '_')*\n"
				 "alt     = concat (' | ' concat)*\n"
				 "concat  = factor (' ' factor)*\n"
				 "factor  = term (opt | rep | opt_rep)?\n"
				 "opt     = '?'\n"
				 "rep     = '+'\n"
				 "opt_rep = '*'\n"
				 "term    = literal | token | rname | group\n"
				 "literal = \"'\" (char | '\"')+ \"'\" | '\"' (char | \"'\")+ '\"'\n"
				 "token   = UPPER+\n"
				 "group   = '(' alt ')'\n"
				 "char    = ALPHA | DIGIT | SYMBOL | ' '\n"
				 "spaces  = ' '+\n");

		lex_tokenize(&lex, &sbnf, STR(__FILE__), line);

		prs_init(&prs, 100, ALLOC_STD);
		prs_node_t prs_root;
		prs_parse(&prs, &lex, &ebnf.stx, ebnf.file, &prs_root, PRINT_DST_NONE());

		strbuf_t names = {0};
		strbuf_init(&names, 16 * sizeof(char), ALLOC_STD);

		estx_init(&estx, 10, 10, ALLOC_STD);
		estx_root = estx_from_ebnf(&ebnf, &prs, prs_root, &estx, &names);

		strbuf_free(&names);
		ebnf_free(&ebnf);
	}

	{
		str_t sbnf = STR("<file> ::= ");
		lex_tokenize(&lex, &sbnf, STR(__FILE__), __LINE__ - 1);

		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, estx_root, NULL, PRINT_DST_NONE()), 1);
	}

	{
		str_t sebnf = STR("");
		lex_tokenize(&lex, &sebnf, STR(__FILE__), __LINE__ - 1);

		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, estx_root, NULL, PRINT_DST_NONE()), 1);
	}

	{
		uint line  = __LINE__ + 1;
		str_t sbnf = STR("file    = ebnf EOF\n"
				 "ebnf    = rule+\n"
				 "rule    = rname spaces '= ' alt NL\n"
				 "rname   = LOWER (LOWER | '_')*\n"
				 "alt     = concat (' | ' concat)*\n"
				 "concat  = factor (' ' factor)*\n"
				 "factor  = term (opt | rep | opt_rep)?\n"
				 "opt     = '?'\n"
				 "rep     = '+'\n"
				 "opt_rep = '*'\n"
				 "term    = literal | token | rname | group\n"
				 "literal = \"'\" (char | '\"')+ \"'\" | '\"' (char | \"'\")+ '\"'\n"
				 "token   = UPPER+\n"
				 "group   = '(' alt ')'\n"
				 "char    = ALPHA | DIGIT | SYMBOL | ' '\n"
				 "spaces  = ' '+\n");

		lex_tokenize(&lex, &sbnf, STR(__FILE__), line);

		eprs_node_t root;
		EXPECT_EQ(eprs_parse(&eprs, &lex, &estx, estx_root, &root, PRINT_DST_NONE()), 0);

		char *buf = malloc(30000);
		EXPECT_EQ(eprs_print(&eprs, root, PRINT_DST_BUF(buf, 30000, 0)), 19851);
		free(buf);
	}

	eprs_free(&eprs);
	estx_free(&estx);
	lex_free(&lex);
	prs_free(&prs);

	END;
}

TEST(eprs_parse)
{
	SSTART;

	RUN(eprs_parse_gen);
	RUN(eprs_parse_rule_invalid);
	RUN(eprs_parse_rule);
	RUN(eprs_parse_token_unexpected);
	RUN(eprs_parse_token);
	RUN(eprs_parse_literal_unexpected_end);
	RUN(eprs_parse_literal_unexpected);
	RUN(eprs_parse_literal);
	RUN(eprs_parse_alt_failed);
	RUN(eprs_parse_alt);
	RUN(eprs_parse_con_failed);
	RUN(eprs_parse_con);
	RUN(eprs_parse_group_failed);
	RUN(eprs_parse_group);
	RUN(eprs_parse_opt);
	RUN(eprs_parse_rep_failed);
	RUN(eprs_parse_rep_loop);
	RUN(eprs_parse_rep);
	RUN(eprs_parse_name);
	RUN(eprs_parse_cache);
	RUN(eprs_parse_ebnf);

	SEND;
}

TEST(eprs_print)
{
	START;

	eprs_t eprs = {0};
	eprs_init(&eprs, 1, ALLOC_STD);

	eprs_node_t root = eprs_add_node(&eprs, EPRS_NODE_END, EPRS_NODE_RULE(&eprs, 0));
	eprs_add_node(&eprs, root, EPRS_NODE_TOKEN(&eprs, (token_t){0}));
	eprs_add_node(&eprs, root, EPRS_NODE_LITERAL(&eprs, 0, 0));
	eprs_add_node(&eprs, root, eprs_add(&eprs, (eprs_node_data_t){.type = EPRS_NODE_UNKNOWN}));

	char buf[64] = {0};
	EXPECT_EQ(eprs_print(NULL, EPRS_NODE_END, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	EXPECT_EQ(eprs_print(&eprs, root, PRINT_DST_BUF(buf, sizeof(buf), 0)), 33);
	EXPECT_STR(buf,
		   "0\n"
		   "├─UNKNOWN()\n"
		   "├─''\n"
		   "└─");

	eprs_free(&eprs);

	END;
}

STEST(eparser)
{
	SSTART;

	RUN(eprs_init_free);
	RUN(eprs_add_node);
	RUN(eprs_remove_node);
	RUN(eprs_get_rule);
	RUN(eprs_get_str);
	RUN(eprs_parse);
	RUN(eprs_print);

	SEND;
}
