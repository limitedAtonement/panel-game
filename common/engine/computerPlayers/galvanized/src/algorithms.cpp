#include "algorithms.hpp"
#include "logging.hpp"

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
    bool stop{false};
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
