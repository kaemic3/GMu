#ifndef GMU_FETCHER_H
#define GMU_FETCHER_H
#include <cstdint>
#include <array>
#include <queue>
#include "Pixel.h"

class DMG_PPU;
struct Fetcher {
    Fetcher() = default;
    ~Fetcher() = default;

    // Function to initialize the fetcher
    void init(uint16_t map_addr, uint16_t data_addr, uint8_t line, uint8_t offset, bool signed_mode, bool win_collision);

    // Tile info
    uint16_t tilemap_address = 0x0000;
    // The base address
    uint16_t tiledata_base_address = 0x0000;
    // This address is modified as tile data is accessed
    uint16_t tiledata_address = 0x0000;
    uint8_t tile_id = 0x00;
    uint8_t tile_index = 0;
    uint8_t tile_line = 0;
    uint8_t tile_low = 0;
    uint8_t tile_high = 0;
    // Flag to use signed addressing to fetch the tile from the tiledata
    bool signed_addressing = false;
    // Fetcher internal clock
    uint8_t clocks = 0;

    // Used for pulling pixel data from the window during scanline that has a collision between the window and the BG
    bool window_collision = false;
    uint16_t window_tilemap_address = 0x0000;
    // Clock function for the fetcher
    void clock(DMG_PPU *ppu);

    // Out pixel FIFO
    std::queue<Pixel> fifo;
    // Clear the FIFO
    void clear_fifo();

    // Array to hold pixels before they go to the FIFO
    std::array<Pixel, 8> pixel_buffer = {};
    // Clear the buffer
    void clear_buffer();
    // Enum to represent the different states the fetcher can be in
    enum FetcherState {
        GetTileId,
        GetTileLow,
        GetTileHigh,
        Sleep,
        Push
    } state = GetTileId;
};


#endif //GMU_FETCHER_H
