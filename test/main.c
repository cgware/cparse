#include "log.h"
#include "mem.h"
#include "test.h"

STEST(bnf);
STEST(cfg);
STEST(cfg_parse);
STEST(ebnf);
STEST(eparser);
STEST(esyntax);
STEST(lexer);
STEST(make);
STEST(parser);
STEST(syntax);
STEST(token);

TEST(cparse)
{
	SSTART;
	RUN(bnf);
	RUN(cfg);
	RUN(cfg_parse);
	RUN(ebnf);
	RUN(eparser);
	RUN(esyntax);
	RUN(lexer);
	RUN(make);
	RUN(parser);
	RUN(syntax);
	RUN(token);
	SEND;
}

int main()
{
	c_print_init();

	log_t log = {0};
	log_set(&log);
	log_add_callback(log_std_cb, DST_STD(), LOG_WARN, 1, 1);

	t_init();

	t_run(test_cparse, 1);

	int ret = t_finish();

	mem_print(DST_STD());

	if (mem_check()) {
		ret = 1;
	}

	return ret;
}
