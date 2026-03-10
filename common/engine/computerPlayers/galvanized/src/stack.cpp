#include "logging.hpp"
#include "stack.hpp"
#include <iomanip>
#include <sstream>

stack::stack(int height, int width)
    : panels(height * width, panel{}), _height(height), _width(width)
{
}

stack::stack(void)
: _height{0}, _width{0}
{
}

std::optional<panel> stack::get_panel(spot const & s) const
{
    if (s.row < 0 || s.row >= _height || s.col < 0 || s.col >= _width)
    {
        LOG("Invalid spot: " + std::to_string(s.row) + ", " + std::to_string(s.col) + " for stack of height " +
                std::to_string(_height) + " and width " + std::to_string(_width));
        return {};
    }
    return panels[s.row * _width + s.col];
}

void stack::set_panel(int row, int col, panel && p)
{
    std::string const state{p.state};
    panels[row * _width + col] = std::move(p);
    //if (row == 0)
    //{
        //if (col == 0 && state != "dimmed")
            //LOG("Panel at " + std::to_string(row) + ", " + std::to_string(col) + " state " + state);
    //}
    //else if (state != "normal")
        //LOG("Panel at " + std::to_string(row) + ", " + std::to_string(col) + " state " + state);
}

void stack::print(void) const
{
    std::ostringstream str;
    str << "stack:\n";
    for (int row = _height-1; row >= 0; --row)
    {
        str << "  row " << std::setw(2) << std::setfill('0') << std::fixed << row + 1 << ":";
        for (int col = 0; col < _width; ++col)
        {
            str << " " << panels[row * _width + col].color;
        }
        str << "\n";
    }
    LOG(str.str());
}

int stack::height(void) const
{
    return _height;
}

int stack::width(void) const
{
    return _width;
}
