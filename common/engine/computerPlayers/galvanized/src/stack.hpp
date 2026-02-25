#pragma once
#include <vector>

struct stack
{
    stack(int height, int width);
    stack(stack const &);
    unsigned get_shape(int row, int col) const;
    // 0 is empty, 1-7 is the colors, 9 is garbage(unswitchable?)
    void set_shape(int row, int col, unsigned shape);
    void print(void) const;
private:
    // The bottom row is inaccessible.
    std::vector<unsigned> shapes;
    int const height;
    int const width;
};
