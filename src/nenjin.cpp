// TODO(kaelan): Remove all std library dependencies.
// TODO(kaelan): Implement proper memory allocator.

// FIXME(kaelan): LOTS OF COMPILE WARNINGS!!
// FIXME(kaelan): Program state is all over the place!
// FIXME(kaelan): All dynamic memory needs to be processed through memory arenas

// NOTE: Any header file that depends on iostream will need to be included before nenjin.h, since this file uses the internal keyword.

// TODO(kaelan): IDK how this just works, but it seems suspicous...
// NOTE: Things with the emualtor are bad, but working on getting it running first!
#include "Bus.h"
#include "Bus.cpp"
#include "SM83.cpp"
#include "FG_Fetcher.cpp"
#include "BG_Fetcher.cpp"
#include "Sprite.cpp"
#include "DMG_PPU.cpp"
#include "Cartridge.cpp"
#include "Mapper.cpp"
#include "Mapper_00.cpp"
#include "Mapper_01.cpp"
#include "nenjin.h"
#include "nenjin_render.cpp"
// STB TTF
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
// TODO(kaelan): Implement memory allocation, change stb's allocator to it.

// FIXME: USES ALLOCATED MEMORY FROM STL.
// TODO(kaelan): Need to use a memory arena for this! Rework cartridge class.
// TODO(kaelan): Change these functions to take the cartridge and bus instead of the entire state.
internal void
// TODO(kaelan): For some reason, make_shared is not working when the program is not being run in the debugger???
LoadCartridge(nenjin_state *state, char *file_name) {
    state->gb_cart = std::make_shared<Cartridge>(file_name);
    Assert(state->gb_cart->cart_rom.size() > 0);
}
internal void
InitializeGameBoy(nenjin_state *state) {
    if(state->gb_cart)
    {
        // FIXME: This uses "new" which is not what we want at all! It should use a memory arena.
        // TODO(kaelan): Will look into fixing this after the scaling algorithm gets faster.
        state->game_boy_bus = new Bus();
        Bus *gb = state->game_boy_bus;
        gb->insert_cartridge(state->gb_cart);
        // TODO(kaelan): Need to load the boot rom
        // For now set pc to 0x0100 and sp to 0xfffe
        gb->cpu.pc = 0x0100;
        gb->cpu.sp = 0xfffe;
        gb->if_reg.data = 0xe1;
    }
    else
    {

    }

}
// TODO(kaelan): Figure out why this function crashes the exe when not loaded in the debugger.
// NOTE: This crash does not occur when building with SDL.
internal void
GenerateGameBoyFrame(Bus *gb, bool32 run_emulator) {
    do {
        gb->clock();
    }while(!gb->ppu.get_frame_state() && run_emulator);
    gb->joypad_state_change = false;
}
#define BITMAP_BYTES_PER_PIXEL 4
internal loaded_bitmap
MakeEmptyBitmap(memory_arena *arena, s32 width, s32 height, bool32 clear_to_zero = false) {
    loaded_bitmap result = {};

    result.width = width;
    result.height = height;
    result.width_in_bytes = result.width * BITMAP_BYTES_PER_PIXEL;
    size_t total_bitmap_size = width*height*BITMAP_BYTES_PER_PIXEL;

    result.memory = (u32 *)PushSize(arena, total_bitmap_size);
    // TODO(kaelan): Create a ClearBitmap function.
    #if 0
    if(clear_to_zero)
    {
        ClearBitmap(&result);
    }
    #endif
    return result;
}
// TODO(kaelan): Need to generate a font atlas that can be accessed to get a char bitmap.
// TODO(kaelan): Create a function that will draw strings to the screen.
// TODO(kaelan): Need to create a render queue for things to be drawn.
internal font_bitmap 
STBFontTest(debug_platform_read_entire_file ReadEntireFile, thread_context *thread, memory_arena *arena) {
    stbtt_fontinfo font;
    u8 *mono_bitmap;
    debug_read_file_result read_result = ReadEntireFile(thread, "../Fonts/amstrad_cpc464.ttf");
    u8 *ttf_buffer = (u8*)read_result.contents;
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
    s32 width, height, x_offset, y_offset;
    mono_bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, 32.0f), 'k', 
                                           &width, &height, &x_offset, &y_offset);
    loaded_bitmap bitmap = MakeEmptyBitmap(arena, width, height);
    u8 *dest_row = (u8 *)bitmap.memory + (height-1) * bitmap.width_in_bytes;
    u8 *source = mono_bitmap;
    for(s32 y = 0; y < height; ++y)
    {
        u32 *dest = (u32 *)dest_row;
        for(s32 x = 0; x < width; ++x)
        {
            u8 alpha = *source++;
            *dest++ = ((alpha << 24) |
                       (alpha << 16) |
                       (alpha << 8) | 
                       (alpha << 0));
        }
        dest_row -= bitmap.width_in_bytes;
    }
    font_bitmap result = {};
    result.bitmap = bitmap;
    result.x_offset = x_offset;
    result.y_offset = y_offset;
    stbtt_FreeBitmap(mono_bitmap, 0);
    return result;
}
extern "C"
NENJIN_UPDATE_AND_RENDER(NenjinUpdateAndRender) {
    // nenjin_state acts as the structure for the permanent storage.
    Assert((sizeof(nenjin_state) <= memory->permanent_storage_size));
    nenjin_state *emulator_state = (nenjin_state *)memory->permanent_storage;
    if(!memory->is_initialized)
    {
        LoadCartridge(emulator_state, "../data/ROMs/gb_snek.gb");
        InitializeGameBoy(emulator_state);
        memory->is_initialized = true;
        emulator_state->run_emulator = true;
        InitializeArena(&emulator_state->bitmap_arena, memory->permanent_storage_size - sizeof(nenjin_state),
                        (u8 *)memory->permanent_storage + sizeof(nenjin_state));
        emulator_state->test_text = STBFontTest(memory->DEBUGPlatformReadEntireFile, thread, &emulator_state->bitmap_arena);
    }

    // Clear screen to black.
    ClearBackBufferToBlack(buffer);
    // Draw a K generated with stb_ttf.
    DrawBitmap(buffer, &emulator_state->test_text.bitmap, 1000.0f, 200.0f, emulator_state->test_text.x_offset, emulator_state->test_text.y_offset);
    gb_color_palette palette;
    palette.index_0 = {1.0f, 1.0f, 1.0f, 1.0f};
    palette.index_1 = {1.0f, 0.66f, 0.66f, 0.66f};
    palette.index_2 = {1.0f, 0.33f, 0.33f, 0.33f};
    palette.index_3 = {0.0f, 0.0f, 0.0f, 0.0f};
    // Input handling
    nenjin_controller_input *controller = GetController(input, 0);
    if(controller->pause_emulator)
    {
        emulator_state->run_emulator = false;
    }
    else
    {
        emulator_state->run_emulator = true;
    }
    if(controller->up.ended_down)
    {
        emulator_state->game_boy_bus->joypad_directional &= ~(1 << 2);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    else
    {
        emulator_state->game_boy_bus->joypad_directional |= (1 << 2);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    if(controller->down.ended_down)
    {
        emulator_state->game_boy_bus->joypad_directional &= ~(1 << 3);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    else
    {
        emulator_state->game_boy_bus->joypad_directional |= (1 << 3);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    if(controller->left.ended_down)
    {
        emulator_state->game_boy_bus->joypad_directional &= ~(1 << 1);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    else
    {
        emulator_state->game_boy_bus->joypad_directional |= (1 << 1);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    if(controller->right.ended_down)
    {
        emulator_state->game_boy_bus->joypad_directional &= ~(1 << 0);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    else
    {
        emulator_state->game_boy_bus->joypad_directional |= (1 << 0);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    if(controller->a.ended_down)
    {
        emulator_state->game_boy_bus->joypad_action &= ~(1 << 0);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    else
    {
        emulator_state->game_boy_bus->joypad_action |= (1 << 0);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    if(controller->b.ended_down)
    {
        emulator_state->game_boy_bus->joypad_action &= ~(1 << 1);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    else
    {
        emulator_state->game_boy_bus->joypad_action |= (1 << 1);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    if(controller->start.ended_down)
    {
        emulator_state->game_boy_bus->joypad_action &= ~(1 << 3);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    else
    {
        emulator_state->game_boy_bus->joypad_action |= (1 << 3);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    if(controller->select.ended_down)
    {
        emulator_state->game_boy_bus->joypad_action &= ~(1 << 2);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }
    else
    {
        emulator_state->game_boy_bus->joypad_action |= (1 << 2);
        emulator_state->game_boy_bus->joypad_state_change = true;
    }

    GenerateGameBoyFrame(emulator_state->game_boy_bus, emulator_state->run_emulator);

    // TODO(kaelan): The algorithm to upscale the pixel out is better, but I feel like it can get even faster.
    // TODO(kaelan): I think that the best optimization would be to use StretchDIBits on Windows to scale up the image. 
    //               This would be by far the fastest option, and with optimizations, so far this algorithm works well enough.
    //               To do this, I need to make a callback of some kind, or add a flag to the nenjin_offscreen_buffer and 
    //               win32_offscreen_buffer structs that would scale the buffer somehow.

    // IDEA: Create a separate function that updates the GameBoy screen based on a buffer that updates in this engine update function.
    // IDEA: Create a rendering queue? Queue the gb screen as a separate buffer entirely?
    //         - Each buffer would have coordinates, size and scale??
    // NOTE: Currently, this algorithm is fast enough in O2 mode to run on my zenbook. Without O2, it runs slower than 16.74 ms.
    DrawGameBoyScreen(buffer, emulator_state->game_boy_bus, &palette, 4);
}