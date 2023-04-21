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
    // This will act as the pixel FIFO fetcher
    Fetcher fetch;
    // Used in the pixel transfer state

    uint16_t tilemap_address = 0x0000;
    uint16_t tiledata_address = 0x0000;
    uint8_t tile_id = 0x00;
    uint8_t tile_x = 0;
    uint8_t tile_y = 0;
    uint8_t tile_line = 0;
    uint8_t tile_low = 0;
    uint8_t tile_high = 0;
    uint8_t tile_color = 0;


    // Pixel FIFOS - these need to be 16 in size
    std::queue<Pixel> fifo_bg;
    std::queue<Pixel> fifo_obj;
    // FIFO push
    void fifo_push(std::array<Pixel, 8> pixels);
    // Clear's a FIFO
    static void clear_fifo(std::queue<Pixel> &q);
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
