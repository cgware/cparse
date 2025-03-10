#include "file.h"
#include "log.h"
#include "mem.h"
#include "test.h"

STEST(bnf);
STEST(ebnf);
STEST(eparser);
STEST(esyntax);
STEST(lexer);
STEST(make);
STEST(parser);
STEST(syntax);
STEST(token);
STEST(toml);
STEST(toml_parse);

TEST(cparse)
{
	SSTART;
	RUN(bnf);
	RUN(ebnf);
	RUN(eparser);
	RUN(esyntax);
	RUN(lexer);
	RUN(make);
	RUN(parser);
	RUN(syntax);
	RUN(token);
	RUN(toml);
	RUN(toml_parse);
	SEND;
}

int main()
{
	c_print_init();

	log_t log = {0};
	log_set(&log);
	log_add_callback(log_std_cb, PRINT_DST_FILE(stderr), LOG_WARN, 1, 1);

	t_init();

	t_run(test_cparse, 1);

	int ret = t_finish();

	mem_print(PRINT_DST_STD());

	if (mem_check()) {
		ret = 1;
	}

	return ret;
}
