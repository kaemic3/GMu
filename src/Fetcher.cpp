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
                tile_id = ppu->vram[tilemap_address + tile_index];
            else
                // Cast signed int if signed addressing is used. For 0x8800 mode only
                tile_id = ppu->vram[tilemap_address + (int8_t) tile_index];


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
                pixel_buffer[i] = {low_color, ppu->bgp, 0xff};
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
            tile_high= ppu->vram[tiledata_address + 1];
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
    std::queue<Pixel> empty;
    std::swap(fifo, empty);
}