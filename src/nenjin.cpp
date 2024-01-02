// TODO(kaelan): Remove all std library dependencies.
// TODO(kaelan): Implement proper memory allocator.

// FIXME(kaelan): LOTS OF COMPILE WARNINGS!!
// FIXME(kaelan): Program state is all over the place!
// FIXME(kaelan): All dynamic memory needs to be processed through memory arenas

// NOTE: Any header file that depends on iostream will need to be included before nenjin.h, since this file uses the internal keyword.

// TODO(kaelan): IDK how this just works, but it seems suspicous...
// NOTE: Things with the emualtor are bad, but working on getting
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

// FIXME: USES NEW KEYWORD!!!! FIX THIS
// TODO(kaelan): Need to use a memory arena for this! Rework cartridge class.
// TODO(kaelan): Change these functions to take the cartridge and bus instead of the entire state.
internal void
LoadCartridge(nenjin_state *state, char *file_name) {
    state->gb_cart = std::make_shared<Cartridge>(file_name);
}
internal void
InitializeGameBoy(nenjin_state *state) {
    if(state->gb_cart)
    {
        Bus *gb = &state->game_boy_bus;
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
struct gb_color_palette
{
    nenjin_color index_0;
    nenjin_color index_1;
    nenjin_color index_2;
    nenjin_color index_3;
};
internal u32
NenjinColorToU32(nenjin_color *color) {
    f32 a = color->alpha;
    f32 r = color->red;
    f32 g = color->green;
    f32 b = color->blue;
    u32 result = 0;
    // De-normalize.
    result = (u32)(RoundFloat32ToU32(a * 255.0f) << 24 |
                   RoundFloat32ToU32(r * 255.0f) << 16 | 
				   RoundFloat32ToU32(g * 255.0f) << 8 | 
				   RoundFloat32ToU32(b * 255.0f) << 0);
    return result;
}
// TODO(kaelan): Create a DrawPixel funciton?
// TODO(kaelan): Need to scale the render up!
internal void
DrawGameBoyScreen(nenjin_offscreen_buffer *buffer, Bus *gb, gb_color_palette *palette) {
    // GameBoy has 4 colors
    // 0x00 White 0x01 Light gray 0x02 Dark gray 0x03 Black
    // TODO(kaelan): Create a color palette for these?
    // NOTE: GameBoy screen size is 160x144 pixles
    u32 screen_width = 160;
    u32 screen_height = 144;
    u8 *dest_row = (u8*)buffer->memory;
    enum color_index
    {
        WHITE, LIGHT_GRAY, DARK_GRAY, BLACK
    };
    for(s32 y = 0; y < screen_height; ++y)
    {
        u32 *dest = (u32 *)dest_row;
        for(s32 x = 0; x < screen_width; ++x)
        {
            // Grab the color index from the screen memory.
            u32 color_index = gb->screen[x + y*screen_width];
            nenjin_color color = {};
            // Pick the pixel color based on the passed color palette.
            switch(color_index)
            {
                case WHITE:
                {
                    color = palette->index_0;
                } break;
                case LIGHT_GRAY:
                {
                    color = palette->index_1;
                } break;
                case DARK_GRAY:
                {
                    color = palette->index_2;
                } break;
                case BLACK:
                {
                    color = palette->index_3;
                } break;
            }
            // Write the pixel to memory.
            *dest++ = NenjinColorToU32(&color);
        }
        dest_row += buffer->width_in_bytes;
    }


#if 0
 for(uint32_t i = 0; i < p_bus->screen.size(); i++) {
            // GB only has 4 colors
            // Screen size is 160 x 144 pixels
            switch (p_bus->screen[i]) {
                // White
                case 0x00:
                    // Set render color to white
                    SDL_SetRenderDrawColor(p_window->p_renderer, 255, 255, 255, 255);
                    // Set the render scale
                    SDL_RenderSetScale(p_window->p_renderer, 4, 4);
                    break;
                case 0x01:
                    // Set render color to gray
                    SDL_SetRenderDrawColor(p_window->p_renderer, 169, 169, 169, 255);
                    // Set the render scale
                    SDL_RenderSetScale(p_window->p_renderer, 4, 4);
                    break;
                case 0x02:
                    // Set render color to dark gray
                    SDL_SetRenderDrawColor(p_window->p_renderer, 84, 84, 84, 255);
                    // Set the render scale
                    SDL_RenderSetScale(p_window->p_renderer, 4, 4);
                    break;
                case 0x03:
                    // Set render color to black
                    SDL_SetRenderDrawColor(p_window->p_renderer, 0, 0, 0, 255);
                    // Set the render scale
                    SDL_RenderSetScale(p_window->p_renderer, 4, 4);
                    break;
                default:
                    break;
            }
            // Draw the pixel
            SDL_RenderDrawPoint(p_window->p_renderer, (x), (y));
            // Reset the scale
            SDL_RenderSetScale(p_window->p_renderer, 1, 1);
            // Change x and y
            x++;
            if(x == 163) {
                y++;
                x = 3;
            }

        }
#endif
}

extern "C"
NENJIN_UPDATE_AND_RENDER(NenjinUpdateAndRender) {
    // nenjin_state acts as the structure for the permanent storage.
    Assert((sizeof(nenjin_state) <= memory->permanent_storage_size));
    nenjin_state *emulator_state = (nenjin_state *)memory->permanent_storage;
    if(!memory->is_initialized)
    {
        emulator_state->test_txt = DEBUGLoadBMP(thread, memory->DEBUGPlatformReadEntireFile, "test_text.bmp");
        LoadCartridge(emulator_state, "./ROMs/gb_snek.gb");
        InitializeGameBoy(emulator_state);
    }
    //Clear screen to black
    ClearBackBufferToBlack(buffer);
    // Draw test text to the center of the window.
    DrawBitmap(buffer, &emulator_state->test_txt, 1280.0f/2.0f, 720.0f/2.0f, 319, 100);
    gb_color_palette palette;
    palette.index_0 = {1.0f, 1.0f, 1.0f, 1.0f};
    palette.index_1 = {1.0f, 0.66f, 0.66f, 0.66f};
    palette.index_2 = {1.0f, 0.33f, 0.33f, 0.33f};
    palette.index_3 = {0.0f, 0.0f, 0.0f, 0.0f};
    DrawGameBoyScreen(buffer, &emulator_state->game_boy_bus, &palette);
}