#include "functions.h"
#include <gtest/gtest.h>

TEST(FUNCTIONS, TestPascalization)
{
    EXPECT_EQ(Pascalization("evSu"), "EvSu");
    EXPECT_EQ(Pascalization("boost_asio"), "BoostAsio");
    EXPECT_EQ(Pascalization("a_b_c"), "ABC");
    EXPECT_EQ(Pascalization("a-b_c"), "ABC");
    EXPECT_EQ(Pascalization("a_b-c"), "ABC");
    EXPECT_EQ(Pascalization("xsp_opt_api"), "XspOptApi");
    EXPECT_EQ(Pascalization("c--"), "C");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
