#include "token.h"

#include "test.h"

TEST(token_type_print)
{
	START;

	char buf[32] = {0};

	EXPECT_EQ(token_type_print((1 << TOKEN_ALPHA) | (1 << TOKEN_UPPER), DST_BUF(buf)), 11);

	EXPECT_STR(buf, "ALPHA|UPPER");

	END;
}

TEST(token_type_enum)
{
	START;

	EXPECT_EQ(token_type_enum(STRV("ALPHA")), TOKEN_ALPHA);
	EXPECT_EQ(token_type_enum(STRV("")), TOKEN_UNKNOWN);

	END;
}

STEST(token)
{
	SSTART;

	RUN(token_type_print);
	RUN(token_type_enum);

	SEND;
}
