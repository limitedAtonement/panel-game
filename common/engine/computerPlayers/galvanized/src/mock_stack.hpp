#pragma once
#include "stack.hpp"
#include "utils.hpp"

struct mock_stack
{
    mock_stack(stack &&);
    // Returns false if the move is invalid
    bool make_move(spot const &);
    unsigned get_total_combinations(void) const;
    // Returns the combinations that were made in the last step
    std::vector<std::vector<spot>> const & get_combinations(void) const;
    // Move forward one step (falling blocks fall one, matches turn to matched blocks, etc.)
    // Returns true if anything changed.
    bool step(void);
    // Move forward steps until no more changes will happen
    void settle(void);

private:
    stack my_stack;
    std::vector<std::vector<spot>> combinations;
    unsigned total_combinations{0};

    void check_combinations(void);
};
