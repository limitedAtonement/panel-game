#include "logging.hpp"
#include "mock_stack.hpp"
#include "algorithms.hpp"
#include <set>

mock_stack::mock_stack(stack && st)
: my_stack(std::move(st))
{
}

bool mock_stack::step(void)
{
    check_combinations();
    return false;
}

bool mock_stack::make_move(spot const & s)
{
    std::optional<panel> left_panel = my_stack.get_panel(s);
    if (!left_panel)
        return false;
    std::optional<panel> right_panel = my_stack.get_panel({s.row, s.col+1});
    if (!right_panel)
        return false;
    if (!can_swap(*left_panel) || !can_swap(*right_panel))
    {
        // We can swap into empty space or falling panels
        if (can_swap(*left_panel))
        {
            if (right_panel->color != 0 && right_panel->state != "falling")
                return false;
        }
        else if (can_swap(*right_panel))
        {
            if (left_panel->color != 0 && left_panel->state != "falling")
                return false;
        }
        else
            return false;
    }
    my_stack.set_panel(s.row, s.col, std::move(*right_panel));
    my_stack.set_panel(s.row, s.col+1, std::move(*left_panel));
    return true;
}

static std::optional<spot> matches(stack const & st, spot const & s, panel const & p)
{
    if (s.row < 0 || s.col < 0 || s.row >= st.height() || s.col >= st.width())
        return {};
    if (!p.color)
        return {};
    std::optional<panel> test_panel = st.get_panel(s);
    if (!test_panel)
        return {};
    if (test_panel->color != p.color)
        return {};
    return s;
}

static std::optional<spot> match_above(stack const & st, spot const & s, panel const & p)
{
    spot above{s.row+1, s.col};
    return matches(st, above, p);
}

static std::optional<spot> match_below(stack const & st, spot const & s, panel const & p)
{
    spot below{s.row-1, s.col};
    return matches(st, below, p);
}

static std::optional<spot> match_left(stack const & st, spot const & s, panel const & p)
{
    spot left{s.row, s.col-1};
    return matches(st, left, p);
}

static std::optional<spot> match_right(stack const & st, spot const & s, panel const & p)
{
    spot right{s.row, s.col +1};
    return matches(st, right, p);
}

static bool match_above_and_below(stack const & st, spot const & s, panel const & p)
{
    return match_above(st, s, p) && match_below(st, s, p);
}

static bool match_left_and_right(stack const & st, spot const & s, panel const & p)
{
    return match_left(st, s, p) && match_right(st, s, p);
}

static bool find_spot(std::vector<std::vector<spot>> const & haystacks, spot const & needle)
{
    for (std::vector<spot> const & haystack : haystacks)
    {
        if (std::find(haystack.begin(), haystack.end(), needle) != haystack.end())
            return true;
    }
    return false;
}

std::vector<spot> check_combinations_impl(stack & st, int row, int col, panel const & p,
        std::vector<std::vector<spot>> const & already_matched)
{
    std::set<spot> matched_spots;
    std::deque<spot> spots_to_check {{row,col}};
    std::set<spot> checked_spots;
    while (spots_to_check.size())
    {
        spot current_spot{std::move(spots_to_check.front())};
        spots_to_check.pop_front();
        checked_spots.insert(current_spot);
        if (find_spot(already_matched, current_spot))
            continue;
        if (match_above_and_below(st, current_spot, p))
        {
            spot above{current_spot.row+1, current_spot.col};
            spot below{current_spot.row-1, current_spot.col};
            matched_spots.insert(current_spot);
            matched_spots.insert(above);
            matched_spots.insert(below);
            auto it {std::find(spots_to_check.begin(), spots_to_check.end(), above)};
            if (it == spots_to_check.end() && checked_spots.find(above) == checked_spots.end())
                spots_to_check.push_back(above);
            it = std::find(spots_to_check.begin(), spots_to_check.end(), below);
            if (it == spots_to_check.end() && checked_spots.find(below) == checked_spots.end())
                spots_to_check.push_back(below);
        }
        else
        {
            spot above{current_spot.row+1, current_spot.col};
            if (checked_spots.find(above) == checked_spots.end() && 
                    matches(st, above, p))
                spots_to_check.push_back(above);
            spot below{current_spot.row-1, current_spot.col};
            if (checked_spots.find(below) == checked_spots.end() && 
                    matches(st, below, p))
                spots_to_check.push_back(below);
        }
        if (match_left_and_right(st, current_spot, p))
        {
            spot left{current_spot.row, current_spot.col-1};
            spot right{current_spot.row, current_spot.col+1};
            matched_spots.insert(current_spot);
            matched_spots.insert(left);
            matched_spots.insert(right);
            auto it {std::find(spots_to_check.begin(), spots_to_check.end(), left)};
            if (it == spots_to_check.end() && checked_spots.find(left) == checked_spots.end())
                spots_to_check.push_back(left);
            it = std::find(spots_to_check.begin(), spots_to_check.end(), right);
            if (it == spots_to_check.end() && checked_spots.find(right) == checked_spots.end())
                spots_to_check.push_back(right);
        }
        else
        {
            spot right{current_spot.row+1, current_spot.col};
            if (checked_spots.find(right) == checked_spots.end() && 
                    matches(st, right, p))
                spots_to_check.push_back(right);
            spot left{current_spot.row-1, current_spot.col};
            if (checked_spots.find(left) == checked_spots.end() && 
                    matches(st, left, p))
                spots_to_check.push_back(left);
        }
    }
    for (spot const & s : matched_spots)
    {
        st.set_panel(s.row, s.col, panel{p.color, "matched"});
    }
    return {matched_spots.begin(), matched_spots.end()};
}

// Looks through the stack. Any combinations are changed to "matched" panels
void mock_stack::check_combinations(void)
{
    combinations.clear();
    for (int row{0}; row < my_stack.height(); ++row)
    {
        for (int col{0}; col < my_stack.width(); ++col)
        {
            std::optional<panel> panel = my_stack.get_panel({row, col});
            if (!panel)
            {
                LOG("ERROR: no panel at " + std::to_string(row) + ", " + std::to_string(col));
                continue;
            }
            if (panel->state != "normal")
                continue;
            if (find_spot(combinations, {row, col}))
                continue;
            std::vector<spot> combination {check_combinations_impl(my_stack, row, col, *panel, combinations)};
            if (combination.size())
            {
                combinations.push_back(std::move(combination));
            }
        }
    }
    total_combinations += combinations.size();
}

unsigned mock_stack::get_total_combinations(void) const
{
    return total_combinations;
}

std::vector<std::vector<spot>> const & mock_stack::get_combinations(void) const
{
    return combinations;
}
