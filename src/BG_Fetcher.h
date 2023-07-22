#ifndef GMU_BG_FETCHER_H
#define GMU_BG_FETCHER_H
#include <cstdint>
#include <array>
#include <queue>
#include "Pixel.h"
#include "Sprite.h"

#define LCDC_ADDRESS    0xff40
#define WX_ADDRESS      0xff4b
#define WY_ADDRESS      0xff4a
#define SCX_ADDRESS     0xff43
#define SCY_ADDRESS     0xff42
#define LY_ADDRESS      0xff44
#define BGP_ADDRESS     0xff47

class DMG_PPU;
// Background and window fetcher
class BG_Fetcher {
public:
    explicit BG_Fetcher(DMG_PPU *p_ppu) : ppu(p_ppu) { }
    BG_Fetcher() = default;
    ~BG_Fetcher() = default;
    // Functions

    // Return the fetched pixels
    std::vector<Pixel_BG> get_pixels();
    // Check if the pixel array is empty
    bool pixels_ready();
    // Initialize the fetcher - called each scanline
    void init();
    // Returns the BG Win. swap position
    uint8_t get_swap_pos() const { return swap_position; }
    // Returns the pixel offset when the window is not aligned to the 8x8 tile grid
    uint8_t get_win_pixel_offset() const { return pixel_offset_win; }
    uint8_t get_bg_pixel_offset() const { return pixel_offset_bg; }
    bool fetcher_mode_win() const { return (fetch_mode == WindowOnly); }
    // Main clock for the BG fetcher
    void clock();
private:
    // Tile related members
    uint16_t tiledata_base_addr = 0x0000;
    uint16_t tiledata_addr = 0x0000;
    uint16_t tilemap_bg = 0x0000;
    uint16_t tilemap_win = 0x0000;
    uint8_t tile_id = 0;
    uint8_t tile_line_offset = 0;
    // This is only used when the tilemap needs to switch from BG to win
    uint8_t tile_line_offset_win = 0;
    uint8_t tile_data_low = 0;
    uint8_t tile_data_high = 0;
    // The offset/index to the current tile in the tilemap
    uint8_t tile_index = 0;
    uint8_t pixel_offset_win = 0;
    uint8_t pixel_offset_bg = 0;
    bool tilemap_swap = false;
    uint8_t swap_position = 0xff;
    // Number of pixels popped off the fifo for the current scanline
    uint8_t pixel_count = 0;
    // Background palette
    uint8_t bgp = 0;

    std::vector<Pixel_BG> pixel_buffer = {};
    // Flag is set when 1 clock cycle of idle completes
    bool idle_complete = false;

    // Different fetcher states
    enum FetcherState {
        GetTileId,
        GetTileLow,
        GetTileHigh,
        Idle
    } state = GetTileId;
    // Different fetcher modes
    enum FetcherMode {
        BackgroundOnly,
        WindowOnly,
        BackgroundAndWindow,
        BGWinOff
    } fetch_mode;

    // PPU access
    DMG_PPU *ppu = nullptr;
    bool ppu_read(uint16_t addr, uint8_t &data);
    // Clock count
    uint8_t clock_count = 0;
};
#endif //GMU_BG_FETCHER_H
