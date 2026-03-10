#pragma once
#include <optional>
#include <vector>
#include "utils.hpp"
#include "panel.hpp"

struct stack
{
    stack(void);
    stack(int height, int width);
    stack(stack const &) = default;
    stack & operator=(stack const &) = default;
    std::optional<panel> get_panel(spot const &) const;
    // 0 is empty, 1-7 is the colors, 9 is garbage(unswitchable?)
    void set_panel(int row, int col, panel &&);
    void print(void) const;
    int height(void) const;
    int width(void) const;
private:
    // The bottom row is usually inaccessible.
    // First panel is lower left, second panel is one right of that.
    std::vector<panel> panels;
    int _height;
    int _width;
};
