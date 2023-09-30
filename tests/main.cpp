
#include "gtest/gtest.h"

/*
 *TEST(TestSuitName, TestName) {
 *    ASSERT_TRUE(true);
 *    ASSERT_EQ(0, 0);
 *
 *}
 */

#ifdef HAS_VLD_H
#include <vld.h>
#endif

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
