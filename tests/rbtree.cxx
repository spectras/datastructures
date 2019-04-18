/* Copyright 2019 Julien Hartmann
 */
#include "rbtree.h"

#include "allocators.h"
#include <gtest/gtest.h>
#include <string>

using datastructure::RBTree;

TEST(RBTree, requirements)
{
    using type = RBTree<int, std::string>;

    // Container requirements
    static_assert(std::is_same_v<type::key_type, int>);
    static_assert(std::is_same_v<type::mapped_type, std::string>);
    static_assert(std::is_same_v<type::value_type, std::pair<const int, std::string>>);
    static_assert(std::is_same_v<type::key_compare, std::less<int>>);
    static_assert(std::is_same_v<type::allocator_type, std::allocator<std::string>>);
    static_assert(std::is_same_v<type::reference, std::string &>);
    static_assert(std::is_same_v<type::const_reference, std::string const &>);
    static_assert(std::is_same_v<typename type::difference_type, std::ptrdiff_t>);
    static_assert(std::is_same_v<typename type::size_type, std::size_t>);

    static_assert(std::is_default_constructible_v<type>);
    static_assert(std::is_copy_constructible_v<type>);
    static_assert(std::is_move_constructible_v<type>);
    static_assert(std::is_copy_assignable_v<type>);
    static_assert(std::is_move_assignable_v<type>);
    static_assert(std::is_destructible_v<type>);

    SUCCEED();
}

TEST(RBTree, basic_lifecycle)
{
    auto tree = RBTree<int, std::string>();
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);

    tree[0] = "value";
    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.at(0), "value");
    EXPECT_EQ(tree[0], "value");

    tree.clear();
    EXPECT_TRUE(tree.empty());
    EXPECT_THROW(tree.at(0), std::out_of_range);
}

TEST(RBTree, basic_operations)
{
    auto tree = RBTree<int, std::string>({
        {1, "1"},
        {2, "2"},
        {3, "3"},
        {4, "4"},
        {5, "5"}
    });
    EXPECT_EQ(tree.size(), 5);

    for (int i = 1; i <= 5; ++i) { EXPECT_EQ(tree.at(i), std::to_string(i)); }

    {
        int i = 1;
        for (auto & value : tree) {
            EXPECT_EQ(value.first, i);
            EXPECT_EQ(value.second, std::to_string(i));
            i += 1;
        }
    }

    for (int i = 1; i <= 5; i += 2) { tree.erase(i); }
    EXPECT_EQ(tree.size(), 2);
}

TEST(RBTree, basic_comparator)
{
    auto tree = RBTree<int, std::string, std::greater<int>>({
        {1, "1"},
        {2, "2"},
        {3, "3"},
        {4, "4"},
        {5, "5"}
    });
    EXPECT_EQ(tree.size(), 5);

    for (int i = 1; i <= 5; ++i) { EXPECT_EQ(tree.at(i), std::to_string(i)); }

    {
        int i = 5;
        for (auto & value : tree) {
            EXPECT_EQ(value.first, i);
            EXPECT_EQ(value.second, std::to_string(i));
            i -= 1;
        }
    }

    for (int i = 1; i <= 5; i += 2) { tree.erase(i); }
    EXPECT_EQ(tree.size(), 2);
}

TEST(RBTree, basic_allocator)
{
    using Tree = RBTree<int, std::string, std::less<int>, testing::counting_allocator<std::string>>;
    unsigned allocations = 0, deallocations = 0;
    {
        auto tree = Tree(testing::counting_allocator<std::string>(&allocations, &deallocations));
        tree[1] = "hello1";
        tree[2] = "hello2";
        tree[3] = "hello3";
        tree[4] = "hello4";
        tree[1] = "replaced";
        EXPECT_EQ(tree.size(), 4);
        EXPECT_EQ(allocations, 5);

        auto treeB = tree;              // propagate on copy construction
        EXPECT_EQ(tree, treeB);
        EXPECT_EQ(allocations, 10);

        auto treeC = std::move(tree);   // propagate on move construction
        EXPECT_EQ(tree.size(), 0);
        EXPECT_EQ(treeB, treeC);
        EXPECT_EQ(allocations, 10);
    }
    EXPECT_EQ(deallocations, 10);
}
