#ifndef GMU_FG_FETCHER_H
#define GMU_FG_FETCHER_H
#include <cstdint>
#include <array>
#include <queue>
#include "Pixel.h"
#include "Sprite.h"

#define TILEDATA_BASE_ADDR  0x8000
#define OBP0_ADDR           0xff48
#define OBP1_ADDR           0xff49

class DMG_PPU;

class FG_Fetcher {
public:
    explicit FG_Fetcher(DMG_PPU *p_ppu) : ppu (p_ppu) { }
    FG_Fetcher() = default;
    ~FG_Fetcher() = default;
    // FG_Fetcher clock function - returns the number of additional clocks required by the fetcher
    uint8_t clock();
    // Sprite vector member functions
    void clear_sprites();
    void emplace_sprite(const Sprite &sprite);
    void pop_sprite();
    uint8_t get_sprite_count();
    // Returns the first sprite in the sprite list
    Sprite get_front_sprite();
    void sort_sprites();
    // Returns true when the fetcher is done fetching pixels
    bool pixels_ready();
    // Returns a copy of the pixel buffer
    std::vector<Pixel_FG> get_pixels();
private:
    // Connection to the PPU
    DMG_PPU *ppu = nullptr;
    std::deque<Sprite> scanned_sprites;
    // Pixel buffer
    std::vector<Pixel_FG> pixel_buffer;
    bool idle_complete = false;
    enum FetcherState {
        GetTileId,
        GetTileLow,
        GetTileHigh,
        Idle
    } state = GetTileId;
    enum SpriteMode {
        Standard,
        Tall
    } mode;
    // Tile ID of the current sprite being fetched
    uint8_t tile_id = 0;
    // Line number of the current sprite that is being fetched
    uint8_t tile_line = 0;
    uint16_t tiledata_address = 0x0000;
    uint8_t tiledata_low = 0x00;
    uint8_t tiledata_high = 0x00;
    uint8_t color_palette = 0;
    uint8_t bg_win_priority = 0;
    // Flip the order of pixels in the pixel buffer. Used when sprite has attribute to flip X
    void pixel_buffer_flip();
    // Connector to the PPU read function
    bool ppu_read (uint16_t addr, uint8_t &data);
};




#endif //GMU_FG_FETCHER_H
