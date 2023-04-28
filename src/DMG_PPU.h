#ifndef GMU_DMG_PPU_H
#define GMU_DMG_PPU_H

#include <cstdint>
#include <array>
#include <queue>
#include "Pixel.h"
#include "Fetcher.h"

// Forward declare Bus
class Bus;

class DMG_PPU {
friend struct Fetcher;
public:
    DMG_PPU();
    ~DMG_PPU() = default;

    // Communication with the main bus
    bool cpu_write(uint16_t addr, uint8_t data);
    bool cpu_read(uint16_t addr, uint8_t &data);
    // No need for ppu_r/w functions since there is no PPU bus

    // Allow cpu
    bool cpu_access = false;
    // Connect the Bus to the PPU
    void connect_bus(Bus *p_bus) { bus = p_bus; }

    // Clock function
    void clock();

    // An enum that contains the different states of the PPU
    enum PPUState {
        OAMSearch,
        PixelTransfer,
        HBlank,
        VBlank
    } state;

    // Color palettes
    uint8_t bgp = 0;
    uint8_t obp0 = 0;
    uint8_t obp1 = 0;

    // Convert color to match with palette
    static uint8_t map_color(uint8_t color, uint8_t palette);

    // Set to true if the frame has been completed
    bool frame_complete = false;

    // Current scanline
    // uint32_t to make it easier to draw to the debug viewport
    uint32_t ly = 0;
    // count of pixels pushed to the screen
    uint8_t pixel_count = 0;
    // X and Y position on the current scanline
    uint8_t pixel_x = 0;
    uint8_t pixel_y = 0;


    // LYC register
    uint8_t lyc = 0;

private:
    // This will act as the pixel FIFO fetcher
    Fetcher fetch;
    // Used in the pixel transfer state
    uint8_t tile_line = 0;
    uint16_t tilemap_row_addr = 0x0000;
    uint16_t tiledata_addr = 0x0000;
    bool signed_mode = false;
    // For when the window is on
    uint8_t win_ly = 0;
    bool window_draw = false;
    // For when the window is on, and there is a collision between the BG and window during a scanline
    bool win_collision = false;
    uint8_t win_collision_offset = 0;

    // Bus pointer
    Bus *bus = nullptr;
    // Initialize 8 KiB VRAM
    std::array<uint8_t, 8 * 1024> vram = {};
    // Initialize OAM
    std::array<uint8_t, 160> oam = {};

    // Clock count
    uint32_t clock_count = 0;

    // Display registers

    // LCD control register - top to bottom bit 0 - 7
    union lcdc_register {
        struct {
            uint8_t bg_window_enable        : 1;
            uint8_t obj_enable              : 1;
            uint8_t obj_size                : 1;
            uint8_t bg_tile_map_area        : 1;
            uint8_t bg_win_tile_data_area   : 1;
            uint8_t win_enable              : 1;
            uint8_t win_tile_map_area       : 1;
            uint8_t lcd_ppu_enable          : 1;
        };
        uint8_t data;
    } lcdc;

    // Stat register - top to bottom bit 0 - 7
    union stat_register {
        struct {
            uint8_t mode_flag      : 2;    // Read only
            uint8_t lyc_ly_flag    : 1;    // Read only
            uint8_t hblank_int_src : 1;
            uint8_t vblank_int_src : 1;
            uint8_t oam_int_src    : 1;
            uint8_t lyc_int_src    : 1;
            uint8_t unused         : 1;
        };
        uint8_t data;
    } stat;

    // Scroll registers: Y,X
    uint8_t scx = 0;
    uint8_t scy = 0;
    // Window registers
    uint8_t wx = 0;
    uint8_t wy = 0;

};


#endif //GMU_DMG_PPU_H
