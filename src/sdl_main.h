#if !defined(SDL_MAIN_H)
struct sdl_offscreen_buffer
{
    void *memory;
    int width;
    int height;
    int width_in_bytes;
    int bytes_per_pixel;
};
struct sdl_window_dimensions
{
    int width;
    int height;
};
struct sdl_state
{
    u64 total_memory_size;
    void *engine_memory_block;
};
#define SDL_MAIN_H
#endif