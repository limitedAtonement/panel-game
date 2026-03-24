#include <gtest/gtest.h>
#include "utils.hpp"
#include "algorithms.hpp"
#include "stack.hpp"

unsigned const rows {12};
stack get_stack(void)
{
    unsigned const cols {6};
    stack st(rows, cols);
    unsigned const fill_rows{rows-2};
    // Create a swirl
    for (unsigned row{0}; row < fill_rows; ++row)
    {
        for (unsigned col{0}; col < cols; ++col)
        {
            st.set_panel(row,col, {static_cast<int>((row+col)%6+1), row == 0 ? "dimmed" : "normal"});
        }
    }
    return st;
}

// Returns a stack with 60 unique colors: [10,70].
// Make patterns with numbers below 10 to be sure they are unique.
stack get_full_unique_stack(void)
{
    unsigned const cols {6};
    stack st(rows, cols);
    unsigned const fill_rows{rows-2};
    int color{10};
    for (unsigned row{0}; row < fill_rows; ++row)
    {
        for (unsigned col{0}; col < cols; ++col)
        {
            st.set_panel(row,col, {color++, row == 0 ? "dimmed" : "normal"});
        }
    }
    return st;
}

TEST(Algorithms, can_get_there_normal)
{
    stack st {get_stack()};
    // Create a chasm...
    for (unsigned row{5}; row < rows; ++row)
    {
        st.set_panel(row, 3, {0, "normal"});
    }
    EXPECT_TRUE(can_get_there(st, {1, 0}, {1,5})) << "Can pass a block here";
    EXPECT_TRUE(can_get_there(st, {5, 0}, {5,5})) << "Can pass a block through the chasm";
    EXPECT_TRUE(can_get_there(st, {5, 0}, {1,5})) << "Should ignore end spot row";
    EXPECT_TRUE(!can_get_there(st, {6, 0}, {6,5})) << "Block falls in the chasm";
}

TEST(Algorithms, can_get_there_bases)
{
    stack st {get_stack()};
    // Create a chasm...
    for (unsigned row{5}; row < rows; ++row)
    {
        st.set_panel(row, 3, {0, "normal"});
    }
    st.set_panel(5, 3, {1, "popped"});
    EXPECT_TRUE(!can_get_there(st, {6, 0}, {6,5})) << "Popped blocks are the same as empty blocks";
    st.set_panel(5, 3, {1, "popping"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "Popping blocks can be used as a base";
    st.set_panel(5, 3, {1, "hovering"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "Hovering blocks can be used as a base (I think)";
    st.set_panel(5, 3, {1, "matched"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "matched blocks can be used as a base";
    st.set_panel(5, 3, {1, "swapping"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "swapping blocks can be used as a base";
    st.set_panel(5, 3, {1, "falling"});
    EXPECT_TRUE(!can_get_there(st, {6, 0}, {6,5})) << "Falling blocks aren't a good base for sliding";
    st.set_panel(5, 3, {1, "landing"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "Landing blocks are fine base";
    st.set_panel(5, 3, {1, "dimmed"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "dimmed blocks are fine base";
}

TEST(Algorithms, can_get_there_pass_through)
{
    stack st {get_stack()};
    st.set_panel(6, 3, {1, "popped"});
    EXPECT_TRUE(!can_get_there(st, {6, 0}, {6,5})) << "Can't pass through popped blocks";
    st.set_panel(6, 3, {1, "popping"});
    EXPECT_TRUE(!can_get_there(st, {6, 0}, {6,5})) << "Can't pass through popping blocks";
    st.set_panel(6, 3, {1, "hovering"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "Can pass through hovering blocks";
    st.set_panel(6, 3, {1, "matched"});
    EXPECT_TRUE(!can_get_there(st, {6, 0}, {6,5})) << "Can't pass through matched blocks";
    st.set_panel(6, 3, {1, "swapping"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "Maybe can't swap through swapping, but it should be gone soon";
    st.set_panel(6, 3, {1, "falling"});
    EXPECT_TRUE(!can_get_there(st, {6, 0}, {6,5})) << "Can pass through falling blocks, but don't count on it";
    st.set_panel(6, 3, {1, "landing"});
    EXPECT_TRUE(can_get_there(st, {6, 0}, {6,5})) << "Can't pass through landing, but it should be good soon enough";
    st.set_panel(6, 3, {1, "dimmed"});
    EXPECT_TRUE(!can_get_there(st, {6, 0}, {6,5})) << "dimmed blocks are locked";
}

TEST(Algorithms, can_get_there_chasm)
{
    stack st {get_stack()};
    // Create a chasm...
    for (unsigned row{5}; row < rows; ++row)
    {
        st.set_panel(row, 3, {0, "normal"});
    }
    st.set_panel(3, 5, {1, "normal"});
    st.set_panel(4, 4, {1, "normal"});
    st.set_panel(4, 5, {3, "normal"});
    st.set_panel(5, 5, {3, "normal"});
    EXPECT_FALSE(can_get_there(st, {6,2}, {6, 3})) << "Can't move over chasm";
}

TEST(Algorithms, find_three_blocks_vertical_ignore_dimmed)
{
    unsigned const cols {6};
    stack st(rows, cols);
    // In reverse order
    std::array<std::array<panel, 6>, 4> rows {{
        {{{4, "normal"}, {4, "normal"}, {1, "normal"}, {4, "normal"}, {4, "normal"}, {4, "normal"}}},
        {{{4, "normal"}, {1, "normal"}, {5, "normal"}, {4, "normal"}, {4, "normal"}, {4, "normal"}}},
        {{{1, "normal"}, {2, "normal"}, {3, "normal"}, {2, "normal"}, {3, "normal"}, {5, "normal"}}},
        {{{1, "dimmed"}, {2, "dimmed"}, {3, "dimmed"}, {2, "dimmed"}, {3, "dimmed"}, {4, "dimmed"}}},
    }};
    for (unsigned row{0}; row < rows.size(); ++row)
    {
        for (unsigned col{0}; col < rows[0].size(); ++col)
        {
            st.set_panel(rows.size()-row-1,col, std::move(rows[row][col]));
        }
    }
    std::optional<std::array<spot, 3>> spots {find_three_blocks_vertical(st, {1,0})};
    ASSERT_TRUE(spots);
    EXPECT_EQ((*spots)[0], spot(1,0));
    EXPECT_EQ((*spots)[1], spot(2,1));
    EXPECT_EQ((*spots)[2], spot(3,2));
}

TEST(Algorithms, find_three_blocks_vertical_chasm)
{
    unsigned const cols {6};
    stack st(rows, cols);
    // In reverse order
    std::array<std::array<panel, 6>, 4> rows {{
        {{{4, "normal"}, {4, "normal"}, {0, "normal"}, {1, "normal"}, {4, "normal"}, {4, "normal"}}},
        {{{4, "normal"}, {1, "normal"}, {0, "normal"}, {4, "normal"}, {4, "normal"}, {4, "normal"}}},
        {{{1, "normal"}, {2, "normal"}, {0, "normal"}, {2, "normal"}, {3, "normal"}, {5, "normal"}}},
        {{{1, "dimmed"}, {2, "dimmed"}, {3, "dimmed"}, {2, "dimmed"}, {3, "dimmed"}, {4, "dimmed"}}},
    }};
    for (unsigned row{0}; row < rows.size(); ++row)
    {
        for (unsigned col{0}; col < rows[0].size(); ++col)
        {
            st.set_panel(rows.size()-row-1,col, std::move(rows[row][col]));
        }
    }
    std::optional<std::array<spot, 3>> spots {find_three_blocks_vertical(st, {1,0})};
    EXPECT_FALSE(spots) << "Can't make three blocks across a chasm";
}

TEST(Algorithms, can_get_to_same_column_simple)
{
    stack st {get_stack()};
    std::optional<unsigned> col {can_get_to_same_column(st, std::array<spot, 3>{{{1,5}, {2,4}, {3,3}}})};
    ASSERT_TRUE(col);
    EXPECT_EQ(*col, 4);
}

TEST(Algorithms, can_get_to_same_column_chasm_right)
{
    stack st {get_stack()};
    // Create a chasm...
    for (unsigned row{5}; row < rows; ++row)
    {
        st.set_panel(row, 3, {0, "normal"});
    }
    // The average of these blocks is around column 3, but the top one can't get there because of the chasm.
    std::optional<unsigned> col {can_get_to_same_column(st, std::array<spot, 3>{{{4,0}, {5,5}, {6,4}}})};
    ASSERT_TRUE(col);
    EXPECT_EQ(*col, 4);
}

TEST(Algorithms, can_get_to_same_column_chasm_left)
{
    stack st {get_stack()};
    // Create a chasm...
    for (unsigned row{5}; row < rows; ++row)
    {
        st.set_panel(row, 3, {0, "normal"});
    }
    st.set_panel(3, 5, {1, "normal"});
    st.set_panel(4, 4, {1, "normal"});
    st.set_panel(4, 5, {3, "normal"});
    st.set_panel(5, 5, {3, "normal"});
    // The average of these blocks is around column 4 or 5, but the top one has to stay left of the chasm
    std::optional<unsigned> col {can_get_to_same_column(st, std::array<spot, 3>{{{4,5}, {5,5}, {6,2}}})};
    ASSERT_TRUE(col);
    EXPECT_EQ(*col, 2);
}

TEST(Algorithms, cant_get_there)
{
    stack st {get_stack()};
    // Create a chasm...
    for (unsigned row{5}; row < rows; ++row)
    {
        st.set_panel(row, 3, {0, "normal"});
    }
    // The average of these blocks is around column 4 or 5, but the top one has to stay left of the chasm
    std::optional<unsigned> col {can_get_to_same_column(st, std::array<spot, 3>{{{6,1}, {7,0}, {8,5}}})};
    EXPECT_FALSE(col);
}

TEST(Algorithms, find_three_blocks_horizontal_simple)
{
    stack st {get_stack()};
    st.set_panel(3, 0, {1, "normal"});
    // (3,3) is already 1
    st.set_panel(3, 5, {1, "normal"});
    std::optional<std::array<spot, 3>> spots {find_three_blocks_horizontal(st, {3,0})};
    ASSERT_TRUE(spots);
    EXPECT_EQ((*spots)[0], spot(3,0));
    EXPECT_EQ((*spots)[1], spot(3,3));
    EXPECT_EQ((*spots)[2], spot(3,5));
}

TEST(Algorithms, find_three_blocks_horizontal_chasm)
{
    stack st {get_stack()};
    // Create a chasm...
    for (unsigned row{5}; row < rows; ++row)
    {
        st.set_panel(row, 3, {0, "normal"});
    }
    st.set_panel(9, 0, {2, "normal"});
    st.set_panel(9, 1, {2, "normal"});
    // The match is at 9,4, but there is a chasm separating
    std::optional<std::array<spot, 3>> spots {find_three_blocks_horizontal(st, {9,1})};
    ASSERT_FALSE(spots);
}

TEST(Algorithms, move_horizontal_blocks_two_blocks_in)
{
    // x--x--
    std::vector<spot> source_spots {{3,0}, {3,3}};
    // -xx---
    std::vector<spot> target_spots {{3,1}, {3,2}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 2);
    bool got_1{false};
    bool got_2{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
            got_1 = true;
        if (flip == spot(3,2))
            got_2 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
}

TEST(Algorithms, move_horizontal_blocks_two_close_blocks_right)
{
    // xx----
    std::vector<spot> source_spots {{3,0}, {3,1}};
    // -xx---
    std::vector<spot> target_spots {{3,1}, {3,2}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 2);
    bool got_1{false};
    bool got_2{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
        {
            EXPECT_TRUE(got_2) << "We must flip the right side first";
            got_1 = true;
        }
        if (flip == spot(3,1))
            got_2 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
}

TEST(Algorithms, move_horizontal_blocks_two_close_blocks_left)
{
    // -xx---
    std::vector<spot> source_spots {{3,1}, {3,2}};
    // xx----
    std::vector<spot> target_spots {{3,0}, {3,1}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 2);
    bool got_1{false};
    bool got_2{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
            got_1 = true;
        if (flip == spot(3,1))
        {
            EXPECT_TRUE(got_1) << "We must flip the left side first";
            got_2 = true;
        }
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
}

TEST(Algorithms, move_horizontal_blocks_two_blocks_out)
{
    // _xx___
    std::vector<spot> source_spots {{3,1}, {3,2}};
    // x--x--
    std::vector<spot> target_spots {{3,0}, {3,3}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 2);
    bool got_1{false};
    bool got_2{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
            got_1 = true;
        if (flip == spot(3,2))
            got_2 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
}

TEST(Algorithms, move_horizontal_blocks_two_blocks_right)
{
    // x _ x _ _ _
    std::vector<spot> source_spots {{3,0}, {3,2}};
    // _ x _ x _ _
    std::vector<spot> target_spots {{3,1}, {3,3}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 2);
    bool got_1{false};
    bool got_2{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
            got_1 = true;
        if (flip == spot(3,2))
            got_2 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
}

TEST(Algorithms, move_horizontal_blocks_two_blocks_left)
{
    // _ x _ x _ _
    std::vector<spot> source_spots {{3,1}, {3,3}};
    // x _ x _ _ _
    std::vector<spot> target_spots {{3,0}, {3,2}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 2);
    bool got_1{false};
    bool got_2{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
            got_1 = true;
        if (flip == spot(3,2))
            got_2 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
}

TEST(Algorithms, move_horizontal_blocks_3_blocks_in)
{
    // x _ _ x _ x
    std::vector<spot> source_spots {{3,0}, {3,3}, {3,5}};
    // _ x x _ x _
    std::vector<spot> target_spots {{3,1}, {3,2}, {3,4}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 3);
    bool got_1{false};
    bool got_2{false};
    bool got_3{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
            got_1 = true;
        if (flip == spot(3,2))
            got_2 = true;
        if (flip == spot(3,4))
            got_3 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
    EXPECT_TRUE(got_3);
}

TEST(Algorithms, move_horizontal_blocks_3_blocks_out)
{
    // _ x _ x x _
    std::vector<spot> source_spots {{3,1}, {3,3}, {3,4}};
    // x _ x _ _ x
    std::vector<spot> target_spots {{3,0}, {3,2}, {3,5}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 3);
    bool got_1{false};
    bool got_2{false};
    bool got_3{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
            got_1 = true;
        if (flip == spot(3,2))
            got_2 = true;
        if (flip == spot(3,4))
            got_3 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
    EXPECT_TRUE(got_3);
}

TEST(Algorithms, move_horizontal_blocks_3_blocks_right)
{
    // x x _ _ x _
    // _ x x _ _ x
    std::vector<spot> source_spots {{3,0}, {3,1}, {3,4}};
    std::vector<spot> target_spots {{3,1}, {3,2}, {3,5}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 3);
    bool got_1{false};
    bool got_2{false};
    bool got_3{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
        {
            EXPECT_TRUE(got_2) << "We must flip the right side before flipping the outside";
            got_1 = true;
        }
        if (flip == spot(3,1))
            got_2 = true;
        if (flip == spot(3,4))
            got_3 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
    EXPECT_TRUE(got_3);
}

TEST(Algorithms, move_horizontal_blocks_3_blocks_left)
{
    // _ x x _ _ x
    // x x _ _ x _
    std::vector<spot> source_spots {{3,1}, {3,2}, {3,5}};
    std::vector<spot> target_spots {{3,0}, {3,1}, {3,4}};
    std::vector<spot> flips {move_horizontal_blocks(source_spots, target_spots)};
    EXPECT_EQ(flips.size(), 3);
    bool got_1{false};
    bool got_2{false};
    bool got_3{false};
    for (auto const & flip : flips)
    {
        if (flip == spot(3,0))
        {
            got_1 = true;
        }
        if (flip == spot(3,1))
        {
            EXPECT_TRUE(got_1) << "We must flip the left side before flipping the inside";
            got_2 = true;
        }
        if (flip == spot(3,4))
            got_3 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
    EXPECT_TRUE(got_3);
}

TEST(Algorithms, DISABLED_clear_way_simple)
{
    stack st {get_full_unique_stack()};

}

TEST(Algorithms, find_vertical_combination_3)
{
    stack st {get_full_unique_stack()};
    // - - x o - -
    // - - - x o -
    // - - - - x o
    st.set_panel(1, 4, {1, "normal"});
    st.set_panel(2, 3, {1, "normal"});
    st.set_panel(3, 2, {1, "normal"});
    st.set_panel(1, 5, {2, "normal"});
    st.set_panel(2, 4, {2, "normal"});
    st.set_panel(3, 3, {2, "normal"});
    std::vector<std::vector<spot>> const combos {find_vertical_combinations(st)};
    EXPECT_EQ(combos.size(), 2);
    bool got_1{false};
    bool got_2{false};
    const std::function<bool(spot const &, std::vector<spot> const &)> has_spot =
                [](spot const & sp, std::vector<spot> const & vertical) {
        return std::find(vertical.begin(), vertical.end(), sp) != vertical.end();
    };
    for (std::vector<spot> const & vertical : combos)
    {
        if (has_spot({1,4}, vertical) && has_spot({2,3}, vertical) && has_spot({3,2}, vertical))
            got_1 = true;
        else if (has_spot({1,5}, vertical) && has_spot({2,4}, vertical) && has_spot({3,3}, vertical))
            got_2 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
}

TEST(Algorithms, find_vertical_combination_4)
{
    stack st {get_full_unique_stack()};
    // x o - - - -
    // - - x o - -
    // - - - x o -
    // - - - - x o
    // - - - - - -
    // - - - - - -
    st.set_panel(2, 4, {1, "normal"});
    st.set_panel(3, 3, {1, "normal"});
    st.set_panel(4, 2, {1, "normal"});
    st.set_panel(5, 0, {1, "normal"});
    st.set_panel(2, 5, {2, "normal"});
    st.set_panel(3, 4, {2, "normal"});
    st.set_panel(4, 3, {2, "normal"});
    st.set_panel(5, 1, {2, "normal"});
    std::vector<std::vector<spot>> const combos {find_vertical_combinations(st)};
    EXPECT_EQ(combos.size(), 2) << "After deduplication, we should only have the two 4-combos";
    bool got_1{false};
    bool got_2{false};
    const std::function<bool(spot const &, std::vector<spot> const &)> has_spot =
                [](spot const & sp, std::vector<spot> const & vertical) {
        return std::find(vertical.begin(), vertical.end(), sp) != vertical.end();
    };
    for (std::vector<spot> const & vertical : combos)
    {
        if (vertical.size() == 3)
            continue;
        if (has_spot({2,4}, vertical) && has_spot({3,3}, vertical) &&
                    has_spot({4,2}, vertical) && has_spot({5,0}, vertical))
            got_1 = true;
        else if (has_spot({2,5}, vertical) && has_spot({3,4}, vertical) &&
                        has_spot({4,3}, vertical) && has_spot({5,1}, vertical))
            got_2 = true;
    }
    EXPECT_TRUE(got_1);
    EXPECT_TRUE(got_2);
}

TEST(Algorithms, find_vertical_combination_chasm)
{
    stack st {get_full_unique_stack()};
    // x -   - - -
    // - x   - - -
    // - -   x - -
    // - -   - x -
    // - - - - - -
    st.set_panel(1, 4, {1, "normal"});
    st.set_panel(2, 3, {1, "normal"});
    st.set_panel(3, 1, {1, "normal"});
    st.set_panel(4, 1, {1, "normal"});
    st.set_panel(1, 2, {0, "normal"});
    st.set_panel(2, 2, {0, "normal"});
    st.set_panel(3, 2, {0, "normal"});
    st.set_panel(4, 2, {0, "normal"});
    std::vector<std::vector<spot>> const combos {find_vertical_combinations(st)};
    EXPECT_EQ(combos.size(), 0) << "No combinations should be found because of the chasm";
}

TEST(Algorithms, find_simple_combo)
{
    stack st {get_full_unique_stack()};
    // - - x - - -
    // - - - x - -
    // x x - - x -
    // - - - - - -
    st.set_panel(1, 0, {1, "normal"});
    st.set_panel(1, 1, {1, "normal"});
    st.set_panel(1, 4, {1, "normal"});
    st.set_panel(2, 3, {1, "normal"});
    st.set_panel(3, 2, {1, "normal"});
    std::vector<plan> const combos {find_vert_hor_combos(st)};
    EXPECT_EQ(combos.size(), 1);
    plan const & combo_plan{combos.front()};
    EXPECT_EQ(combo_plan.spot_flips.back(), spot(1,1)) << "Last move should be 1,1";
}

TEST(Algorithms, find_middle_combo)
{
    stack st {get_full_unique_stack()};
    // - - x - - -
    // x x - x - -
    // - - - - x -
    // - - - - - -
    st.set_panel(1, 4, {1, "normal"});
    st.set_panel(2, 0, {1, "normal"});
    st.set_panel(2, 1, {1, "normal"});
    st.set_panel(2, 3, {1, "normal"});
    st.set_panel(3, 2, {1, "normal"});
    std::vector<plan> const combos {find_vert_hor_combos(st)};
    EXPECT_EQ(combos.size(), 1);
    plan const & combo_plan{combos.front()};
    EXPECT_EQ(combo_plan.spot_flips.back(), spot(2,2));
}

TEST(Algorithms, find_middle_combo_conflict)
{
    stack st {get_full_unique_stack()};
    // - - x - - -
    // x - x x - -
    // - - - - x -
    // - - - - - -
    st.set_panel(1, 4, {1, "normal"});
    st.set_panel(2, 0, {1, "normal"});
    st.set_panel(2, 1, {1, "normal"});
    st.set_panel(2, 3, {1, "normal"});
    st.set_panel(3, 2, {1, "normal"});
    std::vector<plan> const combos {find_vert_hor_combos(st)};
    EXPECT_EQ(combos.size(), 1);
    plan const & combo_plan{combos.front()};
    EXPECT_EQ(combo_plan.spot_flips.back(), spot(2,2));
}

TEST(Algorithms, DISABLED_find_simple_combo_chasm)
{
    stack st {get_full_unique_stack()};
    // - - x   - -
    // - - -   x -
    // x x -   x -
    // - - -   - -
    st.set_panel(1, 0, {1, "normal"});
    st.set_panel(1, 1, {1, "normal"});
    st.set_panel(1, 4, {1, "normal"});
    st.set_panel(2, 4, {1, "normal"});
    st.set_panel(3, 2, {1, "normal"});
    st.set_panel(0, 3, {0, "normal"});
    st.set_panel(1, 3, {0, "normal"});
    st.set_panel(2, 3, {0, "normal"});
    st.set_panel(3, 3, {0, "normal"});
    std::vector<plan> const combos {find_vert_hor_combos(st)};
    EXPECT_EQ(combos.size(), 0) << "Can't make combination across chasm";
}

TEST(Algorithms, clear_way_middle)
{
    stack st {get_full_unique_stack()};
    // - - x - - -
    // - x - - - -
    // - - x - - -
    // - - - - - -
    st.set_panel(1, 2, {1, "normal"});
    st.set_panel(2, 1, {1, "normal"});
    st.set_panel(3, 2, {1, "normal"});
    std::optional<plan> plan {clear_way(st, 2, 1, 3, 1)};
    EXPECT_TRUE(plan);
    EXPECT_GT(plan->spot_flips.size(), 0);
}

TEST(Algorithms, clear_way_middle_harder)
{
    stack st {get_full_unique_stack()};
    // - - x - - -
    // - x - - - -
    // - - x x - -
    // - - - - - -
    st.set_panel(1, 2, {1, "normal"});
    st.set_panel(1, 3, {1, "normal"});
    st.set_panel(2, 1, {1, "normal"});
    st.set_panel(3, 2, {1, "normal"});
    std::optional<plan> plan {clear_way(st, 2, 1, 3, 1)};
    EXPECT_TRUE(plan);
    EXPECT_GT(plan->spot_flips.size(), 0);
    EXPECT_NE(plan->spot_flips.front(), spot(2, 2)) << "Can't flip at (2,2) because that creates another barrier";
}

// If one of the blocks can't be matched (dimmed), we don't need to clear the way.
TEST(Algorithms, clear_way_middle_no_match)
{
    stack st {get_full_unique_stack()};
    // - - x - - -
    // - x - - - -
    // - - x x - -  dimmed
    st.set_panel(0, 2, {1, "dimmed"});
    st.set_panel(0, 3, {1, "dimmed"});
    st.set_panel(1, 1, {1, "normal"});
    st.set_panel(2, 2, {1, "normal"});
    std::optional<plan> plan {clear_way(st, 1, 1, 3, 1)};
    EXPECT_FALSE(plan) << "No need to clear anything because blocks can't match anyway";
}

