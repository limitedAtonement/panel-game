#include "test_utils.hpp"
#include "utils.hpp"
#include "stack.hpp"
#include <vector>
#include <iostream>

unsigned const rows {12};
// Returns a stack with 60 unique colors: [10,70].
// Make patterns with numbers below 10 to be sure they are unique.
stack get_full_unique_stack(void)
{
    unsigned const cols {6};
    stack st(rows, cols);
    unsigned const fill_rows{rows-2};
    int color{10};
    for (unsigned row{0}; row < fill_rows; ++row)
    {
        for (unsigned col{0}; col < cols; ++col)
        {
            st.set_panel(row,col, {color++, row == 0 ? "dimmed" : "normal"});
        }
    }
    return st;
}

stack get_stack(std::vector<std::string> const & rows, bool includes_bottom_row)
{
    stack st{get_full_unique_stack()};
    int start_row{static_cast<int>(rows.size())};
    if (includes_bottom_row)
        --start_row;
    for (int stack_row{start_row}, text_row{0}; stack_row >= 0 && text_row < static_cast<int>(rows.size()); --stack_row, ++text_row)
    {
        int char_num{0};
        while (true)
        {
            if (static_cast<int>(rows[text_row].size()) <= char_num)
                break;
            int col = {char_num / 2};
            if (col >= st.width())
            {
                std::cerr << "Row " << text_row << " has too many columns (" << col << ")\n";
                return stack{};
            }
            switch (rows[text_row][char_num])
            {
                case 'x':
                    st.set_panel(stack_row, col, {1, "normal"});
                    break;
                case 'o':
                    st.set_panel(stack_row, col, {2, "normal"});
                    break;
                case ' ':
                    st.set_panel(stack_row, col, {0, "normal"});
                    break;
                case '-':
                default:
                    break;
            }
            char_num += 2;
        }
    }
    return st;
}
