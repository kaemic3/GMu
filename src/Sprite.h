#ifndef PPU_OVERHAUL_SPRITE_H
#define PPU_OVERHAUL_SPRITE_H
#include <cstdint>

struct Sprite {
public:
    Sprite(uint8_t id, uint8_t y, uint8_t x, uint8_t tile_id, uint8_t line, uint8_t attr) {
        y_pos = y; x_pos = x; this->tile_id = tile_id; tile_line = line;
        attrs.data = attr; sprite_id = id;
    }
    ~Sprite() = default;

    // Sprite position
    uint8_t y_pos;
    uint8_t x_pos;
    // Tile info, this will help with finding the correct tile data
    uint8_t tile_id;
    uint8_t tile_line;
    // This bit field represents the different attributes that a sprite can have
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
    // Sprite identifier
    uint8_t sprite_id;
    bool operator < (const Sprite &s) const;
};


#endif //PPU_OVERHAUL_SPRITE_H
