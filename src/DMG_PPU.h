#ifndef GMU_DMG_PPU_H
#define GMU_DMG_PPU_H

#include <cstdint>
#include <array>
#include <queue>

// Forward declare Bus and Fetcher

class Bus;
class Fetcher;

class DMG_PPU {
public:
    DMG_PPU();
    ~DMG_PPU() = default;

    // Communication with the main bus
    bool cpu_write(uint16_t addr, uint8_t data);
    bool cpu_read(uint16_t addr, uint8_t &data);
    // No need for ppu_r/w functions since there is no PPU bus

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

    // Data the FIFO will need
    struct Pixel {
        uint8_t color;
        uint8_t palette;
        uint8_t bg_priority;
    };

    // Color palettes
    uint8_t bgp = 0;
    uint8_t obp0 = 0;
    uint8_t obp1 = 0;

    // Pixel FIFOS - these need to be 16 in size
    std::queue<Pixel> fifo_bg;
    std::queue<Pixel> fifo_obj;

    // Set to true if the frame has been completed
    bool frame_complete = false;

    // Current scanline
    // uint32_t to make it easier to draw to the debug viewport
    uint32_t ly = 0;
    // X position on the current scanline
    uint8_t x = 0;

    // LYC register
    uint8_t lyc = 0;

private:
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
            unsigned int bg_window_enable        : 1;
            unsigned int obj_enable              : 1;
            unsigned int obj_size                : 1;
            unsigned int bg_tile_map_area        : 1;
            unsigned int bg_win_tile_data_area   : 1;
            unsigned int win_enable              : 1;
            unsigned int win_tile_map_area       : 1;
            unsigned int lcd_ppu_enable          : 1;
        };
        uint8_t data;
    } lcdc;

    // Stat register - top to bottom bit 0 - 7
    union stat_register {
        struct {
            unsigned int mode_flag      : 2;    // Read only
            unsigned int lyc_ly_flag    : 1;    // Read only
            unsigned int hblank_int_src : 1;
            unsigned int vblank_int_src : 1;
            unsigned int oam_int_src    : 1;
            unsigned int lyc_int_src    : 1;
            unsigned int unused         : 1;
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

// This will act as the pixel FIFO fetcher
class Fetcher {

};

#endif //GMU_DMG_PPU_H
