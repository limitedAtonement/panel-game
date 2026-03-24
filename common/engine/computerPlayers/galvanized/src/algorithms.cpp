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

bool good_base(panel const & p)
{
    return p.color != 0 && p.state != "falling" && p.state != "popped";
}

bool can_pass_through(panel const & p)
{
    return p.state != "popping" && p.state != "matched" && p.state != "popped" &&
            p.state != "dimmed" && p.state != "falling";
}

bool can_match(panel const & p)
{
    if (!p.color)
        return false;
    return p.state == "normal" || p.state == "landing";
}

bool can_swap(panel const & p)
{
    if (!p.color)
        return false;
    return p.state == "normal" || p.state == "hovering" || p.state == "falling";
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
    /*std::cout << "Moving from ";
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
    */
    bool left_spot_moving_left {sorted_source_spots.front().col > sorted_target_spots.front().col};
    // If we can move leftmost to the left, do it.
    if (left_spot_moving_left)
    {
        //std::cout << "left moving left...\n";
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
        //std::cout << "right moving right...\n";
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
        //std::cout << " Only " << sorted_source_spots.size() << " spots to move...doing it\n";
        // Move the left spot to its place
        ret = move_panel(sorted_source_spots.front(), sorted_target_spots.front());
        if (sorted_source_spots.size() > 1)
        {
            //std::cout << "   greater than one, doing the second.\n";
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
        //std::cout << "    sending " << send_source_spots.size() << " spots to be placed...\n";
        ret = move_horizontal_blocks(send_source_spots, send_target_spots);
        //std::cout << "   moving the outer two blocks to finish up this iteration\n";
        // Now put the outer blocks into position
        std::vector<spot> temp_moves = move_panel(sorted_source_spots.front(), sorted_target_spots.front());
        std::copy(temp_moves.begin(), temp_moves.end(), std::back_inserter(ret));
        temp_moves = move_panel(sorted_source_spots.back(), sorted_target_spots.back());
        std::copy(temp_moves.begin(), temp_moves.end(), std::back_inserter(ret));
    }
    return ret;
}

// Returns the longest vertical combination going up from the specified spot
static std::optional<std::vector<spot>> find_vertical_combination(stack const & st, spot const & start_spot)
{
    std::optional<panel> const start_panel{st.get_panel(start_spot)};
    if (!start_panel)
    {
        LOG("WARNING No panel at start spot: " + start_spot.to_string());
        return {};
    }
    if (!can_match(*start_panel))
    {
        // Won't work
        return {};
    }
    std::vector<spot> ret{start_spot};
    for (int row{start_spot.row+1}; row < st.height(); ++row)
    {
        bool got_one{false};
        // Used to make sure we can move a potential block over to make the vertical combination
        spot dest{row, start_spot.col};
        for (int col{0}; col < st.width(); ++col)
        {
            spot temp_spot{row,col};
            std::optional<panel> const temp_panel{st.get_panel(temp_spot)};
            if (!temp_panel)
            {
                LOG("WARNING: find_vertical_combination:no panel at tempspot");
                return {};
            }
            if (!can_match(*temp_panel))
                continue;
            if (temp_panel->color != start_panel->color)
                continue;
            if (!can_get_there(st, temp_spot, dest))
                continue;
            ret.push_back(temp_spot);
            got_one = true;
            break;
        }
        if (!got_one)
            break;
    }
    if (ret.size() > 2)
        return ret;
    return {};
}

std::vector<std::vector<spot>> find_vertical_combinations(stack const & st)
{
    // We store the color along with the spots of the combinations
    std::vector<std::tuple<int, std::vector<spot>>> ret;
    for (int row{0}; row < st.height(); row++)
    {
        for (int col{0}; col < st.width(); col++)
        {
            std::optional<panel> temp_panel{st.get_panel({row, col})};
            // We need to de-duplicate. Consider
            // - - 1 - -
            // - - - 1 - 
            // 1 1 - - -
            // This will return two vertical combinations (one for each of the base 1s), but
            // we only want one.
            bool found_duplicate{false};
            for (auto const & t : ret)
            {
                if (std::get<0>(t) != temp_panel->color)
                    continue;
                std::vector<spot> const & temp_spots {std::get<1>(t)};
                for (spot const & temp_spot : temp_spots)
                {
                    if (temp_spot.row == row)
                    {
                        found_duplicate = true;
                        break;
                    }
                }
                if (found_duplicate)
                    break;
            }
            if (found_duplicate)
                continue;
            std::optional<std::vector<spot>> vertical {find_vertical_combination(st, {row, col})};
            if (vertical)
                ret.push_back(std::make_tuple(temp_panel->color, std::move(*vertical)));
        }
    }
    std::vector<std::vector<spot>> real_ret;
    std::transform(ret.begin(), ret.end(), std::back_inserter(real_ret), [](auto const & t) {
        return std::move(std::get<1>(t));
    });
    return real_ret;
}

// Returns all the spots in a vertical/horizontal combination along with the row on which the horizontal is
// if we were able to add a horizontal component. Otherwise returns nothing.
static std::optional<std::tuple<int,std::vector<spot>>> add_horizontal(stack const & st, std::vector<spot> const & vertical)
{
    for (spot const & start_spot : vertical)
    {
        std::optional<panel> start_panel {st.get_panel(start_spot)};
        if (!start_panel)
        {
            LOG("WARNING add_horizontal: No panel at start spot: " + start_spot.to_string());
            return {};
        }
        std::vector<spot> additional_spots;
        for (int col{0}; col < st.width(); ++col)
        {
            if (col == start_spot.col)
                continue;
            spot const temp_spot{start_spot.row, col};
            std::optional<panel> temp_panel{st.get_panel(temp_spot)};
            if (!temp_panel)
            {
                LOG("WARNING add_horivontal: no panel at temp spot: " + temp_spot.to_string());
                return {};
            }
            if (temp_panel->color != start_panel->color)
                continue;
            if (!can_get_there(st, start_spot, temp_spot))
                continue;
            additional_spots.push_back(temp_spot);
        }
        if (additional_spots.size() < 2)
        {
            additional_spots.clear();
            continue;
        }
        std::copy(vertical.begin(), vertical.end(), std::back_inserter(additional_spots));
        return {{start_spot.row, additional_spots}};
    }
    return {};
}

// If the provided spot can be swapped left or right, return the appropriate swap.
static std::optional<spot> can_swap_left_or_right(stack const & st, spot const & s)
{
    std::optional<panel> right{st.get_panel({s.row, s.col+1})};
    if (right && can_swap(*right))
        return s;
    std::optional<panel> left{st.get_panel({s.row, s.col-1})};
    if (left && can_swap(*left))
        return spot{s.row,s.col-1};
    return {};
}

// Makes way in a particular row consider particular other rows for a particular color
static std::optional<plan> clear_way(stack const & st, int row_clear, int row_start, int row_end, int col, int color)
{
    unsigned problems{0};
    for (int row{row_start}; row <= row_end; ++row)
    {
        if (row == row_clear)
            continue;
        std::optional<panel> temp_panel{st.get_panel({row, col})};
        if (!temp_panel)
        {
            LOG("WARNING clear_way: no panel at " + std::to_string(row) + ", " + std::to_string(col));
            return {};
        }
        if (can_match(*temp_panel) && temp_panel->color == color)
            ++problems;
    }
    // If there's no problem, there's no problem
    if (problems < 2)
        return {};
    // Since there is a problem, we need to be able to clear one of them.
    for (int row{row_start}; row <= row_end; ++row)
    {
        if (row == row_clear)
            continue;
        spot temp_spot{row, col};
        std::optional<panel> temp_panel{st.get_panel(temp_spot)};
        std::optional<spot> swap_spot{can_swap_left_or_right(st, temp_spot)};
        if (swap_spot)
            return plan{{*swap_spot}};
    }
    return {};
}

std::optional<plan> clear_way(stack const & st, int row, int start_col, int end_col, int color)
{
    plan ret;
    bool check_middle{true};
    std::optional<plan> temp_plan;
    for (int col{start_col}; col <= end_col; ++col)
    {
        // check top
        if (row < st.height()-2)
        {
            temp_plan = clear_way(st, row, row, row+2, col, color);
            if (temp_plan)
            {
                // If just above this row was disrupted, we don't need to check a possible middle combination.
                for (spot const & flip : temp_plan->spot_flips)
                {
                    if (flip.row == row+1)
                    {
                        check_middle = false;
                        break;
                    }
                }
                std::copy(temp_plan->spot_flips.begin(), temp_plan->spot_flips.end(), std::back_inserter(ret.spot_flips));
            }
        }
        // check bottom
        if (row > 1)
        {
            temp_plan = clear_way(st, row, row-2, row, col, color);
            if (temp_plan)
            {
                // If just below this row was disturbed, we won't need to check for a middle combination.
                for (spot const & flip : temp_plan->spot_flips)
                {
                    if (flip.row == row-1)
                    {
                        check_middle = false;
                        break;
                    }
                }
                std::copy(temp_plan->spot_flips.begin(), temp_plan->spot_flips.end(), std::back_inserter(ret.spot_flips));
            }
        }
        if (check_middle && row > 0 && row < st.height()-1)
        {
            temp_plan = clear_way(st, row, row-1, row+1, col, color);
            if (temp_plan)
            {
                std::copy(temp_plan->spot_flips.begin(), temp_plan->spot_flips.end(), std::back_inserter(ret.spot_flips));
            }
        }
    }
    if (ret.spot_flips.size())
        return ret;
    return {};
}

std::optional<bool> finish_hor_vert_combo_left(stack const & st, std::vector<spot> const & spots, int hor_row)
{
    int vertical_average_col{0};
    int vertical_average_count{0};
    for (spot const & s : spots)
    {
        if (s.row == hor_row)
            continue;
        vertical_average_col += s.col;
        ++vertical_average_count;
    }
    if (vertical_average_count == 0)
        return {};
    vertical_average_col /= vertical_average_count;
    if (vertical_average_col < 2) 
        return true;
    if (vertical_average_col > st.width()-3)
        return false;
    int hor_count_right{0};
    for (spot const & s : spots)
    {
        if (s.row != hor_row)
            continue;
        if (s.col > vertical_average_col)
            ++hor_count_right;
    } 
    // If most of the horizontal blocks are to the right, we should finish from the left.
    return hor_count_right > 1;
}

std::optional<plan> execute_vert_hor_combination(stack const & st, std::vector<spot> const & spots, int hor_row)
{
    std::optional<bool> maybe_finish_left{finish_hor_vert_combo_left(st, spots, hor_row)};
    if (!maybe_finish_left)
        // Something must be wrong?
        return {};
    bool finish_left{*maybe_finish_left};
    int vertical_average_col{0};
    int vertical_average_count{0};
    for (spot const & s : spots)
    {
        if (s.row == hor_row)
            continue;
        vertical_average_col += s.col;
        ++vertical_average_count;
    }
    if (vertical_average_count == 0)
        return {};
    plan ret;
    vertical_average_col /= vertical_average_count;
    vertical_average_col = std::clamp(vertical_average_col, 1, st.width()-2);
    std::vector<spot> hor_spots;
    std::copy_if(spots.begin(), spots.end(), std::back_inserter(hor_spots), [hor_row](spot const & s)
            {return s.row == hor_row;});
    if (hor_spots.size() < 3)
    {
        LOG("Error, execute_vert_hor_combination: expected at least 3 horizontal spots");
        return {};
    }
    std::vector<spot> target_spots;
    if (finish_left)
    {
        // Get two blocks right of the vertical column and one left
        target_spots = {{hor_row, vertical_average_col-1},
                {hor_row, vertical_average_col+1}, {hor_row, vertical_average_col+2}};
    }
    else
    {
        target_spots = {{hor_row, vertical_average_col+1}, {hor_row, vertical_average_col-1},
            {hor_row, vertical_average_col-2}};
    }
    std::optional<std::vector<spot>> horizontal_plan{move_horizontal_blocks(hor_spots, target_spots)};
    if (!horizontal_plan)
    {
        LOG("Error? execute_vert_hor_combination, failed to move horizontal blocks");
        return {};
    }
    std::copy(horizontal_plan->begin(), horizontal_plan->end(), std::back_inserter(ret.spot_flips));
    // Now get the vertical blocks in line
    for (spot const & s : spots)
    {
        if (s.row == hor_row)
            continue;
        std::optional<std::vector<spot>> temp_plan{move_panel(s, {s.row, vertical_average_col})};
        if (!temp_plan)
        {
            LOG("Error? execute_vert_hor_combination, failed to move vertical block into place");
            return {};
        }
        std::copy(temp_plan->begin(), temp_plan->end(), std::back_inserter(ret.spot_flips));
    }
    // Add the last move
    if (finish_left)
    {
        ret.spot_flips.push_back({hor_row, vertical_average_col-1});
    }
    else
    {
        ret.spot_flips.push_back({hor_row, vertical_average_col});
    }
    return ret;
}

std::vector<plan> find_vert_hor_combos(stack const & st)
{
    std::vector<plan> ret;
    std::vector<std::vector<spot>> verticals {find_vertical_combinations(st)};
    for (std::vector<spot> vertical: verticals)
    {
        std::optional<std::tuple<int, std::vector<spot>>> added {add_horizontal(st, vertical)};
        if (!added)
            continue;
        int hor_row {std::get<0>(*added)};;
        std::vector<spot> spots {std::move(std::get<1>(*added))};;
        std::optional<plan> temp_plan {execute_vert_hor_combination(st, spots, hor_row)};
        if (temp_plan)
            ret.push_back(*temp_plan);
    }
    return ret;
}
