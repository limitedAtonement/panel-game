#pragma once
#include "logging.hpp"
#include "utils.hpp"
#include "stack.hpp"
#include <array>
#include <iostream>
#include <algorithm>
#include <optional>

std::optional<std::array<spot, 3>> find_three_blocks_vertical(stack const &, spot const & start_spot);
std::optional<std::array<spot, 3>> find_three_blocks_horizontal(stack const &, spot const & start_spot);
// Only looks at the start_spot row.
bool can_get_there(stack const &, const spot & whence, const spot & whither);
// Finds the best combo with horizontal component on the stack
std::vector<plan> find_vert_hor_combos(stack const &);
// Given some spots in a vertical and horizontal combination, returns a plan to execute it
std::optional<plan> execute_vert_hor_combination(stack const & st, std::vector<spot> const & spots, int hor_row);
int increment_column_diff(int old_column_diff);
// Returns a vector of flips that can be added to a plan to move a panel from one spot to another
std::vector<spot> move_panel(spot const & whence, spot const & whither);

// Given a row in which we want to move a panel, come up with a plan
// to make sure the desired panel can move without making a combination.
std::optional<plan> clear_way(stack const & st, int row, int start_col, int end_col, int color);

// Returns true if the last block in the vertical combination should be added from the left.
//  - - x - -
//  - x - x x
//  - - x - -
//  - - x - -
std::optional<bool> finish_hor_vert_combo_left(stack const & st, std::vector<spot> const & spots, int hor_row);

// Returns flips to move source blocks to target blocks
// This isn't trivial because moving one block may mess up another block.
// Assumes blocks are equal (may move source panel 2 to target panel 1, etc.)
std::vector<spot> move_horizontal_blocks(std::vector<spot> const & source_spots,
            std::vector<spot> const & target_spots);

// Creates a plan to move the horizontal source blocks together
std::optional<plan> bring_together_horizontal(std::array<spot, 3> const &);

// Returns the blocks in the vertical combinations available
std::vector<std::vector<spot>> find_vertical_combinations(stack const &);

// Creates a plan to move the starting blocks to the target blocks
template<typename collection_t>
std::optional<plan> create_plan(collection_t const & starting_blocks, collection_t const & target_blocks);

// Returns the column where all the blocks can go
template<typename collection_t>
std::optional<unsigned> can_get_to_same_column(stack const & st, collection_t const & spots);

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

template<typename collection_t>
std::optional<plan> create_plan(collection_t const & raw_starting_blocks, collection_t const & target_blocks)
{
    if (raw_starting_blocks.size() != target_blocks.size())
    {
        LOG("ERROR: create_plan: starting blocks and target blocks have different sizes");
        return {};
    }
    std::vector<std::tuple<unsigned, spot>> sorted_starting_blocks;
    for (unsigned i{0}; i < raw_starting_blocks.size(); ++i)
    {
        sorted_starting_blocks[i] = {i, raw_starting_blocks[i]};
    }
    std::sort(sorted_starting_blocks.begin(), sorted_starting_blocks.end(), [](auto const & a, auto const & b)
        { return std::get<1>(a).row < std::get<1>(b).row; });
    // Each row has a vector of blocks on that row sorted by column
    // The unsigned is the index in the original collection
    std::vector<std::vector<std::tuple<unsigned, spot>>> grouped_starting_blocks;
    for (auto const & block : sorted_starting_blocks)
    {
        if (grouped_starting_blocks.empty())
        {
            grouped_starting_blocks.push_back({block});
            continue;
        }
        if (std::get<1>(grouped_starting_blocks.back().back()).row == std::get<1>(block).row)
        {
            grouped_starting_blocks.back().push_back(block);
            continue;
        }
        grouped_starting_blocks.push_back({block});
    }
    for (auto & group : grouped_starting_blocks)
    {
        std::sort(group.begin(), group.end(), [](auto const & a, auto const & b)
        {
            return std::get<1>(a).col < std::get<1>(b).col;
        });
    }
    plan ret;
    for (std::vector<std::tuple<unsigned, spot>> const & group : grouped_starting_blocks)
    {
        if (group.size() > 1)
        {
            std::vector<spot> source_spots;
            std::vector<spot> target_spots;
            for (unsigned i{0}; i < group.size(); ++i)
            {
                target_spots.push_back(target_blocks[std::get<0>(group[i])]);
                source_spots.push_back(std::get<1>(group[i]));
            }
            std::vector<spot> flips{move_horizontal_blocks(source_spots, target_spots)};
            ret.spot_flips.insert(ret.spot_flips.end(), flips.begin(), flips.end());
        }
        else if (group.size() == 0)
        {
            LOG("create_plan: empty group, what are we doing here?");
            continue;
        }
        else // group.size() == 1
        {
            spot const & target_spot {target_blocks[std::get<0>(group[0])]};
            if (target_spot == std::get<1>(group[0]))
                continue;
            if (group.size() == 1)
            {
                auto flips{move_panel(std::get<1>(group[0]), target_spot)};
                ret.spot_flips.insert(ret.spot_flips.end(), flips.begin(), flips.end());
                continue;
            }
        }
    }
    return ret;
}
