#include "utils.hpp"
#include <sstream>

std::string spot::to_string(void) const
{
    return "{" + std::to_string(row) + "," + std::to_string(col) + "}";
}

std::string plan::to_string(void) const
{
    std::ostringstream ret;
    ret << "plan:{";
    for (auto const & flip : spot_flips)
    {
        ret << ',' << flip.to_string();
    }
    ret << '}';
    return ret.str();
}

bool operator<(spot const & lhs, spot const & rhs)
{
    if (lhs.row < rhs.row)
        return true;
    return lhs.col < rhs.col;
}
