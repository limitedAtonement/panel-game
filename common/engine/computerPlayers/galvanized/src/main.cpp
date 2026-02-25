#include <iostream>
#include "logging.hpp"
#include <lua.hpp>
#include "stack.hpp"
#include "galvanized.hpp"
#include <limits>

// Use documentation at https://www.lua.org/manual/5.1/manual.html
// According to https://archlinux.org/packages/extra/x86_64/luajit/ it says,
// "Just-in-time compiler and drop-in replacement for Lua 5.1"
int get_integer(lua_State* L, char const * field)
{
    lua_getfield(L, -1, field);
    lua_Integer const ret {lua_tointeger(L, -1)};
    if (ret > std::numeric_limits<int>::max())
    {
        LOG(std::string{field} + " is too large");
    }
    lua_pop(L, 1);
    return ret;
}

double get_number(lua_State* L, char const * field)
{
    // Push the field value onto the stack
    lua_getfield(L, -1, field);
    double const ret {lua_tonumber(L, -1)};
    lua_pop(L, 1);
    return ret;
}

static void read_stack(lua_State* L, stack & the_stack)
{
    lua_getfield(L, -1, "panels");
    size_t const rows {lua_objlen(L, -1)};
    for (int row{0}; row < rows; row++)
    {
        lua_pushinteger(L, row);
        // Push the next row of panels onto the stack
        lua_gettable(L, -2);
        size_t const cols {lua_objlen(L, -1)};
        for (int col{1}; col <= cols; col++)
        {
            lua_pushinteger(L, col);
            lua_gettable(L, -2);
            if (lua_isnil(L, -1))
            {
                // We don't expect NIL entries. If there is no panel, it should be color 0.
                LOG("No panel at row " + std::to_string(row) + " column " + std::to_string(col));
                // Pop the panel
                lua_pop(L, 1);
                continue;
            }
            // Top of the stack should be a panel
            lua_getfield(L, -1, "color");
            the_stack.set_shape(row, col-1, lua_tointeger(L, -1));
            // Pop the panel and color
            lua_pop(L, 2);
        }
        // Pop the row
        lua_pop(L, 1);
    }
    //the_stack.print();
}

static int get_input(lua_State* L)
{
    int const height {static_cast<int>(get_number(L, "height"))};
    int const width {static_cast<int>(get_number(L, "width"))};
    static stack the_stack{height, width};
    read_stack(L, the_stack);
    static galvanized the_galvanized{100};
    std::string const input {the_galvanized.get_input(the_stack)};
    lua_pushstring(L, input.c_str());
    int const taunt {the_galvanized.get_taunt()};
    lua_pushinteger(L, taunt);
    return 2;
}

static const luaL_Reg funcs[] = {
    {"getInput", get_input},
    {nullptr, nullptr}
};

// Lua looks for this function based on the module or directory of the so.
extern "C" int luaopen_common_engine_computerPlayers_galvanized_dist_galvanized(lua_State* L)
{
    luaL_newlib(L, funcs);
    return 1;
}
