#pragma once
#include <string>
#include <iostream>

std::string prepare_file_name(char const *);
std::string get_thread_string(void);

void do_log(std::string const & logger, unsigned linenum, std::string const & message);

#define LOG(...) do_log(prepare_file_name(__FILE__), __LINE__, __VA_ARGS__)
