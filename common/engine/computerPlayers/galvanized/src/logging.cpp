#include "logging.hpp"

std::string prepare_file_name(char const * file)
{
    std::string name {file};
    auto index {name.find_last_of('/')};
    name = name.substr(index + 1);
    index = name.find_last_of('.');
    name = name.substr(0, index);
    return name;
}

void do_log(std::string const & logger, unsigned linenum, std::string const & message)
{
    std::cout << "[" << logger << ":" << linenum << "] " << message << '\n';
}
