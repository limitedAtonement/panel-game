#include "logging.hpp"
#include "galvanized.hpp"
#include <string>

galvanized::galvanized(int speed)
: speed{speed}
{
}

int const raise_mask {32};
int const swap_mask {16};
int const up_mask {8};
int const down_mask {4};
int const left_mask {2};
int const right_mask {1};
int const idle_mask {0};
char const * const base64encode = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890+/";

std::string galvanized::get_input(stack const & the_stack)
{
    if (!counter--) {
        counter = speed;
        LOG("returning calculated input!");
        return get_input_impl(the_stack);
    }
    return {base64encode[idle_mask]};
}

std::string galvanized::get_input_impl(stack const & the_stack)
{
    int const choice = rand() % 4;
    int input;
    switch (choice) {
        case 0: input = down_mask; break;
        case 1: input = up_mask; break;
        case 2: input = left_mask; break;
        case 3: input = right_mask; break;
    }
    if (choice % 2 == 0) {
        input &= swap_mask;
    }
    //LOG("input returning " + std::to_string(input));
    return {base64encode[input]};
}

int galvanized::get_taunt()
{
    return 1;
}
