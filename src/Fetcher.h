#ifndef GMU_FETCHER_H
#define GMU_FETCHER_H
#include <cstdint>
#include <array>
#include <queue>
#include "Pixel.h"

class DMG_PPU;
// Background and window fetcher
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
    std::queue<Pixel_BG> fifo;
    // Clear the FIFO
    void clear_fifo();

    // Array to hold pixels before they go to the FIFO
    std::array<Pixel_BG, 8> pixel_buffer = {};

    // Enum to represent the different states the fetcher can be in
    enum FetcherState {
        GetTileId,
        GetTileLow,
        GetTileHigh,
        Sleep,
        Push
    } state = GetTileId;
};

// Sprite fetcher
struct FG_Fetcher {
    FG_Fetcher() = default;
    ~FG_Fetcher() = default;

    // Enum to represent the different states the fetcher can be in
    enum FetcherState {
        GetTileId,
        GetTileLow,
        GetTileHigh,
        Sleep,
        Push
    } state = GetTileId;

    struct Sprite {
        Sprite(uint8_t id, uint8_t y, uint8_t x, uint8_t index, uint8_t line, uint8_t attr) {
            y_pos = y; x_pos = x; tile_index = index; tile_line = line;
            attrs.data = attr; sprite_id = id;
        }
        ~Sprite() = default;
        uint8_t y_pos;
        uint8_t x_pos;
        uint8_t tile_index;
        uint8_t tile_line;
        union attributes{
            struct {
                uint8_t unused          : 4;
                uint8_t palette_number  : 1;
                uint8_t x_flip          : 1;
                uint8_t y_flip          : 1;
                uint8_t bg_win_over_obj : 1;
            };
            uint8_t data;
        } attrs;
        uint8_t sprite_id = 0;
        // Overload < operator for sorting the list
        bool operator < (const Sprite &s) const {
            // The sprite with the lower X pos will go first
            if (x_pos < s.x_pos) {
                return true;
            }
            // If the X pos is the same, store the sprite with the lower sprite id first
            if (x_pos == s.x_pos) {
                if (sprite_id < s.sprite_id)
                    return true;
                else
                    return false;
            }
            return false;
        }
    };

    // Store the sprite IDs for the sprites on the current scanline
    std::vector<Sprite> sprites;
    uint8_t sprite_index = 0;
    uint16_t tiledata_base_address = 0x0000;
    uint16_t tiledata_address = 0x0000;
    uint8_t tile_id = 0;
    uint8_t tile_line = 0;
    uint8_t tile_low = 0;
    uint8_t tile_high = 0;
    uint8_t palette = 0;
    uint8_t bg_priority = 0;

    bool pushed = false;

    void init();
    // Pixel buffer
    std::array<Pixel_FG, 8> pixel_buffer = {};

    // FG FIFO
    std::queue<Pixel_FG> fifo;
    // Clear the FIFO
    void clear_fifo();
    // Clock function for the FG_Fetcher
    void clock(DMG_PPU *ppu);
    uint8_t clocks = 0;
};

#endif //GMU_FETCHER_H
