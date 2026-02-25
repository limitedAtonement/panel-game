#include "logging.hpp"
#include "stack.hpp"
#include <iomanip>
#include <sstream>

stack::stack(int height, int width)
    : shapes(height * width, 0), height(height), width(width)
{
}

unsigned stack::get_shape(int row, int col) const
{
    return shapes[row * width + col];
}

void stack::set_shape(int row, int col, unsigned shape)
{
    shapes[row * width + col] = shape;
}

void stack::print(void) const
{
    std::ostringstream str;
    str << "stack:\n";
    for (int row = height-1; row >= 0; --row)
    {
        str << "  row " << std::setw(2) << std::setfill('0') << std::fixed << row + 1 << ":";
        for (int col = 0; col < width; ++col)
        {
            str << " " << shapes[row * width + col];
        }
        str << "\n";
    }
    LOG(str.str());
}

