#include <stdexcept>
#include "FG_Fetcher.h"
#include "DMG_PPU.h"

/*
 * The FG_Fetcher will fetch the 8 pixels for the first sprite
 */
uint8_t FG_Fetcher::clock() {
    uint8_t add_dots = 0;

    // Additional check for empty sprites
    if (scanned_sprites.empty())
        return add_dots;

    switch (state) {
        case GetTileId:
            // Look at the front of the scanned_sprites and get its tile id
            tile_id = scanned_sprites.front().tile_id;
            state = GetTileLow;
            break;
        case GetTileLow:
            // Get the tiledata address based on the tile ID
            tile_line = scanned_sprites.front().tile_line;
            // Check to see if the sprite has the Y flip attribute
            if (scanned_sprites.front().attrs.y_flip == 1)
                // If so, adjust the tile_line
                tile_line = 7 - tile_line;
            // Grab the low byte of the tile from VRAM
            tiledata_address = TILEDATA_BASE_ADDR + tile_id * 0x10;
            // Add the tile line offset to the tiledata address
            tiledata_address += tile_line * 2;
            // Grab the low byte of tile data
            ppu_read(tiledata_address, tiledata_low);
            // Now generate pixel data and push to the pixel buffer
            for (uint8_t i = 0; i < 8; i++) {
                // Grab the low_color
                uint8_t low_color = tiledata_low >> i & 1;
                // Select correct color palette
                if (scanned_sprites.front().attrs.palette_number == 0)
                    ppu_read(OBP0_ADDR, color_palette);
                else
                    ppu_read(OBP1_ADDR, color_palette);
                // The lower 2 bits of the color palette are ignored for objs, so set it to 00
                //color_palette &= ~3;
                // Check the BG priority bit and store into the pixel data
                if (scanned_sprites.front().attrs.bg_win_over_obj == 0)
                    bg_win_priority = 0;
                else
                    bg_win_priority = 1;
                // Check X flip attribute, and change x-pos pixel data accordingly
                // Keep in mind that pixels are stored in the least significant order within tiledata RAM
                if (scanned_sprites.front().attrs.x_flip == 0)
                    pixel_buffer.push_back({ low_color, color_palette, bg_win_priority,
                                                static_cast<uint8_t>(scanned_sprites.front().x_pos + (7 - i))});
                else
                    pixel_buffer.push_back({ low_color, color_palette, bg_win_priority,
                                             static_cast<uint8_t>(scanned_sprites.front().x_pos + i)});
                state = GetTileHigh;
            }
            break;
        case GetTileHigh:
            // Grab the high byte of the current sprite
            ppu_read(tiledata_address + 1, tiledata_high);
            // Generate pixel data and push to the pixel buffer
            for (uint8_t i = 0; i < 8; i ++) {
                uint8_t high_color = (tiledata_high >> i & 1) << 1;
                // Or the high color with the existing low color in the pixel buffer
                pixel_buffer[i].color |= high_color;
            }
            // If the sprite has it's X-pos flipped, then we need to flip the pixels in the pixel buffer
            if (scanned_sprites.front().attrs.x_flip == 1)
                pixel_buffer_flip();
            state = Idle;
            break;
        case Idle:
            idle_complete = true;
            break;
    }
    return add_dots;
}

void FG_Fetcher::clear_sprites() {
    scanned_sprites.clear();
}
void FG_Fetcher::emplace_sprite(const Sprite &sprite) {
    scanned_sprites.emplace_back(sprite);
}

void FG_Fetcher::pop_sprite() {
    scanned_sprites.pop_front();
}

uint8_t FG_Fetcher::get_sprite_count() {
    return scanned_sprites.size();
}
Sprite FG_Fetcher::get_front_sprite() {
    if (scanned_sprites.empty()) {
        printf("Out of range access of scanned_sprites\n");
        throw std::out_of_range("scanned_sprites vector is empty.\n");
    }
    return scanned_sprites.front();
}

void FG_Fetcher::sort_sprites() {
    // Do nothing if there are no sprites on this scanline
    if (scanned_sprites.empty())
        return;
    std::sort(scanned_sprites.begin(), scanned_sprites.end());
}

bool FG_Fetcher::ppu_read(uint16_t addr, uint8_t &data) {
    return ppu->cpu_read(addr, data, true);
}

bool FG_Fetcher::pixels_ready() {
    if (pixel_buffer.size() == 8 && idle_complete) {
        return true;
    }
    return false;
}

void FG_Fetcher::pixel_buffer_flip() {
    // Create a copy of the pixel buffer that is reversed, then overwrite the pixel_buffer
    std::vector<Pixel_FG> flipped;
    for (uint8_t i = 0; i < 8; i++) {
        flipped.push_back(pixel_buffer[7 - i]);
    }
    std::swap(pixel_buffer, flipped);
}

std::vector<Pixel_FG> FG_Fetcher::get_pixels() {
    if (pixel_buffer.size() != 8)
        throw std::out_of_range("Cannot get pixels from fg_fetcher. pixel_buffer size is not 8.\n");
    std::vector<Pixel_FG> out = pixel_buffer;
    // Empty the pixel buffer
    pixel_buffer = {};
    // Pop off the front sprite of scanned_sprites
    scanned_sprites.pop_front();
    // Reset the idle complete flag
    idle_complete = false;
    // Change the fetcher state to GetTileId
    state = GetTileId;
    return out;
}
