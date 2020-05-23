#include <gtest/gtest.h>

#include "linked_list.h"

#include <string>

using ::testing::InitGoogleTest;

TEST(LinkedListTest, Instantiate) {
    LinkedList<int> l;
    EXPECT_EQ(l.toString(), std::string(""));
}

TEST(LinkedListTest, AppendValue) {
    LinkedList<int> l;
    l.appendValue(3);
    EXPECT_EQ(l.toString(), std::string("3 "));
    l.appendValue(4);
    EXPECT_EQ(l.toString(), std::string("3 4 "));
    l.appendValue(5);
    EXPECT_EQ(l.toString(), std::string("3 4 5 "));
}

int main(int argc, char **argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}