#include "Fetcher.h"
#include "DMG_PPU.h"

void
BG_Fetcher::init(uint16_t map_addr, uint16_t data_addr, uint8_t line, uint8_t offset, bool signed_mode,
                 uint16_t win_tilemap_addr) {
    tilemap_address = map_addr;
    tiledata_base_address = data_addr;
    tile_line = line;
    tile_index = offset;
    signed_addressing = signed_mode;
    window_tilemap_address = win_tilemap_addr;

    state = GetTileId;

    // Clear the FIFO
    clear_fifo();
}

void BG_Fetcher::clock(DMG_PPU *ppu, bool &swap_to_win, uint8_t win_tile_line, uint8_t win_pixel_x) {
    // Fetcher runs at half the speed of the ppu
    // Only clock the fetcher every 2 clocks of the ppu
    clocks++;
    if (clocks < 2) {
        return;
    }
    // Reset clock count
    clocks = 0;

    // Check the state of the fetcher
    switch (state) {
        case GetTileId:

            // Check if we need to swap over to window tilemap
            if (swap_to_win) {
                tilemap_address = window_tilemap_address;
                tile_line = win_tile_line;
                tile_index = win_pixel_x;
                swap_to_win = false;
            }
            // Need to grab the tile id to be rendered from the corresponding tile map
            // For now we are only looking in the tile map

            // Look in VRAM for the tile in the tile map
            // Masking should be applied in the PPU clock function

            // Make sure to check if signed addressing is needed
            if(!signed_addressing)
                // Make sure that the index is masked to the values 0-31
                tile_id = ppu->vram[tilemap_address + (tile_index & 0x1f)];
            else
                // Cast signed int if signed addressing is used. For 0x8800 mode only
                tile_id = ppu->vram[tilemap_address + (int8_t) (tile_index & 0x1f)];


            state = GetTileLow;
            break;

        case GetTileLow:
            // Check if we need to swap over to window tilemap
            if (swap_to_win) {
                tilemap_address = window_tilemap_address;
                tile_line = win_tile_line;
                tile_index = win_pixel_x;
                swap_to_win = false;
                state = GetTileId;
                break;
            }
            // Find the tile data address
            // The tile data starting address will need to be masked to 0x0000 and the offset added for 0x8800 method
            // Tile id needs to be multiplied by 0x10 or 16 to get the correct 16 byte line in tile data.
            tiledata_address = tiledata_base_address + (tile_id * 0x10);

            // Add the offset of where the line to be fetched is
            // Keep in mind each line is 2 bytes
            tiledata_address = tiledata_address + tile_line * 2;
            // Grab the byte from VRAM
            tile_low = ppu->vram[tiledata_address];

            // Push the low bits to the pixel buffer
            for (uint8_t i = 0; i < 8; i++) {
                uint8_t low_color = tile_low >> i & 1;
                // We set the bg priority to 0xff to indicate this does not use that bit
                pixel_buffer[i] = {low_color, ppu->bgp};
            }
            state = GetTileHigh;
            break;

        case GetTileHigh:
            // Check if we need to swap over to window tilemap
            if (swap_to_win) {
                tilemap_address = window_tilemap_address;
                tile_line = win_tile_line;
                tile_index = win_pixel_x;
                swap_to_win = false;
                state = GetTileId;
                break;
            }
            // Read in the high byte
            tiledata_address = tiledata_base_address + (tile_id * 0x10);

            // Add the offset of where the line to be fetched is
            // Keep in mind each line is 2 bytes
            tiledata_address = tiledata_address + tile_line * 2;
            // Grab the byte from VRAM adding 1 for the high byte
            tile_high = ppu->vram[tiledata_address + 1];
            // Now push the high bits to the pixel buffer
            for (uint8_t i = 0; i < 8; i++) {
                // Shift the high bit one left of the low bit
                uint8_t high_color = (tile_high >> i & 1) << 1;
                // Or the high bit with the low bit
                pixel_buffer[i].color = pixel_buffer[i].color | high_color;
            }
            state = Sleep;
            break;

        case Sleep:
            // Check if we need to swap over to window tilemap
            if (swap_to_win) {
                tilemap_address = window_tilemap_address;
                tile_line = win_tile_line;
                tile_index = win_pixel_x;
                swap_to_win = false;
                state = GetTileId;
                break;
            }
            state = Push;
            break;

        case Push:
            // Check if we need to swap over to window tilemap
            if (swap_to_win) {
                tilemap_address = window_tilemap_address;
                tile_line = win_tile_line;
                tile_index = win_pixel_x;
                swap_to_win = false;
                state = GetTileId;
                break;
            }
            // Need to push pixels from the buffer to the FIFO
            // Need to push to the fifo in reverse order since we pulled pixel data in from least significant to most
            if (fifo.size() <= 8) {
                // Need to use a signed int for this
                for (int8_t i = 7; i >= 0; i--) {
                    fifo.push(pixel_buffer[i]);
                }
            }
            // Advance to the next tile in the tile map
            tile_index++;
            state = GetTileId;
            break;
    }
}

void BG_Fetcher::clear_fifo() {
    // Make an empty queue and swap the memory with the FIFO
    std::queue<Pixel_BG> empty;
    std::swap(fifo, empty);
}

// FG_Fetcher

void FG_Fetcher::clock(DMG_PPU *ppu) {
    // Advance the fetcher every 2 clocks
    /*clocks++;
    if (clocks < 2)
        return;
    clocks = 0;*/
    // If sprite_ids is empty, or if we have pushed all sprites, abort
    if (sprites.empty() || sprite_index > sprites.size() - 1 || ppu->lcdc.obj_enable == 0)
        return;

    // If the x_pos of the sprite is 0, then do not push its pixels to the fifo
    if (sprites[sprite_index].x_pos == 0) {
        sprite_index++;
        return;
    }

    switch (state) {
        case GetTileId:
            // Find the tile ID: Located in byte 2 of the sprite's OAM
            tile_id = sprites[sprite_index].tile_index;
            // Set the tiledata address
            // Sprites always use 0x8000 for tiledata
            tiledata_address = tiledata_base_address;
            state = GetTileLow;
            break;
        case GetTileLow:
            // Pixels need to be pushed to the FIFO differently depending on some attributes in byte 3 of the
            // sprites OAM
            // Byte 3:
            // - Bit 5: X flip (0 = Normal, 1 = Horizontally mirrored)
            // - Bit 6: Y flip (0 = Normal, 1 = Vertically mirrored)

            // Determine if the sprite is vertically flipped
            // Grab bit 6 of byte 3 of the current sprite
            tile_line = sprites[sprite_index].tile_line;
            if (sprites[sprite_index].attrs.y_flip == 1)
                tile_line = 7 - tile_line;

            // Grab the low byte of the tile from VRAM
            tiledata_address += tile_id * 0x10;
            tiledata_address += tile_line * 2;
            tile_low = ppu->vram[tiledata_address];

            // Push the low bits to the pixel buffer
            for (uint8_t i = 0; i < 8; i++) {
                uint8_t low_color = tile_low >> i & 1;
                // Select the correct palette byte 3 bit 4
                if (sprites[sprite_index].attrs.palette_number == 1)
                    palette = ppu->obp1;
                else
                    palette = ppu->obp0;
                // Pull the BG priority byte 3 bit 7
                if (sprites[sprite_index].attrs.bg_win_over_obj == 1)
                    // With this set, BG and Window colors 1-3 draw over the sprite
                    bg_priority = 1;
                else
                    bg_priority = 0;
                // Save the lower pixels to the buffer
                // Check if the sprite should be flipped, if so then we need to push the X pos accordingly
                if (sprites[sprite_index].attrs.x_flip == 1)
                    pixel_buffer[i] = {low_color, palette, bg_priority, static_cast<uint8_t>(sprites[sprite_index].x_pos +i)};
                // Keep in mind we are storing the pixels in the least significant order, and they need to be pushed in most significant order.
                // Update the x_pos member of the pixel in reverse since the sprite is not flipped
                else
                    pixel_buffer[i] = {low_color, palette, bg_priority, static_cast<uint8_t>(sprites[sprite_index].x_pos + (7 - i))};
            }
            state = GetTileHigh;
            break;
        case GetTileHigh:
            // Grab the high byte from VRAM
            tile_high = ppu->vram[tiledata_address + 1];

            // Push the low bits to the pixel buffer
            for (uint8_t i = 0; i < 8; i++) {
                // Shift the high bit one left of the low bit
                uint8_t high_color = (tile_high >> i & 1) << 1;
                // Or the high bit with the low bit
                pixel_buffer[i].color = pixel_buffer[i].color | high_color;
            }
            state = Sleep;
            break;
        case Sleep:
            state = Push;
            break;
        case Push:

            // Pixels need to be pushed to the FIFO differently depending on some attributes in byte 3 of the
            // sprites OAM
            // Byte 3:
            // - Bit 5: X flip (0 = Normal, 1 = Horizontally mirrored)
            // - Bit 6: Y flip (0 = Normal, 1 = Vertically mirrored)
            // In this state, we only need to worry about horizontally mirroring the pixels.

            // The GetTileLow/High states take care of the vertical mirroring
            if (sprites[sprite_index].attrs.x_flip == 1) {
                // Since the sprite is flipped, we can push from the front of the buffer
                if (fifo.size() <= 8) {
                    for (uint8_t i = 0; i < 8; i++)
                        fifo.push(pixel_buffer[i]);
                }
                pushed = true;
            }
            else {
                // Need to push pixels from the buffer to the FIFO
                // Need to push to the fifo in reverse order since we pulled pixel data in from least significant to most
                if (fifo.size() <= 8) {
                    // Need to use a signed int for this
                    for (int8_t i = 7; i >= 0; i--) {
                        fifo.push(pixel_buffer[i]);
                    }
                    pushed = true;
                }
            }
            // The index should only change when there is a need to draw the next sprite
            if (pushed) {
                sprite_index++;
                state = GetTileId;
                pushed = false;
            }
            break;
    }
}

void FG_Fetcher::init() {
    sprite_index = 0;
    tiledata_base_address = 0x0000;
    tiledata_address = 0x0000;
    tile_id = 0;
    tile_line = 0;
    tile_low = 0;
    tile_high = 0;
    palette = 0;
    bg_priority = 0;
    pushed = false;

    clear_fifo();
}

void FG_Fetcher::clear_fifo() {
    std::queue<Pixel_FG> empty;
    std::swap(fifo, empty);
}

