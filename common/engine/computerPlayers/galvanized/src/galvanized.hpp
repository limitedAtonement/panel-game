#pragma once
#include "stack.hpp"
#include <thread>
#include <semaphore>
#include <chrono>
#include <mutex>
#include "utils.hpp"

struct galvanized
{
    galvanized(int speed);
    std::string get_input(stack const &);
    int get_taunt(void);
    void set_cursor_spot(spot const &);
    void set_displacement(int);
private:
    int const speed;
    int counter{0};
    // Only accessed by calculation thread
    stack my_stack;
    // This is updated very often and protected by stack_buffer_mutex
    stack stack_buffer;
    // Only accessed by main thread
    std::chrono::time_point<std::chrono::high_resolution_clock> last_taunt_sent;
    // Protected by stack_buffer_mutex
    std::optional<plan> current_plan;
    // Protected by stack_buffer_mutex
    spot cursor_spot;
    std::thread calculation_thread;
    std::mutex stack_buffer_mutex;
    std::binary_semaphore start_game_semaphore {0};
    // A number between [16,1]. When this value goes back up to 16, a new row has been added.
    int last_displacement{-1};
    // Protected by stack_buffer_mutex
    bool raise_stack{false};

    std::string get_input_impl(stack const &);
    void calculation_func(void);
    void calculate_impl(void);
    void find_three_blocks(void);
    std::optional<std::array<spot, 3>> get_three_blocks_target(std::array<spot, 3> const &);
    std::optional<plan> create_plan(std::array<spot, 3> const &, std::array<spot, 3> const &);
    void wait_for_plan_to_complete(void);
    void advance_stack(void);
};
