#ifndef GMU_FETCHER_H
#define GMU_FETCHER_H
#include <cstdint>
#include <array>
#include <queue>
#include "Pixel.h"

class DMG_PPU;
struct BG_Fetcher {
    BG_Fetcher() = default;
    ~BG_Fetcher() = default;

    // Function to initialize the fetcher
    void init(uint16_t map_addr, uint16_t data_addr, uint8_t line, uint8_t offset, bool signed_mode,
              uint16_t win_tilemap_addr);

    // Tile info
    // Tilemap address used to pull the tile id from
    uint16_t tilemap_address = 0x0000;
    // The base address for tiledata
    uint16_t tiledata_base_address = 0x0000;
    // This address is modified as tiledata is accessed
    uint16_t tiledata_address = 0x0000;
    // Current tile id
    uint8_t tile_id = 0x00;
    // Offset from the beginning of the tilemap to grad a tile id from
    uint8_t tile_index = 0;
    // Offset to grab the correct 2 bytes from tiledata for the corresponding line of pixels
    uint8_t tile_line = 0;
    // Low byte of the tile line
    uint8_t tile_low = 0;
    // High byte of the tile line
    uint8_t tile_high = 0;
    // Flag to use signed addressing to fetch the tile from the tiledata
    bool signed_addressing = false;
    // Fetcher internal clock
    uint8_t clocks = 0;

    // Used for pulling pixel data from the window during scanline that has a collision between the window and the BG
    uint16_t window_tilemap_address = 0x0000;

    // Clock function for the fetcher
    // ppu: Pointer to the PPU
    // swap_to_win: Reference to the flag that tells the fetcher to switch tilemaps to window
    // win_tile_line: Address to the current tilemap line of the window
    // win_pixel_x: Current offset from the start of the window tilemap
    void clock(DMG_PPU *ppu, bool &swap_to_win, uint8_t win_tile_line, uint8_t win_pixel_x);

    // Out pixel FIFO
    std::queue<Pixel> fifo;
    // Clear the FIFO
    void clear_fifo();

    // Array to hold pixels before they go to the FIFO
    std::array<Pixel, 8> pixel_buffer = {};

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
