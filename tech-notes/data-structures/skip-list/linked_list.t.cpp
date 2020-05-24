#include <gtest/gtest.h>

#include "linked_list.h"

#include <string>

using ::testing::InitGoogleTest;

TEST(LinkedListTest, Instantiate) {
    LinkedList<int> l;
    EXPECT_EQ(l.toString(), std::string(""));
    EXPECT_TRUE(l.empty());
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

TEST(LinkedListTest, Iterator) {
    LinkedList<int> l;

    l.appendValue(3);
    EXPECT_FALSE(l.empty());
    
    auto lit = l.begin();
    EXPECT_EQ(*lit, 3);
    EXPECT_FALSE(lit.hasNext());
    EXPECT_FALSE(lit.hasPrev());

    l.appendValue(4);
    EXPECT_TRUE(lit.hasNext());
    EXPECT_FALSE(lit.hasPrev());
    lit.next();
    EXPECT_EQ(*lit, 4);
    EXPECT_FALSE(lit.hasNext());
    EXPECT_TRUE(lit.hasPrev());

    auto lit2 = l.appendValue(5);
    EXPECT_EQ(*lit2, 5);
    EXPECT_FALSE(lit2.hasNext());
    EXPECT_TRUE(lit2.hasPrev());
    lit2.prev();
    EXPECT_EQ(lit, lit2);
}

int main(int argc, char **argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}