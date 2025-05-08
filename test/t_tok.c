#include "tok.h"

#include "test.h"

TEST(tok_type_print)
{
	START;

	char buf[32] = {0};

	EXPECT_EQ(tok_type_print((1 << TOK_ALPHA) | (1 << TOK_UPPER), DST_BUF(buf)), 11);

	EXPECT_STR(buf, "ALPHA|UPPER");

	END;
}

TEST(tok_type_enum)
{
	START;

	EXPECT_EQ(tok_type_enum(STRV("ALPHA")), TOK_ALPHA);
	EXPECT_EQ(tok_type_enum(STRV("")), TOK_UNKNOWN);

	END;
}

STEST(tok)
{
	SSTART;

	RUN(tok_type_print);
	RUN(tok_type_enum);

	SEND;
}
