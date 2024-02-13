#include <SDL.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

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
    #if 0
    WIN32_FIND_DATAA data;
    // Skip the . and .. strings, from what I can find, they always show up as the first two files.
    HANDLE find = FindFirstFileA("../data/ROMs/*", &data);
    FindNextFileA(find, &data);
    s32 string_index = 0;
    while(FindNextFileA(find, &data) != 0)
    {
        char *string = data.cFileName;
        s32 string_length = StringLength(string);
        // Traverse the string to the end of the string, then get the file ext.
        for(s32 ext_index = 0; ext_index < string_length; ++ext_index)
        {
            ++string;
        }
        // Now go backwards until we see a . Store the number of chars.
        s32 ext_length = 0;
        while(*string != '.')
        {
            --string;
            ++ext_length;
        }
        // String should be at the last period now, which should be the beginning of the ext.
        // NOTE: I am not sure how long file extensions can be, but 8 seems like it would be long enough.
        // Store the ext.
        char ext[8];
        for(s32 ext_index = 0; ext_index < ext_length; ++ext_index)
        {
            // Store as lowercase!
            ext[ext_index] = (char)tolower(*++string);
        }
        // Now check the first two indicies of the ext array to see if they are gb
        if(ext[0] == 'g' && ext[1] == 'b')
        {
            // Reset the string pointer
            string = data.cFileName;
            for(s32 index = 0; index < string_length; ++index)
            {
                string_array->strings[string_index].value[index] = string[index];
            }
            // Add null term.
            string_array->strings[string_index].value[string_length] = 0;
            ++string_index;
        }
        
    }
    string_array->size = string_index;
    #endif
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
            SDLProcessKeyboardEvent(&keyboard_controller->b, is_down);
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
        // TODO(kaelan): Need to finish adding all keybinds.
        default:
            break;
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
        nenjin_input engine_input[2] = {};
        nenjin_input *new_input = &engine_input[0];
        nenjin_input *old_input = &engine_input[1];
        // Events for our event loop.
        SDL_Event event;
        // Main loop
        // NOTE: Seems like memory is allocated via malloc and not through SDL.
        while(global_alive)
        {
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
            SDL_UpdateWindowSurface(window);
            nenjin_input *temp = new_input;
            new_input = old_input;
            old_input = temp;
        }
    }
            
    return 0;
}