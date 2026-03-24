#include <gtest/gtest.h>
#include "utils.hpp"
#include "algorithms.hpp"
#include "stack.hpp"
#include "mock_stack.hpp"
#include "test_utils.hpp"

TEST(Mock_stack, can_make_move)
{
    mock_stack mock{get_full_unique_stack()};
    EXPECT_TRUE(mock.make_move({1,0}));
}

TEST(Mock_stack, cant_move_dimmed_spots)
{
    mock_stack mock{get_full_unique_stack()};
    EXPECT_FALSE(mock.make_move({0,0}));
}

TEST(Mock_stack, can_move_into_chasm)
{
    stack st{get_stack({"- - -   - -",
                        "- - -   - -",
                        "- - -   - -",
                        "- - - - - -",
                        "- - - - - -",
                        "- - - - - -",
                        "- - - - - -"})};
    stack st_copy{st};
    mock_stack mock{std::move(st_copy)};
    EXPECT_TRUE(mock.make_move({5,2}));
    st_copy = st;
    mock = std::move(st_copy);
    EXPECT_TRUE(mock.make_move({6,2}));
    st_copy = st;
    mock = std::move(st_copy);
    EXPECT_TRUE(mock.make_move({7,2}));
    st_copy = st;
    mock = std::move(st_copy);
    EXPECT_TRUE(mock.make_move({8,2}));
}

TEST(Mock_stack, simplest_combination)
{
    stack st{get_stack({"- - x - - -",
                        "- x - - - -",
                        "- - x - - -"})};
    mock_stack mock{std::move(st)};
    EXPECT_EQ(mock.get_total_combinations(), 0);
    EXPECT_TRUE(mock.make_move({2,1}));
    mock.step();
    EXPECT_EQ(mock.get_total_combinations(), 1);
    EXPECT_EQ(mock.get_combinations().at(0).size(), 3);
}

TEST(Mock_stack, horizontal_combination)
{
    stack st{get_stack({"- x x - x -"})};
    mock_stack mock{std::move(st)};
    EXPECT_EQ(mock.get_total_combinations(), 0);
    EXPECT_TRUE(mock.make_move({1,3}));
    mock.step();
    EXPECT_EQ(mock.get_total_combinations(), 1);
    EXPECT_EQ(mock.get_combinations().at(0).size(), 3);
}

TEST(Mock_stack, combination_one)
{
    stack st{get_stack({"- - - x - -",
                        "- - - x - -",
                        "- x x - x -"})};
    mock_stack mock{std::move(st)};
    EXPECT_EQ(mock.get_total_combinations(), 0);
    EXPECT_TRUE(mock.make_move({1,3}));
    mock.step();
    EXPECT_EQ(mock.get_total_combinations(), 1);
    EXPECT_EQ(mock.get_combinations().at(0).size(), 5);
    bool got_1{false};
    bool got_2{false};
    bool got_3{false};
    bool got_4{false};
    bool got_5{false};
    for (spot const & s : mock.get_combinations().at(0))
    {
        if (s == spot{1,1})
            got_1=true;
        else if (s == spot{1,2})
            got_2=true;
        else if (s == spot{1,3})
            got_3=true;
        else if (s == spot{2,3})
            got_4=true;
        else if (s == spot{3,3})
            got_5=true;
    }
    EXPECT_TRUE(got_1 && got_2 && got_3 && got_4 && got_5);
}

TEST(Mock_stack, tall_vertical)
{
    stack st{get_stack({"- - - x - -",
                        "- - - x - -",
                        "- - - - x -",
                        "- - - x - -",
                        "- - - x - -"})};
    mock_stack mock{std::move(st)};
    EXPECT_EQ(mock.get_total_combinations(), 0);
    EXPECT_TRUE(mock.make_move({3,3}));
    mock.step();
    EXPECT_EQ(mock.get_total_combinations(), 1);
    EXPECT_EQ(mock.get_combinations().at(0).size(), 5);
}

TEST(Mock_stack, combination_two)
{
    // I guess it just fell into place
    stack st{get_stack({"- - x - - -",
                        "- x x x",
                        "- - x -"})};
    mock_stack mock{std::move(st)};
    EXPECT_EQ(mock.get_total_combinations(), 0);
    mock.step();
    EXPECT_EQ(mock.get_total_combinations(), 1);
    EXPECT_EQ(mock.get_combinations().at(0).size(), 5);
}

TEST(Mock_stack, double_horizontal_combination)
{
    stack st{get_stack({"o o x o x x"})};
    mock_stack mock{std::move(st)};
    EXPECT_EQ(mock.get_total_combinations(), 0);
    EXPECT_TRUE(mock.make_move({1,2}));
    mock.step();
    EXPECT_EQ(mock.get_total_combinations(), 2);
    EXPECT_EQ(mock.get_combinations().at(0).size(), 3);
    EXPECT_EQ(mock.get_combinations().at(1).size(), 3);
}

TEST(Mock_stack, double_vertical_combination)
{
    stack st{get_stack({"- - o x - -",
                        "- - o x - -",
                        "- - x o - -",
                        "- - o x - -",
                        "- - o x - -",
    })};
    mock_stack mock{std::move(st)};
    EXPECT_EQ(mock.get_total_combinations(), 0);
    EXPECT_TRUE(mock.make_move({3,2}));
    mock.step();
    EXPECT_EQ(mock.get_total_combinations(), 2);
    EXPECT_EQ(mock.get_combinations().at(0).size(), 5);
    EXPECT_EQ(mock.get_combinations().at(1).size(), 5);
}

