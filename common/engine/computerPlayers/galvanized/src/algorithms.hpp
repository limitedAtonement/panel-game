#pragma once
#include <optional>
#include "utils.hpp"
#include "stack.hpp"
#include <array>
#include <iostream>

std::optional<std::array<spot, 3>> find_three_blocks_vertical(stack const &, spot const & start_spot);
std::optional<std::array<spot, 3>> find_three_blocks_horizontal(stack const &, spot const & start_spot);
// Only looks at the start_spot row.
bool can_get_there(stack const &, const spot & whence, const spot & whither);

int increment_column_diff(int old_column_diff);

// Returns the column where all thre blocks can go
template<typename collection_t>
std::optional<unsigned> can_get_to_same_column(stack const & st, collection_t const & spots)
{
    if (spots.empty())
        return {};
    unsigned preferred_col{0};
    unsigned count{0};
    for (auto const & spot : spots)
    {
        preferred_col += spot.col;
        ++count;
    }
    preferred_col /= count;
    //std::cout << "Preferred col: " << preferred_col << '\n';
    // 0, 1, -1, 2, -2, etc.
    for (int col_diff{0}; col_diff < st.width(); col_diff = increment_column_diff(col_diff))
    {
        int test_col{static_cast<int>(preferred_col) + col_diff};
        //std::cout << "Checking col diff: " << col_diff << " ( column " << test_col << ")\n";
        if (test_col < 0 || test_col >= st.width())
        {
            //std::cout << "   col " << test_col << " out of bounds\n";
            continue;
        }
        bool can_get{true};
        for (auto const & spot : spots)
        {
            //std::cout << "   checking spot: " << spot.to_string() << " can get to desired column...\n";
            if (!can_get_there(st, spot, {spot.row, test_col}))
            {
                //std::cout << "   can't get there\n";
                can_get = false;
                break;
            }
        }
        if (can_get)
        {
            //std::cout << "    match\n!";
            return test_col;
        }
    }
    //std::cout << "Done searchnig\n";
    return {};
}
