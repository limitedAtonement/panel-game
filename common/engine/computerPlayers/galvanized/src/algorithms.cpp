#include "algorithms.hpp"

// Returns the three blocks that can be swapped to make a vertical match.
std::optional<std::array<spot, 3>> find_three_blocks_vertical(stack const & st, spot const & start_spot)
{
    std::optional<panel> const start_panel{st.get_panel(start_spot)};
    if (!start_panel)
    {
        LOG("WARNING No panel at start spot: " + start_spot.to_string());
        return {};
    }
    if (!start_panel->color)
    {
        // Empty spot, do nothing
        return {};
    }
    // Check for all the matching blocks above the start spot.
    std::array<std::vector<spot>, 2> potential_blocks;
    for (int row_diff{1}; row_diff < 3 && start_spot.row + row_diff < st.height(); ++row_diff)
    {
        for (int col{0}; col < st.width(); ++col)
        {
            spot const current_spot {start_spot.row+row_diff, col};
            std::optional<panel> potential_panel {st.get_panel(current_spot)};
            // No effort is made to stay on the board, etc.
            if (!potential_panel)
                break;
            if (potential_panel->color != start_panel->color)
                continue;
            potential_blocks[row_diff-1].push_back(current_spot);
        }
    }
    for (unsigned i{0}; i < potential_blocks[0].size(); ++i)
    {
        for (unsigned j{0}; j < potential_blocks[1].size(); ++j)
        {
            std::array<spot, 3> test_spots {start_spot, potential_blocks[0][i], potential_blocks[1][j]};
            std::optional<unsigned> dest_column {can_get_to_same_column(st, test_spots)};
            if (dest_column)
                return {{start_spot, potential_blocks[0][i], potential_blocks[1][j]}};
        }
    }
    return {};
}

std::optional<std::array<spot, 3>> find_three_blocks_horizontal(stack const & st, spot const & start_spot)
{
    std::optional<panel> const start_panel{st.get_panel(start_spot)};
    if (!start_panel)
    {
        LOG("WARNING No panel at start spot: " + start_spot.to_string());
        return {};
    }
    if (!start_panel->color)
    {
        // Empty spot, do nothing
        return {};
    }
    std::vector<spot> found_blocks;
    for (int col{0}; col < st.width(); ++col)
    {
        if (col == start_spot.col)
            continue;
        spot const current_spot {start_spot.row, col};
        std::optional<panel> potential_panel {st.get_panel(current_spot)};
        // No effort is made to stay on the board, etc.
        if (!potential_panel)
            break;
        if (potential_panel->color != start_panel->color)
            continue;
        // If the current spot can get to the start spot, it's good enough
        // Of course, you won't actually want to move it all the way to the start spot
        if (!can_get_there(st, start_spot, current_spot))
            continue;
        found_blocks.push_back(current_spot);
        if (found_blocks.size() == 2)
            return {{start_spot, found_blocks[0], found_blocks[1]}};
    }
    return {};
}

std::optional<plan> bring_together_horizontal(std::array<spot, 3> const & source_blocks)
{
    auto sorted_blocks{source_blocks};
    spot const leftmost_block{*std::min_element(sorted_blocks.begin(), sorted_blocks.end(), [](spot const & a, spot const & b) { return a.col < b.col; })};
    std::array<spot, 3> target_blocks{sorted_blocks[0], {sorted_blocks[0].row, sorted_blocks[0].col+1},
            {sorted_blocks[0].row, sorted_blocks[0].col+2}};
    return create_plan(sorted_blocks, target_blocks);
}

static bool good_base(panel const & p)
{
    return p.color != 0 && p.state != "falling" && p.state != "popped";
}

static bool can_pass_through(panel const & p)
{
    return p.state != "popping" && p.state != "matched" && p.state != "popped" &&
            p.state != "dimmed" && p.state != "falling";
}

// Given two spots horizontally adjacent, check between them to see if there are any holes or walls
bool can_get_there(stack const & st, const spot & start_spot, const spot & target_spot)
{
    int const start_col = std::min(start_spot.col, target_spot.col);
    int const end_col = std::max(start_spot.col, target_spot.col);
    if (start_spot.row < 1)
    {
        // If we're trying to slide blocks on row 0, there should be no holes underneath.
        return true;
    }
    for (int col{start_col}; col <= end_col; ++col)
    {
        std::optional<panel> p {st.get_panel({start_spot.row-1,col})};
        if (!p)
        {
            LOG("ERROR: no panel at " + std::to_string(start_spot.row-1) + ", " + std::to_string(col));
            return false;
        }
        if (!good_base(*p))
        {
            return false;
        }
        p = st.get_panel({start_spot.row, col});
        if (!p)
        {
            LOG("ERROR: no panel at " + std::to_string(start_spot.row) + ", " + std::to_string(col));
            return false;
        }
        if (!can_pass_through(*p))
        {
            return false;
        }
    }
    return true;
}

int increment_column_diff(int old_column_diff)
{
    if (!old_column_diff)
        return 1;
    if (old_column_diff < 0)
        return old_column_diff - old_column_diff*2+1;
    return old_column_diff - old_column_diff*2;
}

std::vector<spot> move_panel(spot const & whence, spot const & whither)
{
    std::vector<spot> ret;
    const int num_flips = whence.col - whither.col;
    if (num_flips == 0)
        return {};
    const bool flip_left = num_flips > 0;
    int flip_spot = whence.col;
    if (flip_left)
    {
        flip_spot -= 1;
        while (flip_spot >= whither.col)
        {
            const spot temp_flip_spot {whence.row, flip_spot};
            // The flip_spot is the left side of the cursor
            ret.push_back(temp_flip_spot);
            --flip_spot;
        }
    } else {
        while (flip_spot < whither.col)
        {
            const spot temp_flip_spot {whence.row, flip_spot};
            // The flip_spot is the left side of the cursor
            ret.push_back(temp_flip_spot);
            ++flip_spot;
        }
    }
    return ret;
}

// A recursive algorithm:
// 1. If there are 2 or fewer blocks, move them into place naively
// 2. If there are three or more blocks
//    a. Call this algorithm with the outer two blocks removed (or only remove the rightmost if there are only three)
//    b. Move the outer blocks into place naively
std::vector<spot> move_horizontal_blocks(std::vector<spot> const & source_spots,
            std::vector<spot> const & target_spots)
{
    if (source_spots.size() != target_spots.size())
    {
        LOG("ERROR: move_horizontal_blocks: source_spots and target_spots have different sizes");
        return {};
    }
    if (source_spots.empty())
    {
        LOG("ERROR: move_horizontal_blocks: source_spots and target_spots are empty");
        return {};
    }
    std::vector<spot> ret{0};
    std::vector<spot> sorted_source_spots {source_spots};
    std::vector<spot> sorted_target_spots {target_spots};
    std::sort(sorted_source_spots.begin(), sorted_source_spots.end(), [](spot const & a, spot const & b)
                { return a.col < b.col; });
    std::sort(sorted_target_spots.begin(), sorted_target_spots.end(), [](spot const & a, spot const & b)
                { return a.col < b.col; });
    std::cout << "Moving from ";
    unsigned spot_i{0};
    for (unsigned c{0}; c < 6; ++c)
    {
        if (sorted_source_spots.size() > spot_i && sorted_source_spots[spot_i].col == static_cast<int>(c))
        {
            ++spot_i;
            std::cout << 'x';
        }
        else
            std::cout << '-';
    }
    std::cout <<'\n';
    std::cout << "Moving to   ";
    spot_i = 0;
    for (unsigned c{0}; c < 6; ++c)
    {
        if (sorted_target_spots.size() > spot_i && sorted_target_spots[spot_i].col == static_cast<int>(c))
        {
            ++spot_i;
            std::cout << 'x';
        }
        else
            std::cout << '-';
    }
    std::cout << '\n';
    bool left_spot_moving_left {sorted_source_spots.front().col > sorted_target_spots.front().col};
    // If we can move leftmost to the left, do it.
    if (left_spot_moving_left)
    {
        std::cout << "left moving left...\n";
        ret = move_panel(sorted_source_spots.front(), sorted_target_spots.front());
        if (sorted_source_spots.size() == 1)
            return ret;
        sorted_source_spots.erase(sorted_source_spots.begin());
        sorted_target_spots.erase(sorted_target_spots.begin());
        std::vector<spot> temp_moves{move_horizontal_blocks(sorted_source_spots, sorted_target_spots)};
        std::copy(temp_moves.begin(), temp_moves.end(), std::back_inserter(ret));
        return ret;
    }
    bool right_spot_moving_right {sorted_source_spots.back().col < sorted_target_spots.back().col};
    // If we can move rightmost to the right, do it.
    if (right_spot_moving_right)
    {
        std::cout << "right moving right...\n";
        ret = move_panel(sorted_source_spots.back(), sorted_target_spots.back());
        if (sorted_source_spots.size() == 1)
            return ret;
        sorted_source_spots.pop_back();
        sorted_target_spots.pop_back();
        std::vector<spot> temp_moves{move_horizontal_blocks(sorted_source_spots, sorted_target_spots)};
        std::copy(temp_moves.begin(), temp_moves.end(), std::back_inserter(ret));
        return ret;
    }
    if (sorted_source_spots.size() <= 2)
    {
        std::cout << " Only " << sorted_source_spots.size() << " spots to move...doing it\n";
        // Move the left spot to its place
        ret = move_panel(sorted_source_spots.front(), sorted_target_spots.front());
        if (sorted_source_spots.size() > 1)
        {
            std::cout << "   greater than one, doing the second.\n";
            // Move the right spot to its place
            std::vector<spot> temp_moves {move_panel(sorted_source_spots.back(), sorted_target_spots.back())};
            std::copy(temp_moves.begin(), temp_moves.end(), std::back_inserter(ret));
        }
    }
    else
    {
        // trim down the operation to do inner moves without messing things up.
        std::vector<spot> send_source_spots {sorted_source_spots};
        std::vector<spot> send_target_spots {sorted_target_spots};
        // Trim off the leftmost spot
        send_source_spots.pop_back();
        send_target_spots.pop_back();
        send_source_spots.erase(send_source_spots.begin());
        send_target_spots.erase(send_target_spots.begin());
        std::cout << "    sending " << send_source_spots.size() << " spots to be placed...\n";
        ret = move_horizontal_blocks(send_source_spots, send_target_spots);
        std::cout << "   moving the outer two blocks to finish up this iteration\n";
        // Now put the outer blocks into position
        std::vector<spot> temp_moves = move_panel(sorted_source_spots.front(), sorted_target_spots.front());
        std::copy(temp_moves.begin(), temp_moves.end(), std::back_inserter(ret));
        temp_moves = move_panel(sorted_source_spots.back(), sorted_target_spots.back());
        std::copy(temp_moves.begin(), temp_moves.end(), std::back_inserter(ret));
    }
    return ret;
}
