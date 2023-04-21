//
// Created by Kaelan on 4/21/23.
//

#include "Fetcher.h"
#include "DMG_PPU.h"

// May want to move some of the values into the Fetcher struct
void Fetcher::clock(DMG_PPU *ppu) {

    // Fetcher runs at half the speed of the ppu
    clocks++;
    if (clocks < 2) {
        return;
    }
    // Reset clock count
    clocks = 0;
    // Check the state of the fetcher
    switch (state) {

        case Fetcher::GetTileId:
            // Need to grab the tile id to be rendered from the corresponding tile map
            // For now just pull from the BG map

            // Check the BG tilemap location
            if (ppu->lcdc.bg_tile_map_area == 0) {
                // 0x9800 - but here it needs to be offset to 0x0000
                ppu->tilemap_address = 0x1800;
                // To find the tile in the map we need to convert the fetch.y to a
                // tile position.
                ppu->tile_x = x;
                ppu->tile_y = y / 8 * 32;
                // With these we can now find the tile id in the tilemap
                ppu->tile_id = ppu->vram[ppu->tilemap_address + ppu->tile_x + ppu->tile_y];
                // Change the state of the fetcher to GetTileLow
                state = Fetcher::GetTileLow;
            }
            else if (ppu->lcdc.bg_tile_map_area == 1) {
                // 0x9c00 - but here it needs to be offset to 0x0000
                ppu->tilemap_address = 0x1c00;
            }
            break;

        case Fetcher::GetTileLow:
            // With the tile id we can now find the correct line of the tile to grab
            // from tile data VRAM

            // Get the line of the tile we need to grab from VRAM
            ppu->tile_line = ppu->ly % 8;
            // Find the tile data in VRAM
            // Check for the tile data area in LCDC
            if (ppu->lcdc.bg_win_tile_data_area == 0) {      // <- Can restructure for better readability
                // This uses signed addressing and the 00 is at 0x9000
                // 0x8800
                ppu->tiledata_address = 0x1000;
                // Need to find the tile in data ram
                ppu->tiledata_address = ppu->tiledata_address + (ppu->tile_id * 0x10);
                // Pull the data from VRAM
                ppu->tile_low = ppu->vram[ppu->tiledata_address + ppu->tile_line];
            }
            else if (ppu->lcdc.bg_win_tile_data_area == 1) {
                // 0x8000
            }
            state = Fetcher::GetTileHigh;


        case Fetcher::GetTileHigh:
            // Read in the high byte
            ppu->tile_high = ppu->vram[ppu->tiledata_address + ppu->tile_line + 1];
            state = Fetcher::Sleep;
            break;

        case Fetcher::Sleep:
            state = Fetcher::Push;
            break;

        case Fetcher::Push:
            // Assume bgp for now
            // Remember that the fetcher needs to provide 8 pixels at a time to the FIFO
            // Generate 8 pixels and store in fetcher
            for(uint8_t i = 0; i < 8; i++) {
                // Create a pixel color storing bit i from low and high bytes
                // then align to the first 2 bits
                uint8_t pixel_color = (ppu->tile_low >> i & 1 | (ppu->tile_high >> i & 1) << 1);
                pixel_store[i] = {pixel_color, ppu->bgp, 0xff};
            }
            // Now we have 8 pixels in our fetcher, push them to the FIFO
            ppu->fifo_push(pixel_store);
            // Clear the pixel store in the fetcher
            clear_store();
            state = Fetcher::GetTileId;
            break;
    }
}