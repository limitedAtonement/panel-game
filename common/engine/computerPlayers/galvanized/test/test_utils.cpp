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
    EXPECT_TRUE(spots);
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
    EXPECT_TRUE(col);
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
    EXPECT_TRUE(col);
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
    EXPECT_TRUE(col);
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
