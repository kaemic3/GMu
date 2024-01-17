// TODO(kaelan): Remove all std library dependencies.
// TODO(kaelan): Implement proper memory allocator.

// FIXME(kaelan): Program state is all over the place!
// FIXME(kaelan): All dynamic memory needs to be processed through memory arenas.
// NOTE: Any header file that depends on iostream will need to be included before nenjin.h, since this file uses the internal keyword.

// TODO(kaelan): IDK how this just works, but it seems suspicous...
// TODO(kaelan): Need to add support for re-sizeable arrays? Also need to create a queue/deque so I can remove stl.
// TODO(kaelan): Tetris still has the missing block bug...
//               This bug occurs because of the way that tetris uses hblank to update vram when tilesets are 
//               swapped from sprites to bg tiles.
#include "Bus.h"
#include "Bus.cpp"
#include "SM83.cpp"
#include "FG_Fetcher.cpp"
#include "BG_Fetcher.cpp"
#include "Sprite.cpp"
#include "DMG_PPU.cpp"
#include "Mapper.cpp"
#include "Mapper_00.cpp"
#include "Mapper_01.cpp"
#include "nenjin.h"
#include "nenjin_render.cpp"
#include "nenjin_string.h"
#include "Cartridge.cpp"
// STB TTF
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
// TODO(kaelan): Implement memory allocation, change stb's allocator to it.

// FIXME: USES ALLOCATED MEMORY FROM STL.
// TODO(kaelan): Need to use a memory arena for this! Rework cartridge class.
// TODO(kaelan): Change these functions to take the cartridge and bus instead of the entire state.
internal void
LoadCartridge(nenjin_state *state, char *file_name) {
    // Free the existing cartridge.
    if(state->gb_cart)
    {
        state->gb_cart->~Cartridge();
        memset(state->gb_cart, 0, sizeof(Cartridge));
        free(state->gb_cart);
        state->game_boy_bus->cart = 0;
    }
    state->gb_cart = (Cartridge *)calloc(1, sizeof(Cartridge));
    new (state->gb_cart) Cartridge();
    Assert(state->gb_cart->cart_rom.size() > 0);
}
internal void
CreateCartridge(nenjin_state *state, debug_platform_read_entire_file *DEBUGPlatformReadEntireFile, 
                debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory, char *file_name) {
    if(!state->gb_cart)
    {   
        #if 0
        state->gb_cart = (Cartridge *)calloc(1, sizeof(Cartridge));
        new (state->gb_cart) Cartridge();
        #endif
        state->gb_cart = new Cartridge();
    }
    state->gb_cart->CreateCartridge(&state->game_boy_arena, DEBUGPlatformReadEntireFile, DEBUGPlatformFreeFileMemory, file_name);
}
#if 0
internal void
CreateCartridge(nenjin_state *state, debug_platform_read_entire_file *DEBUGPlatformReadEntireFile, 
                debug_platform_find_rom_file *DEBUGPlatformFindRomFile,
                debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory) {
    #if 0
    if(state->gb_cart)
    {
        state->gb_cart->~Cartridge();
        free(state->gb_cart);

    }
    state->gb_cart = (Cartridge *)calloc(1, sizeof(Cartridge));
    new (state->gb_cart) Cartridge();
    #endif
    state->gb_cart->CreateCartridge(&state->game_boy_arena, DEBUGPlatformReadEntireFile, DEBUGPlatformFindRomFile, DEBUGPlatformFreeFileMemory);

}
#endif

// TODO(kaelan): Remove shared_ptr.
// TODO(kaelan): This function should return void, but becuase the Bus class is not setup for memory arenas, it has to be this way for now.
internal Bus * 
InitializeGameBoy(memory_arena *gb_arena, Cartridge *gb_cart) {
    Bus *gb = 0;
    if(gb_cart)
    {
        // TODO(kaelan): This seems like a bad idea, the Bus class is not setup for memory arenas, so it looks super sketchy here.
        // Still, there are no other allocations to the Bus class, and to use pre allocated memory, this is the best for now.
        u32 size = sizeof(Bus);
        gb_arena->base_ptr = (u8 *)PushSize(gb_arena, sizeof(Bus));
        gb = new(gb_arena->base_ptr) Bus();
        gb->insert_cartridge(gb_cart);
        // TODO(kaelan): Need to load the boot rom
        // For now set pc to 0x0100 and sp to 0xfffe
        gb->cpu.pc = 0x0100;
        gb->cpu.debug_pc = 0x0100;
        gb->cpu.sp = 0xfffe;
        gb->if_reg.data = 0xe1;
    }
    else
    {

    }
    return gb;
}
internal void
InsertCartridge(Bus *gb, Cartridge *gb_cart) {
    gb->insert_cartridge(gb_cart);
}
internal void
ResetGameBoy(nenjin_state *state) {
    state->game_boy_bus->reset();
    state->game_boy_bus->cpu.pc = 0x0100;
    state->game_boy_bus->cpu.debug_pc = 0x0100;
    state->game_boy_bus->cpu.sp = 0xfffe;
    state->game_boy_bus->if_reg.data = 0xe1;
}
internal void
GenerateGameBoyFrame(Bus *gb, bool32 run_emulator) {
    if(run_emulator)
    {
        do {
            gb->clock();
        }while(!gb->ppu.get_frame_state());
    }
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
// TODO(kaelan): Need to create a render queue for things to be drawn.
// TODO(kaelan): Where does this function belong?
// TODO(kaelan): Add support for different font colors?
// NOTE: This is an asset generator, so maybe create an asset file?
// NOTE: I know this generates stuff that is not actually char data, but IDK if have empty spaces is better??
internal void
GenerateFontTable(memory_arena *arena, font_bitmap *font_map, char *file_name, f32 font_size, nenjin_color font_color, debug_platform_read_entire_file ReadEntireFile) {
    // Generate ascii chars!
    // https://www.cs.cmu.edu/~pattis/15-1XX/common/handouts/ascii.html 
    stbtt_fontinfo font;
    u8 *mono_bitmap;
    debug_read_file_result read_result = ReadEntireFile(file_name);
    u8* ttf_buffer = (u8 *)read_result.contents;
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0));
    u32 color = NenjinColorToU32(&font_color);
    u32 red = (color & 0x00ff0000) >> 16;
    u32 green = (color & 0x0000ff00) >> 8;
    u32 blue = (color & 0x000000ff) >> 0;
    for(u32 char_index = 0; char_index < 127; ++char_index)
    {
        s32 width, height, x_offset, y_offset;
        mono_bitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, font_size), char_index,
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
                           (red << 16) |
                           (green << 8) | 
                           (blue << 0));
            }
            dest_row -= bitmap.width_in_bytes;
        }
        font_bitmap result = {};
        result.bitmap = bitmap;
        result.x_offset = x_offset;
        result.y_offset = y_offset;
        result.font_size = font_size;
        stbtt_FreeBitmap(mono_bitmap, 0);
        font_map[char_index] = result;
    }
}
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
internal void
DrawROMSelectMenu(nenjin_offscreen_buffer *buffer, nenjin_memory *memory, font_maps *font_maps, directory_string_array *directory_struct, s32 selected_rom) {
    f32 left_x = SCREEN_WIDTH/2.0f - 400.0f;
    f32 top_y = 50.0f;
    f32 right_x = SCREEN_WIDTH/2.0f + 400.0f;
    f32 bottom_y = 500.0f;
    f32 padding_y = 25.0f;
    f32 first_rom_y = top_y+padding_y*3;
    // Backdrop for the ROM menu.
    DrawRectangle(buffer, left_x, top_y, right_x, bottom_y, 0.34f, 0.33f, 0.33f);

    DrawString(buffer, (font_bitmap *)font_maps->font_large_pink, left_x + 50.0f, top_y + padding_y*2, "ROMS:");
    // NOTE: Max number of ROMs on screen at one time is 15.
    s32 rom_count = directory_struct->size;
    s32 base_index = 0;

    if(rom_count >= 15)
    {
        rom_count = 15;
    }
    s32 last_index = rom_count;
    if((selected_rom + 1) / 15 > 0)
    {
        base_index = base_index + 15*((selected_rom + 1)/15)-1;
        last_index = base_index + rom_count;
    }
    s32 draw_offset = 0;
    for(s32 index = base_index; index < last_index; ++index)
    {
        if(selected_rom == index)
        {
            DrawString(buffer, (font_bitmap *)font_maps->font_small_pink, left_x + 50.0f, top_y + padding_y*(3 + draw_offset), directory_struct->strings[index].value);
        }
        else
        {
            DrawString(buffer, (font_bitmap *)font_maps->font_small_white, left_x + 50.0f, top_y + padding_y*(3 + draw_offset), directory_struct->strings[index].value);
        }
        ++draw_offset;
    }
}

extern "C"
NENJIN_UPDATE_AND_RENDER(NenjinUpdateAndRender) {
    // nenjin_state acts as the structure for the permanent storage.
    Assert((sizeof(nenjin_state) <= memory->permanent_storage_size));
    nenjin_state *emulator_state = (nenjin_state *)memory->permanent_storage;

    // TODO(kaelan): Create proper allocation for this.
    directory_string *directory_array = (directory_string *)memory->transient_storage;
    Assert((sizeof(directory_array) <= memory->transient_storage_size));
    if(!memory->is_initialized)
    {
        emulator_state->directory_struct = {};
        emulator_state->directory_struct.strings = directory_array;
        emulator_state->run_emulator = false;
        emulator_state->show_rom_select = false;
        emulator_state->selected_rom = 0;
        nenjin_controller_input *controller = GetController(input, 0);
        controller->pause_emulator = true;
        // Allocate 4 MiB for bitmaps.
        // TODO(kaelan): Change this later after checking how much memory we actually use.
        InitializeArena(&emulator_state->bitmap_arena, Megabytes(4),
                        (u8 *)memory->permanent_storage + sizeof(nenjin_state));
        emulator_state->font_color_pink = {1.0f, 0.95f, 0.51f, 0.78f};
        emulator_state->font_color_white = {1.0f, 1.0f, 1.0f, 1.0f};
        // Generate pink large fonts.
        GenerateFontTable(&emulator_state->bitmap_arena, (font_bitmap *)emulator_state->font_maps.font_large_pink, 
                          "../Fonts/amstrad_cpc464.ttf", 24.0f, emulator_state->font_color_pink, memory->DEBUGPlatformReadEntireFile);
        // Generate white large fonts.
        GenerateFontTable(&emulator_state->bitmap_arena, (font_bitmap *)emulator_state->font_maps.font_large_white, 
                          "../Fonts/amstrad_cpc464.ttf", 24.0f, emulator_state->font_color_white, memory->DEBUGPlatformReadEntireFile);
        // Generate white small fonts. 
        GenerateFontTable(&emulator_state->bitmap_arena, (font_bitmap *)emulator_state->font_maps.font_small_white, 
                          "../Fonts/amstrad_cpc464.ttf", 16.0f, emulator_state->font_color_white, memory->DEBUGPlatformReadEntireFile);
        // Generate pink small fonts. 
        GenerateFontTable(&emulator_state->bitmap_arena, (font_bitmap *)emulator_state->font_maps.font_small_pink, 
                          "../Fonts/amstrad_cpc464.ttf", 16.0f, emulator_state->font_color_pink, memory->DEBUGPlatformReadEntireFile);

        InitializeArena(&emulator_state->game_boy_arena, sizeof(Bus), 
                        (u8 *)memory->permanent_storage + (sizeof(nenjin_state) + emulator_state->bitmap_arena.size));
        InitializeArena(&emulator_state->cartridge_arena, Kilobytes(3), 
                        (u8 *)memory->permanent_storage + (sizeof(nenjin_state)) + emulator_state->bitmap_arena.size + 
                        emulator_state->game_boy_arena.size);
        CreateCartridge(emulator_state, memory->DEBUGPlatformReadEntireFile, memory->DEBUGPlatformFreeFileMemory,
                        "../data/ROMs/Zelda.gb");
                        
        emulator_state->game_boy_bus = InitializeGameBoy(&emulator_state->game_boy_arena, emulator_state->gb_cart);
        memory->is_initialized = true;
    }
    // Clear screen to black.
    ClearBackBufferToBlack(buffer);
    u32 screen_width = 1280;
    u32 screen_height = 720;
    f32 text_width_offset = 660.0f;
    // Write my name to the screen.
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, screen_width/2.0f + 15.0f, 
               screen_height/2.0f + 64.0f, "Kaelan :3");
    // TODO(kaelan): Need to figure out if these need to be put in nenjin_state, also probably want to move the init
    //               code for the register text into a function.
    // TODO(kaelan): Looking at the stuff below, could create an intialization function that
    //               gets called and assigns all of the strings. Move the strings into nenjin_state, 
    //               where they would exist in our permanent storage. 
    // TODO(kaelan): Consider the trasient storage space, what will this be used for? Should there be a 
    //               distinction in this program?
    // NOTE: The size of every string remains static. However, when toggling the text on/off the screen, 
    //       the memory could be freed. Does this even matter? The size of these strings is puny, and allocating
    //       would most likely have more overhead in comparison to just leaving them assigned in the permanent store.
    //      
    //       For this program, I feel that every string should be static. Even in the case of showing the ROM name
    //       somewhere in the window, we should be able to assign a static max size, and truncate if needed to avoid any
    //       allocations. But, if I am already going to make a local allocator, then shouldn't it be as memory
    //       efficient as possible, with the speed as well?
#define U8_REG_STRING_SIZE 5
#define REGISTER_GAP 120.0f
    // DEBUG text code.
    char a_hex[3] = "";
    char a_text[3] = "A:";
    char a_reg_text[U8_REG_STRING_SIZE] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cpu.a_reg, a_hex);
    CatString(S32StringLength(a_text), a_text, S32StringLength(a_hex), a_hex, U8_REG_STRING_SIZE, a_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset, 50.0f, a_reg_text);

    char f_hex[3] = "";
    char f_text[3] = "F:";
    char f_reg_text[U8_REG_STRING_SIZE] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cpu.f_reg, f_hex);
    CatString(S32StringLength(f_text), f_text, S32StringLength(f_hex), f_hex, U8_REG_STRING_SIZE, f_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset + REGISTER_GAP, 50.0f, f_reg_text);

    char b_hex[3] = "";
    char b_text[3] = "B:";
    char b_reg_text[U8_REG_STRING_SIZE] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cpu.b_reg, b_hex);
    CatString(S32StringLength(b_text), b_text, S32StringLength(b_hex), b_hex, U8_REG_STRING_SIZE, b_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset, 100.0f, b_reg_text);

    char c_hex[3] = "";
    char c_text[3] = "C:";
    char c_reg_text[U8_REG_STRING_SIZE] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cpu.c_reg, c_hex);
    CatString(S32StringLength(c_text), c_text, S32StringLength(c_hex), c_hex, U8_REG_STRING_SIZE, c_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset + REGISTER_GAP, 100.0f, c_reg_text);


    char d_hex[3] = "";
    char d_text[3] = "D:";
    char d_reg_text[U8_REG_STRING_SIZE] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cpu.d_reg, d_hex);
    CatString(S32StringLength(d_text), d_text, S32StringLength(d_hex), d_hex, U8_REG_STRING_SIZE, d_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset, 150.0f, d_reg_text);

    char e_hex[3] = "";
    char e_text[3] = "E:";
    char e_reg_text[U8_REG_STRING_SIZE] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cpu.e_reg, e_hex);
    CatString(S32StringLength(e_text), e_text, S32StringLength(e_hex), e_hex, U8_REG_STRING_SIZE, e_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset + REGISTER_GAP, 150.0f, e_reg_text);

    char h_hex[3] = "";
    char h_text[3] = "H:";
    char h_reg_text[U8_REG_STRING_SIZE] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cpu.h_reg, h_hex);
    CatString(S32StringLength(h_text), h_text, S32StringLength(h_hex), h_hex, U8_REG_STRING_SIZE, h_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset, 200.0f, h_reg_text);

    char l_hex[3] = "";
    char l_text[3] = "L:";
    char l_reg_text[U8_REG_STRING_SIZE] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cpu.l_reg, l_hex);
    CatString(S32StringLength(l_text), l_text, S32StringLength(l_hex), l_hex, U8_REG_STRING_SIZE, l_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset + REGISTER_GAP, 200.0f, l_reg_text);

    #define U16_REG_STRING_SIZE 8

    char pc_hex[5] = "";
    char pc_text[4] = "PC:";
    char pc_reg_text[U16_REG_STRING_SIZE] = "";
    ToHexStringU16(emulator_state->game_boy_bus->cpu.debug_pc, pc_hex); // NOTE: It is important that cpu.debug_pc is used here!
    CatString(S32StringLength(pc_text), pc_text, S32StringLength(pc_hex), pc_hex, U16_REG_STRING_SIZE, pc_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset, 250.0f, pc_reg_text);

    char sp_hex[5] = "";
    char sp_text[4] = "SP:";
    char sp_reg_text[U16_REG_STRING_SIZE] = "";
    ToHexStringU16(emulator_state->game_boy_bus->cpu.sp, sp_hex);
    CatString(S32StringLength(sp_text), sp_text, S32StringLength(sp_hex), sp_hex, U16_REG_STRING_SIZE, sp_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset, 300.0f, sp_reg_text);

    // Flag register expanded.
    char flags_binary[9] = "";
    char flags_text[7] = "Flags:";
    char flags_reg_text[15] = "";
    ToFlagStringU8(emulator_state->game_boy_bus->cpu.f_reg, flags_binary);
    CatString(S32StringLength(flags_text), flags_text, S32StringLength(flags_binary), flags_binary, 15, flags_reg_text);
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset, 350.0f, flags_reg_text);
    char flags_label[9] = "ZNHC----";
    DrawString(buffer, (font_bitmap *)emulator_state->font_maps.font_large_pink, text_width_offset + 6.0f*24.0f, 380.0f, flags_label);

    // Cartridge bank
    char bank_hex[3] = "";
    char bank_text[6] = "Bank:";
    char bank_reg_text[8] = "";
    ToHexStringU8(emulator_state->game_boy_bus->cart->p_mapper->get_current_rom_bank(), bank_hex);
    CatString(S32StringLength(bank_text), bank_text, S32StringLength(bank_hex), bank_hex, 8, bank_reg_text);
    DrawString(buffer, emulator_state->font_maps.font_large_pink, text_width_offset + REGISTER_GAP*2, 50.0f, bank_reg_text);

    gb_color_palette palette;
    palette.index_0 = {1.0f, 1.0f, 1.0f, 1.0f};
    palette.index_1 = {1.0f, 0.66f, 0.66f, 0.66f};
    palette.index_2 = {1.0f, 0.33f, 0.33f, 0.33f};
    palette.index_3 = {0.0f, 0.0f, 0.0f, 0.0f};
    // Input handling
    // TODO(kaelan): Maybe move this to a function? This is a lot of code to look at.
    nenjin_controller_input *controller = GetController(input, 0);
    if(controller->load_rom.ended_down)
    {
        controller->load_rom.ended_down = false;
        emulator_state->show_rom_select = !emulator_state->show_rom_select;
        memory->DEBUGPlatformGetROMDirectory(&emulator_state->directory_struct);
    }
    if(emulator_state->show_rom_select)
    {
        if(controller->rom_up.ended_down)
        {
            --emulator_state->selected_rom;
            controller->rom_up.ended_down = false;
            if(emulator_state->selected_rom < 0)
            {
                emulator_state->selected_rom = 0;
            }
        }
        if(controller->rom_down.ended_down)
        {
            controller->rom_down.ended_down = false;
            ++emulator_state->selected_rom;
            if(emulator_state->selected_rom > emulator_state->directory_struct.size-1)
            {
                emulator_state->selected_rom = emulator_state->directory_struct.size-1;
            }
        }
        if(controller->rom_select.ended_down)
        {
            controller->pause_emulator = true;
            emulator_state->run_emulator = false;
            controller->rom_select.ended_down = false;
            ResetGameBoy(emulator_state);
            char *file_name = (char *)emulator_state->directory_struct.strings[emulator_state->selected_rom].value;
            char file_directory[32] = "../data/ROMs/";
            char file_location[260] = "";
            CatString(S32StringLength((char *)file_directory), (char*)file_directory, S32StringLength(file_name), file_name, 
                        260, (char *)file_location);
            CreateCartridge(emulator_state, memory->DEBUGPlatformReadEntireFile, memory->DEBUGPlatformFreeFileMemory,
                        (char *)file_location);
            InsertCartridge(emulator_state->game_boy_bus, emulator_state->gb_cart);
            emulator_state->show_rom_select = false;

            //DrawGameBoyScreen(buffer, emulator_state->game_boy_bus, &palette, 4);
            controller->pause_emulator = false;
            emulator_state->run_emulator = true;

        }
    }
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

    if(controller->load_rom_abs.ended_down)
    {
        controller->load_rom_abs.ended_down = false;
        controller->pause_emulator = true;
        emulator_state->run_emulator = false;
        memory->DEBUGPlatformGetROMDirectory(&emulator_state->directory_struct);
    }
    if(controller->reset.ended_down)
    {
        controller->reset.ended_down = false;
        emulator_state->game_boy_bus->reset();
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
    if(emulator_state->show_rom_select)
    {
        DrawROMSelectMenu(buffer, memory, &emulator_state->font_maps, &emulator_state->directory_struct, emulator_state->selected_rom);
    }
}
extern "C"
NENJIN_DRAW_DEBUG(NenjinDrawDebug) {
    Assert(memory->permanent_storage_size >= sizeof(nenjin_state));
    nenjin_state *state = (nenjin_state *)memory->permanent_storage;
    char fps_value[256] = "";
    char fps_string[] = "fps";
    char ms_value[256] = "";
    char ms_string[] = "ms";
    _snprintf_s(fps_value, sizeof(fps_value), "%.02f", fps);
    _snprintf_s(ms_value, sizeof(ms_value), "%.02f", f_time);
    DrawString(buffer, (font_bitmap *)state->font_maps.font_small_pink, 0.0f, 600.0f, fps_string);
    DrawString(buffer, (font_bitmap *)state->font_maps.font_small_white, 16.0f*3, 600.0f, fps_value);
    DrawString(buffer, (font_bitmap *)state->font_maps.font_small_pink, 16.0f*9, 600.0f, ms_string);
    DrawString(buffer, (font_bitmap *)state->font_maps.font_small_white, 16.0f*11, 600.0f, ms_value);
}

    // This kinda works, but has bugs with Mario for some reason.
    // TODO(kaelan): Figure out why the heck this does not work with Mario.
    #if 0
    if(controller->load_rom.ended_down)
    {
        controller->load_rom.ended_down = false;
        //LoadCartridge(emulator_state, "../data/ROMs/Mario.gb");
        controller->pause_emulator = true;
        emulator_state->run_emulator = false;
        ResetGameBoy(emulator_state);
        #if 1
        CreateCartridge(emulator_state, memory->DEBUGPlatformReadEntireFile, memory->DEBUGPlatformFindROMFile,
                        memory->DEBUGPlatformFreeFileMemory);
        #else
        CreateCartridge(emulator_state, memory->DEBUGPlatformReadEntireFile, memory->DEBUGPlatformFreeFileMemory,
                        "../data/ROMs/Mario.gb");
        #endif
        InsertCartridge(emulator_state->game_boy_bus, emulator_state->gb_cart);
    }
    if(controller->load_rom.ended_down)
    {
        controller->load_rom.ended_down = false;
        emulator_state->show_rom_select = !emulator_state->show_rom_select;
    }
    #endif