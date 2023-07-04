#ifndef GMU_DMG_PPU_H
#define GMU_DMG_PPU_H

#include <cstdint>
#include <array>
#include <queue>
#include "Pixel.h"
#include "BG_Fetcher.h"
#include "FG_Fetcher.h"

// Forward declare Bus
class Bus;

class DMG_PPU {

public:
    DMG_PPU();

    ~DMG_PPU() = default;

    // System functions
    void clock();
    void reset();

    // Communication with the main bus
    bool cpu_write(uint16_t addr, uint8_t data, bool is_dma = false);
    bool cpu_read(uint16_t addr, uint8_t &data, bool is_fetcher = false);

    // Connect the Bus to the PPU
    void connect_bus(Bus *p_bus) { bus = p_bus; }
    bool get_vblank_flag() { return vblank_flag; }
    bool get_stat_hblank_flag() { return stat_hblank_flag; }
    bool get_stat_oam_flag() { return stat_oam_flag; }
    bool get_frame_state() { return frame_complete; }
    uint8_t get_scanline_x() { return scanline_x; }
    // TODO Need to add compatability for making these private
    // Cannot make them private due to an issue with the front end using references to the LY register
//private:
    // Bus pointer
    Bus *bus = nullptr;
    // Allow cpu
    bool cpu_access = false;
    // Allow fetcher
    bool fetcher_access = false;
    void set_fetcher_flag(bool val);


    uint32_t clock_count = 0;
    bool frame_complete = false;
    // Memory
    std::array<uint8_t, 8 * 1024> vram = {};
    std::array<uint8_t, 160> oam = {};

    // An enum that contains the different states of the PPU
    enum PPUState {
        OAMSearch,
        PixelTransfer,
        HBlank,
        VBlank
    } state;

    // PixelTransfer modes
    enum PTMode {
        Normal,
        ObjFetch
    } pt_mode;

    enum PushMode {
        BGWin,
        Object
    } push_mode;
    // Fetchers
    // Technically on the GB there is only one fetcher,
    // but it is easier to split the fetchers in our case.
    BG_Fetcher bg_fetcher;
    FG_Fetcher fg_fetcher;
    // FIFOs
    std::queue<Pixel_BG> bg_fifo;
    std::queue<Pixel_FG> fg_fifo;
    // Connector function to the bus push_pixel funciton
    void push_pixel(uint8_t pixel, uint32_t index);
    uint8_t map_color (uint8_t color, uint8_t palette);
    void clear_fifos();
    uint8_t bg_win_swap_pos = 0xff;
    uint8_t win_pixel_offset = 0;
    uint8_t sprite_offscreen_offset = 0;
    // PPU registers
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
    // TODO Re-implement the front end for proper encapsulation
    // Needs to be 32 bit due to a reference in the zViewport class that casts it
    uint32_t ly = 0;
    uint8_t lyc = 0;
    uint8_t scx = 0;
    uint8_t scy = 0;
    uint8_t wx = 0;
    uint8_t wy = 0;
    uint8_t bgp = 0;
    uint8_t obp0 = 0;
    uint8_t obp1 = 0;

    // Scanline X pos - Updated when a pixel is pushed to the screen
    uint8_t scanline_x = 0;

    // Interrupt flags
    bool stat_oam_flag = false;
    bool stat_pixel_transfer_flag = false;
    bool stat_hblank_flag = false;
    bool stat_ly_lyc_flag = false;
    bool vblank_flag = false;
};
#endif //GMU_DMG_PPU_H
