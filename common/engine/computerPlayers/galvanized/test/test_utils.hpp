#pragma once
#include <vector>
#include <string>

struct stack get_full_unique_stack(void);
stack get_stack(std::vector<std::string> const &, bool includes_bottom_row=false);
