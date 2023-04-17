#ifndef GMU_DMG_PPU_H
#define GMU_DMG_PPU_H

#include <cstdint>
#include <array>
#include <queue>

class DMG_PPU {
public:
    DMG_PPU();
    ~DMG_PPU() = default;

    // Communication with the main bus
    bool cpu_write(uint16_t addr, uint8_t data);
    bool cpu_read(uint16_t addr, uint8_t &data);
    // No need for ppu_r/w functions since there is no PPU bus

    // Clock function
    void clock();

    // An enum that contains the different states of the PPU
    enum PPUState {
        OAMSearch,
        PixelTransfer,
        HBlank,
        VBlank
    } state;

    // Data the FIFO will need
    struct Pixel {
        uint8_t color;
        uint8_t palette;
        uint8_t bg_priority;
    };

    // Struct to represent color palettes
    struct Palette {
        char index_3 : 2;
        char index_2 : 2;
        char index_1 : 2;
        char index_0 : 2;
    } bgp, obp0, obp1;


    // Pixel FIFOS
    std::queue<Pixel> fifo_bg;
    std::queue<Pixel> fifo_obj;

    // Set to true if the frame has been completed
    bool frame_complete = false;

    // Current scanline
    // uint32_t to make it easier to draw to the debug viewport
    uint32_t ly = 0;

    // LYC register
    uint8_t lyc = 0;

    // LCD control register
    struct lcdc_register {
        char lcd_ppu_enable         : 1;
        char win_tile_map_area      : 1;
        char win_enable             : 1;
        char bg_win_tile_data_area  : 1;
        char bg_tile_map_area       : 1;
        char obj_size               : 1;
        char obj_enable             : 1;
        char bg_window_enable       : 1;
    } lcdc;

    // Stat register - may turn into an uint8_t later
    struct stat_register {
        char unused         : 1;
        char lyc_int_src    : 1;
        char oam_int_src    : 1;
        char vblank_int_src : 1;
        char hblank_int_src : 1;
        char lyc_ly_flag    : 1;    // Read only
        char mode_flag      : 2;    // Read only
    } stat;

private:
    // Initialize 8 KiB VRAM
    std::array<uint8_t, 8 * 1024> vram = {};
    // Initialize OAM
    std::array<uint8_t, 160> oam = {};

    // Clock count
    uint32_t clock_count = 0;
    // Display registers

    // Scroll registers: Y,X
    uint8_t sc_pos[2] = {0, 0};
    // Window registers
    uint8_t win_pos[2] = {0, 0};
};


#endif //GMU_DMG_PPU_H
