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
    void init(uint16_t addr, uint8_t line, uint8_t offset);

    // Tile info
    uint16_t tilemap_address = 0x0000;
    uint16_t tiledata_address = 0x0000;
    uint8_t tile_id = 0x00;
    uint8_t tile_index = 0;
    uint8_t tile_line = 0;
    uint8_t tile_low = 0;
    uint8_t tile_high = 0;
    // Fetcher internal clock
    uint8_t clocks = 0;
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
