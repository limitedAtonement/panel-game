#include "logging.hpp"
#include "galvanized.hpp"
#include <string>
#include <chrono>
#include "algorithms.hpp"

galvanized::galvanized(int speed)
: speed{speed}, calculation_thread(&galvanized::calculation_func, this)
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
    start_game_semaphore.release();
    if (!counter--) {
        counter = speed;
        //LOG("returning calculated input!");
        return get_input_impl(the_stack);
    }
    return {base64encode[idle_mask]};
}

std::string galvanized::get_input_impl(stack const & the_stack)
{
    std::optional<spot> maybe_flip_spot;
    spot temp_cursor_spot;
    {
        std::lock_guard<std::mutex> lock(stack_buffer_mutex);
        stack_buffer = the_stack;
        if (raise_stack)
        {
            return {base64encode[raise_mask]};
        }
        if (current_plan && current_plan->spot_flips.size())
        {
            maybe_flip_spot = current_plan->spot_flips.front();
            temp_cursor_spot = cursor_spot;
            //LOG("Cursor spot is: " + temp_cursor_spot.to_string() + " and maybe flip spot is: " + maybe_flip_spot->to_string());
            if (*maybe_flip_spot == temp_cursor_spot)
            {
                current_plan->spot_flips.pop_front();
            }
            if (!current_plan->spot_flips.size())
            {
                current_plan.reset();
                //LOG("Plan executed, resetting plan");
            }
        }
    }
    if (!maybe_flip_spot)
    {
        return {base64encode[idle_mask]};
    }
    spot const flip_spot {*maybe_flip_spot};
    if (flip_spot != temp_cursor_spot)
    {
        // Move to the flip spot
        int ret {0};
        if (flip_spot.row > temp_cursor_spot.row)
        {
            ret |= up_mask;
        }
        if (flip_spot.row < temp_cursor_spot.row)
        {
            ret |= down_mask;
        }
        if (flip_spot.col > temp_cursor_spot.col)
        {
            ret |= right_mask;
        }
        if (flip_spot.col < temp_cursor_spot.col)
        {
            ret |= left_mask;
        }
        return {base64encode[ret]};
    }
    return {base64encode[swap_mask]};
}

int galvanized::get_taunt()
{
    std::chrono::high_resolution_clock::time_point now {std::chrono::high_resolution_clock::now()};
    if (now - last_taunt_sent > std::chrono::seconds(10))
    {
        last_taunt_sent = now;
        return rand() % 2 == 0 ? 1 : 2;
    }
    return 0;
}

// Given three blocks of the same color, find the best spot to put those three blocks to clear them.
std::optional<std::array<spot, 3>> galvanized::get_three_blocks_target(std::array<spot, 3> const & starting_blocks)
{
    std::array<spot, 3> ret;
    for (unsigned i{0}; i < starting_blocks.size(); ++i)
    {
        ret[i] = {starting_blocks[i].row, starting_blocks[0].col};
        //LOG("get_three_blocks_target: Starting block " + std::to_string(i) + " is: " + starting_blocks[i].to_string());
        //LOG("    Target: " + ret[i].to_string());
    }
    return ret;
}

std::optional<plan> galvanized::create_plan(std::array<spot, 3> const & starting_blocks, std::array<spot, 3> const & target_blocks)
{
    plan ret;
    //LOG("Creating plan for starting blocks: " + starting_blocks[0].to_string() + " " + starting_blocks[1].to_string() + " " + starting_blocks[2].to_string());
    //LOG("   target blocks: " + target_blocks[0].to_string() + " " + target_blocks[1].to_string() + " " + target_blocks[2].to_string());
    for (size_t i{0}; i < starting_blocks.size(); ++i)
    {
        const int num_flips = starting_blocks[i].col - target_blocks[i].col;
        if (num_flips == 0)
            continue;
        const bool flip_left = num_flips > 0;
        int flip_spot = starting_blocks[i].col;
        if (flip_left)
        {
            flip_spot -= 1;
            while (flip_spot >= target_blocks[i].col)
            {
                const spot temp_flip_spot {starting_blocks[i].row, flip_spot};
                // The flip_spot is the left side of the cursor
                ret.spot_flips.push_back(temp_flip_spot);
                --flip_spot;
                //LOG("Adding flip left: " + temp_flip_spot.to_string());
            }
        } else {
            while (flip_spot < target_blocks[i].col)
            {
                const spot temp_flip_spot {starting_blocks[i].row, flip_spot};
                // The flip_spot is the left side of the cursor
                ret.spot_flips.push_back(temp_flip_spot);
                ++flip_spot;
                //LOG("Adding flip rigt: " + temp_flip_spot.to_string());
            }
        }
    }
    return ret;
}

void galvanized::wait_for_plan_to_complete(void)
{
    while (true)
    {
        std::optional<plan> temp_plan;
        {
            std::lock_guard<std::mutex> lock(stack_buffer_mutex);
            temp_plan = current_plan;
        }
        if (!temp_plan || !temp_plan->spot_flips.size())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void galvanized::find_three_blocks(void)
{
    // Don't use first row because it's the hidden row.
    for (int row{1}; row < my_stack.height(); row++)
    {
        for (int col{0}; col < my_stack.width(); col++)
        {
            spot const start_spot{row,col};
            std::optional<panel> temp_panel {my_stack.get_panel(start_spot)};
            if (!temp_panel)
            {
                LOG("ERROR: no panel at " + std::to_string(row) + ", " + std::to_string(col));
                return;
            }
            if (temp_panel->color == 0)
            {
                continue;
            }
            std::optional<std::array<spot, 3>> three_blocks {find_three_blocks_vertical(my_stack, start_spot)};
            if (!three_blocks)
                continue;
            LOG("Three vertical blocks found: " + (*three_blocks)[0].to_string() + " " + (*three_blocks)[1].to_string() + " " + (*three_blocks)[2].to_string());
            std::optional<std::array<spot, 3>> target {get_three_blocks_target(*three_blocks)};
            if (!target)
                continue;
            LOG(" got target for 3 verticals");
            std::optional<plan> plan {create_plan(*three_blocks, *target)};
            if (!plan)
                continue;
            LOG("   got a plan for 3 verticals");
            {
                std::lock_guard<std::mutex> lock(stack_buffer_mutex);
                current_plan = plan;
            }
            LOG("Plan created, waiting for it to complete");
            wait_for_plan_to_complete();
            return;
        }
    }
}

// Raise the stack until it's all the way up.
void galvanized::advance_stack(void)
{
    LOG("advancing stack...");
    bool raise{true};
    for (int col{0}; col < my_stack.width(); ++col)
    {
        // Don't check the top row because when anything is in the top row, we're done!
        std::optional<panel> const p {my_stack.get_panel({my_stack.height()-2, col})};
        if (!p)
        {
            LOG("ERROR: no panel at " + std::to_string(my_stack.height()-2) + ", " + std::to_string(col));
            return;
        }
        LOG("Checking panel at " + std::to_string(my_stack.height()-2) + ", " + std::to_string(col) + " with color " + std::to_string(p->color));
        if (p->color != 0)
        {
            LOG("Finished rising the stack.");
            raise = false;
            break;
        }
    }

    {
        std::lock_guard<std::mutex> lock(stack_buffer_mutex);
        this->raise_stack = raise;
        LOG("setting raise_stack to " + std::to_string(raise));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void galvanized::calculate_impl(void)
{
    // Do logic over and over until the game is over.
    while (true)
    {
        //LOG(" waiting to check stack...");
        {
            std::lock_guard<std::mutex> lock(stack_buffer_mutex);
            // Copy the most recent stack for computations
            my_stack = stack_buffer;

        }
        //LOG("    updated stack.");
        //my_stack.print();
        if (!my_stack.height())
        {
            //LOG("No stack, not calculating.");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return;
        }
        advance_stack();
        if (raise_stack)
        {
            LOG("Raising stack, sleeping");
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }
        //LOG("Calculating...");
        find_three_blocks();
    }
}

void galvanized::calculation_func()
{
    //LOG("Starting calculation function...");
    while (true)
    {
        //LOG("Waiting for game to start...");
        // Wait for game to start
        start_game_semaphore.acquire();
        calculate_impl();
    }
}

void galvanized::set_cursor_spot(spot const & s)
{
    {
        std::lock_guard<std::mutex> lock(stack_buffer_mutex);
        cursor_spot = s;
    }
}

void galvanized::set_displacement(int d)
{
    {
        std::lock_guard<std::mutex> lock(stack_buffer_mutex);
        if (last_displacement < d)
        {
            //LOG("Displacement reset, CLEARING plan");
            current_plan.reset();
        }
        last_displacement = d;
    }
}
