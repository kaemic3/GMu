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
friend class BG_Fetcher;
friend class FG_Fetcher;
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
    // Reset function
    void reset();

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
    // This will act as the pixel FIFO fetcher for the background and window
    BG_Fetcher bg_fetch;
    // Used in the pixel transfer state

    // The address to the row of the tilemap we need to grab the tile id from for the current scan line
    uint16_t tilemap_row_addr = 0x0000;
    // Where the tile data is located for the current scan line
    uint16_t tiledata_addr = 0x0000;
    // Flag is set when the tilemap selected uses signed access
    bool signed_mode = false;
    // The offset from the tilemap_row_addr to get the correct bytes for the line of pixels to fetch
    uint8_t tile_line = 0;
    // For when the window is on
    // Current window scan line
    uint8_t win_ly = 0;
    // Flag is set when the window is drawing
    bool window_draw = false;
    // For when the window is on, and there is a collision between the BG and window during a scanline
    // Flag is set when there is a collision between the BG and window on the current scan line
    bool win_collision = false;
    // Flag is set when the fetcher needs to change from the BG tilemap to the Win tilemap
    bool swap_to_win = false;
    // The X coordinate of the window with its +7 offset removed
    uint8_t win_collision_pos = 0xff;
    // The number of pixels that will be popped off of the FIFO when there is a mid-tile collision
    uint8_t win_collision_tile_offset = 0;
    // Window tilemap address for when the fetcher changes tilemaps
    uint16_t win_tilemap_addr = 0x0000;
    // Flag is set for when the first tile of the window needs to be drawn at an offset
    bool pop_request = false;
    // Number of pixels to pop off of the FIFO for the according offset of the window
    uint8_t pop_win = 0;

    // Sprite fetcher
    FG_Fetcher fg_fetch;

    bool sprite_pushed = false;
    bool stall = false;
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
