#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <cstring>

#include "nenjin.cpp"
#include "nenjin_platform.h"
#include "sdl_main.h"
global_variable bool32 global_alive = true;
global_variable sdl_offscreen_buffer global_back_buffer;

// DEBUG I/O
DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory) {
    free(memory);
}
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile) {
    // Open a read file handle.
    SDL_RWops *io = SDL_RWFromFile(file_name, "r");
    debug_read_file_result result = {};
    if(io)
    {
        result.content_size = SafeTruncateU64(SDL_RWsize(io));
        void *file_buffer = calloc(result.content_size, sizeof(u8));
        if(SDL_RWread(io, file_buffer, 1, result.content_size))
        {
            // TODO(kaelan): Probably need to do some SDL errors here.
            printf("ERROR: Could not read file!");
        }
        result.contents = file_buffer;
    }
    SDL_RWclose(io);
    return result;
}

// Returns all of the ROM file names from the data/ROMs directory via the directory_string_array out parameter.
// Only files with a .gb extension will be returned.
// TODO(kaelan): Should probably create a max size for this. I don't necessarily know what that should be, but
//               for now, this is the only transient storage type, and that memory pool is very large.
DEBUG_PLATFORM_GET_ROM_DIRECTORY(DEBUGPlatformGetROMDirectory) {
    char directory[] = "../data/ROMs/";
    // Iterates through all files in the directory?
    int string_index = 0;
    for(const auto &entry : std::filesystem::directory_iterator(directory))
    {
        std::string temp = entry.path().generic_string();
        size_t ext_idx = temp.find(".gb");
        if(ext_idx != (size_t) -1)
        {
            temp = temp.substr(13);
            strcpy((char *)string_array->strings[string_index].value, temp.c_str());
            ++string_index;
        }
        else
        {
            // Not .gb.
        }
    }
    string_array->size = string_index;
}
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile) {
    bool32 result = false;

    return result;
}
internal void
SDLProcessKeyboardEvent(nenjin_button_state *new_state, bool32 is_down) {
    if(is_down != new_state->ended_down)
    {
        new_state->ended_down = is_down;
        ++new_state->half_transition_count;
    }
}
internal void
SDLProcessEventQueue(SDL_Event *event, nenjin_controller_input *keyboard_controller) {
    switch (event->type)
    {
        case SDL_QUIT:
        {
            global_alive = false;
        } break;
        default:
        break;
    }
    bool32 is_down = (event->key.state == SDL_PRESSED);
    bool32 was_down = (event->key.state == SDL_RELEASED);
    if(was_down != is_down)
    {
        switch (event->key.keysym.sym)
        {
        case SDLK_w:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->up, is_down);
        } break;
        case SDLK_a:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->left, is_down);
        } break;
        case SDLK_s:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->down, is_down);
        } break;
        case SDLK_d:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->right, is_down);
        } break;
        case SDLK_g:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->a, is_down);
        } break;
        case SDLK_f:
        {
            // NOTE: For some reason SDL sets 0x1000 so mask the top nibble off.
            u16 mod = (0x111 & event->key.keysym.mod);
            if(is_down && mod == KMOD_LALT)
            {
                SDLProcessKeyboardEvent(&keyboard_controller->load_rom, is_down);
            }
            else
            {
                SDLProcessKeyboardEvent(&keyboard_controller->b, is_down);
            }
        } break;
        case SDLK_j:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->start, is_down);
        } break;
        case SDLK_h:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->select, is_down);
        } break;
        case SDLK_p:
        {
            if(is_down)
            {
                keyboard_controller->pause_emulator = ! keyboard_controller->pause_emulator;
            }
        } break;
        case SDLK_r:
        {
            if(is_down)
            {
                SDLProcessKeyboardEvent(&keyboard_controller->reset, is_down);
            }
        } break;
        case SDLK_UP:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->rom_up, is_down);
        } break;
        case SDLK_DOWN:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->rom_down, is_down);

        } break;
        case SDLK_RETURN:
        {
            SDLProcessKeyboardEvent(&keyboard_controller->rom_select, is_down);
        } break;
        // TODO(kaelan): Need to finish adding all keybinds.
        default:
            break;
        }
        if(is_down)
        {
            switch(event->key.keysym.mod)
            {
                case KMOD_ALT:
                {
                    SDL_Keycode code = event->key.keysym.sym;
                    if(code == SDLK_f)
                    {
                        SDLProcessKeyboardEvent(&keyboard_controller->load_rom, is_down);
                    }
                }
                default:
                break;
            }
        }
    }
    
}
internal void
SDLGetDrawInfo(sdl_offscreen_buffer *buffer, SDL_Surface *surface) {
    // NOTE: I do not think we need to lock the surface here since we are only getting
    //       a reference to the surface.
    buffer->memory = surface->pixels;
    buffer->width = surface->w;
    buffer->height = surface->h;
    buffer->width_in_bytes = surface->pitch;
    buffer->bytes_per_pixel = buffer->width_in_bytes/buffer->width;
}
internal f32
SDLGetSecondsElapsed(std::chrono::time_point<std::chrono::high_resolution_clock> start, 
                     std::chrono::time_point<std::chrono::high_resolution_clock> end) {
    using namespace std::chrono;
    duration<f32> result = duration_cast<duration<f32>>(end-start);
    return result.count();
}
int 
main(int argc, char *argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Failed to initialize SDL2\n");
        return -1;
    }
#define SCREEN_HORIZONTAL 1280
#define SCREEN_HEIGHT 720
    // Window handle.
    SDL_Window *window = SDL_CreateWindow("GMu: SDL", SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, SCREEN_HORIZONTAL, SCREEN_HEIGHT, 0);
    // Window surface, this uses software rendering.
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    if(!surface) 
    {
        printf("Failed to get window surface");
        return -1;
    }
    nenjin_update_and_render *UpdateAndRender = NenjinUpdateAndRender;
    nenjin_draw_debug *DrawDebug = NenjinDrawDebug;
    nenjin_memory engine_memory = {};
    engine_memory.permanent_storage_size = Megabytes(16);
    engine_memory.transient_storage_size = Megabytes(48);
    engine_memory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
    engine_memory.DEBUGPlatformGetROMDirectory = DEBUGPlatformGetROMDirectory;
    engine_memory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
    engine_memory.DEBUGPlatformWriteEntireFile= DEBUGPlatformWriteEntireFile;
    // TODO(kaelan): Add DEBUG functions!
    sdl_state platform_state = {};
    platform_state.total_memory_size = engine_memory.permanent_storage_size + engine_memory.transient_storage_size;
    platform_state.engine_memory_block = calloc(platform_state.total_memory_size, sizeof(u8));
    // Fill the gloval back buffer struct.
    SDLGetDrawInfo(&global_back_buffer, surface);
    nenjin_offscreen_buffer offscreen_buffer = {};
    offscreen_buffer.memory = global_back_buffer.memory;
    offscreen_buffer.width = global_back_buffer.width;
    offscreen_buffer.height = global_back_buffer.height;
    offscreen_buffer.width_in_bytes = global_back_buffer.width_in_bytes;
    offscreen_buffer.bytes_per_pixel = global_back_buffer.bytes_per_pixel;

    engine_memory.permanent_storage = platform_state.engine_memory_block;
    engine_memory.transient_storage = ((u8 *)engine_memory.permanent_storage + engine_memory.permanent_storage_size);
    if(engine_memory.permanent_storage && engine_memory.transient_storage)
    {
        using namespace std::chrono;
        nenjin_input engine_input[2] = {};
        nenjin_input *new_input = &engine_input[0];
        nenjin_input *old_input = &engine_input[1];
        // Events for our event loop.
        SDL_Event event;
        // TODO(kaelan): Use std::chrono instead of SDLGetTick64. 
        const f32 engine_update_freq_ms = 16.74f;
        f32 target_seconds_per_frame = engine_update_freq_ms/1000.0f;
        time_point<high_resolution_clock> last_counter = high_resolution_clock::now();
        // Main loop
        // NOTE: Seems like memory is allocated via malloc and not through SDL.
        while(global_alive)
        {
            new_input->d_time_for_frame = target_seconds_per_frame;
            nenjin_controller_input *new_keyboard_controller = GetController(new_input, 0);
            nenjin_controller_input *old_keyboard_controller = GetController(old_input, 0);
            *new_keyboard_controller = {};
            new_keyboard_controller->is_connected = true;
            new_keyboard_controller->pause_emulator = old_keyboard_controller->pause_emulator;
            // Get the down state of the previous input.
            for(int button_index = 0; button_index < ArrayCount(new_keyboard_controller->buttons); ++button_index)
            {
                new_keyboard_controller->buttons[button_index].ended_down = 
                old_keyboard_controller->buttons[button_index].ended_down;
            }
            // TODO(kaelan): Add mouse?
            new_input->controllers[1].is_connected = false;
            old_input->controllers[1].is_connected = false;
            while (SDL_PollEvent(&event) > 0)
            {
                SDLProcessEventQueue(&event, new_keyboard_controller);
            }
            // Lock the buffer before writing to it.
            SDL_LockSurface(surface);
            // Update the emulator.
            UpdateAndRender(&engine_memory, engine_input, &offscreen_buffer);
            SDL_UnlockSurface(surface);
            time_point<high_resolution_clock> work_counter = high_resolution_clock::now();
            f32 work_seconds_elapsed = SDLGetSecondsElapsed(last_counter, work_counter);
            if(work_seconds_elapsed < target_seconds_per_frame)
            {
                // Sleep if we are too fast!
                // NOTE: We sleep 1 ms less so frames are locked in at the target.
                int int_sleep_ms = (int)(1000.0f*(target_seconds_per_frame-work_seconds_elapsed)-1.0f);
                u32 sleep_ms = (int_sleep_ms < 0 ? 0 : u32(int_sleep_ms));
                if(sleep_ms > 0)
                {
                    SDL_Delay(sleep_ms);
                }
                while(work_seconds_elapsed < target_seconds_per_frame)
                {
                    work_seconds_elapsed = SDLGetSecondsElapsed(last_counter, high_resolution_clock::now());
                }
            }
            time_point<high_resolution_clock> end_counter = high_resolution_clock::now();
            f32 ms_per_frame = SDLGetSecondsElapsed(last_counter, end_counter)*1000.0f;
            f32 fps = 1000.0f/ms_per_frame;
            last_counter = end_counter;
            // Debug output
            DrawDebug(&engine_memory, &offscreen_buffer, fps, ms_per_frame);
            SDL_UpdateWindowSurface(window);
            nenjin_input *temp = new_input;
            new_input = old_input;
            old_input = temp;
        }
    }
            
    return 0;
}