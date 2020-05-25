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

    EXPECT_EQ(3, l.size());
}

TEST(LinkedListTest, AppendValueIterator) {
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

TEST(LinkedListTest, InsertAfterIterator) {
    LinkedList<int> l;

    auto it = l.appendValue(3);
    auto eit = l.insertAfter(5, it);

    EXPECT_EQ(it, l.begin());
    EXPECT_EQ(eit, l.last());

    auto eit2 = l.insertAfter(4, it);
    EXPECT_EQ(it, l.begin());
    EXPECT_EQ(eit, l.last());

    auto bit = l.begin();
    EXPECT_EQ(*bit, 3);
    bit.next();
    EXPECT_EQ(*bit, 4);
    bit.next();
    EXPECT_EQ(*bit, 5);
    EXPECT_EQ(bit, l.last());
}

TEST(LinkedListTest, InsertValueOrderedIterator) {
    LinkedList<int> l;

    l.insertValueOrdered(5);
    l.insertValueOrdered(3);
    l.insertValueOrdered(4);
    l.insertValueOrdered(1);
    l.insertValueOrdered(6);
    l.insertValueOrdered(3);

    auto bit = l.begin();
    EXPECT_EQ(*bit, 1);
    bit.next();
    EXPECT_EQ(*bit, 3);
    bit.next();
    EXPECT_EQ(*bit, 3);
    bit.next();
    EXPECT_EQ(*bit, 4);
    bit.next();
    EXPECT_EQ(*bit, 5);
    bit.next();
    EXPECT_EQ(*bit, 6);
}

TEST(LinkedListTest, RemoveAt) {
    LinkedList<int> l;

    l.insertValueOrdered(5);
    l.insertValueOrdered(3);
    l.insertValueOrdered(4);
    l.insertValueOrdered(1);
    l.insertValueOrdered(6);
    l.insertValueOrdered(3);

    auto bit = l.begin();
    l.removeAt(bit);

    auto eit = l.last();
    l.removeAt(eit);

    auto nit = l.begin();
    nit.next();
    nit.next();
    l.removeAt(nit);

    auto oit = l.begin();
    EXPECT_EQ(*oit, 3);
    oit.next();
    EXPECT_EQ(*oit, 3);
    oit.next();
    EXPECT_EQ(*oit, 5);
}

TEST(LinkedListTest, RemoveValue) {
    LinkedList<int> l;

    l.insertValueOrdered(5);
    l.insertValueOrdered(3);
    l.insertValueOrdered(4);
    l.insertValueOrdered(1);
    l.insertValueOrdered(6);
    l.insertValueOrdered(3);

    l.removeValue(3);

    auto bit = l.begin();
    EXPECT_EQ(*bit, 1);
    bit.next();
    EXPECT_EQ(*bit, 4);
    bit.next();
    EXPECT_EQ(*bit, 5);
    bit.next();
    EXPECT_EQ(*bit, 6);
}

int main(int argc, char **argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}