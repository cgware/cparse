#include "token.h"

#include "cstr.h"
#include "test.h"

TEST(token_type_str)
{
	START;

	token_type_str(TOKEN_ALPHA);

	END;
}

TEST(token_type_enum)
{
	START;

	EXPECT_EQ(token_type_enum(STR("ALPHA")), TOKEN_ALPHA);
	EXPECT_EQ(token_type_enum(STR("")), TOKEN_UNKNOWN);

	END;
}

STEST(token)
{
	SSTART;

	RUN(token_type_str);
	RUN(token_type_enum);

	SEND;
}
