#pragma once
#include "stack.hpp"

struct galvanized
{
    galvanized(int speeed);
    std::string get_input(stack const &);
    int get_taunt(void);
    void set_cursor_spot(unsigned row, unsigned col);
private:
    int const speed;
    int counter{0};
    stack my_stack;
    std::string get_input_impl(stack const &);

};
