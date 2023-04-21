//
// Created by Kaelan on 4/21/23.
//

#ifndef GMU_FETCHER_H
#define GMU_FETCHER_H
#include <cstdint>
#include <array>
#include "Pixel.h"

class DMG_PPU;
struct Fetcher {
    Fetcher() = default;
    ~Fetcher() = default;

    // Fetcher position
    uint8_t x = 0;
    uint8_t y = 0;

    // Fetcher internal clock
    uint8_t clocks = 0;
    // Clock function for the fetcher
    void clock(DMG_PPU *ppu);

    // Pixels to be pushed to the FIFO
    std::array<Pixel, 8> pixel_store = {};
    // Clear the array
    void clear_store() {
        // Clear the pixel_store
        std::array<Pixel, 8> empty;
        std::swap(pixel_store, empty);
    }
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
