#pragma once
#include <string>
#include <deque>

struct spot
{
    int row;
    int col;
    spot() = default;
    spot(int row, int col) : row(row), col(col) {}
    spot(spot const &) = default;
    spot & operator=(spot const &) = default;
    bool operator==(spot const & other) const = default;
    bool operator!=(spot const & other) const = default;
    std::string to_string(void) const;
};

struct plan
{
    // A flip spot is the left side of the cursor when the flip happens
    std::deque<spot> spot_flips;
    std::string to_string(void) const;
};
