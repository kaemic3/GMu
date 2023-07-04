#include "BG_Fetcher.h"
#include "DMG_PPU.h"

void BG_Fetcher::init() {
    // Reset all members
    pixel_count = 0;
    tile_line_offset_win = 0;
    tiledata_base_addr = 0x0000;
    tiledata_addr = 0x0000;
    tilemap_bg = 0x0000;
    tilemap_win = 0x0000;
    tile_id = 0;
    tile_line_offset = 0;
    tile_line_offset_win = 0;
    tile_data_low = 0x00;
    tile_data_high = 0x00;
    tile_index = 0;
    pixel_offset = 0;
    bgp = 0;
    clock_count = 0;
    pixel_buffer = {};
    tilemap_swap = false;
    swap_position = 0xff;
    idle_complete = false;
    state = GetTileId;
    // Grab the state of the relevant PPU members via the ppu_read function

    // Create a temporary copy of the lcdc_register union
    union lcdc_register {
        struct {
            uint8_t bg_window_enable        : 1;
            uint8_t obj_enable              : 1;
            uint8_t obj_size                : 1;
            uint8_t bg_tile_map_area        : 1;
            uint8_t bg_win_tile_data_area   : 1;
            uint8_t win_enable              : 1;
            uint8_t win_tile_map_area       : 1;
            uint8_t lcd_ppu_enable          : 1;
        };
        uint8_t data;
    } lcdc;
    uint8_t wx;
    uint8_t wy;
    uint8_t scx;
    uint8_t scy;
    uint8_t ly;
    // Grab the state of lcdc from the PPU
    if (!ppu_read(LCDC_ADDRESS, lcdc.data))
        printf("Error, could not read LCDC via ppu_read.\n");
    // Grab the state of wx
    if (!ppu_read(WX_ADDRESS, wx))
        printf("Error, could not read WX via ppu_read.\n");
    // Grab the state of wy
    if (!ppu_read(WY_ADDRESS, wy))
        printf("Error, could not read WY via ppu_read.\n");
    // Grab the state of scx
    if (!ppu_read(SCX_ADDRESS, scx))
        printf("Error, could not read SCX via ppu_read.\n");
    // Grab the state of scy
    if (!ppu_read(SCY_ADDRESS, scy))
        printf("Error, could not read SCY via ppu_read.\n");
    // Grab the state of ly
    if (!ppu_read(LY_ADDRESS, ly))
        printf("Error, could not read LY via ppu_read.\n");
    // Grab the state of the bgp
    if (!ppu_read(BGP_ADDRESS, bgp))
        printf("Error, could not read BGP via ppu_read.\n");
    // Get tiledata address from lcdc
    switch (lcdc.bg_win_tile_data_area) {
        case 0:
            tiledata_base_addr = 0x9000;
            break;
        case 1:
            tiledata_base_addr = 0x8000;
            break;
    }
    // Now set the fetch_mode

    // Fetch white pixels
    if (lcdc.bg_window_enable == 0) {
        fetch_mode = BGWinOff;
        tilemap_bg = 0xffff;
        tilemap_win = 0xffff;
    }
    // Only draw BG
    else if (lcdc.bg_window_enable == 1 && lcdc.win_enable == 0) {
        fetch_mode = BackgroundOnly;
        // Grab the BG tilemap address
        switch (lcdc.bg_tile_map_area) {
            case 0:
                tilemap_bg = 0x9800;
                break;
            case 1:
                tilemap_bg = 0x9c00;
        }
    }
    // Check to see if the window and BG are drawn, just the window, or just the BG
    else if (lcdc.bg_window_enable == 1 && lcdc.win_enable == 1) {
       /* Check if the window is in a valid location.
        * This means that the window is within the bounds
        * of the viewport, and that part of it is on the current
        * scanline.
        */
        if (wx <= 166 && wy <= 143 && (wy <= ly)) {
            // Draw the window at an offset
            if (wx > 0 && wx < 7) {
                // If we get here, only the window will be drawn on this
                // scanline.
                fetch_mode = WindowOnly;
                pixel_offset = 7 - wx;
            }
            // Draw the window with an 8 pixel offset
           /*
            * Technically, wx = 166 has different behavior.
            * This behavior is unimportant for the goal of this emulator.
            * TODO: Change WX = 166 behavior to be accurate
            */
            else if (wx == 0 || wx == 166) {
                fetch_mode = WindowOnly;
                pixel_offset = 8;
            }
                // Draw the window from the beginning of the scanline
            else if (wx == 7) {
                fetch_mode = WindowOnly;
            }
                // Draw the window in a normal position
            else if (wx > 7 && wx < 166) {
                fetch_mode = BackgroundAndWindow;
            }
            // Grab the window tile map
            switch (lcdc.win_tile_map_area) {
                case 0:
                    tilemap_win = 0x9800;
                    break;
                case 1:
                    tilemap_win = 0x9c00;
                    break;
            }
        }
            // If the window is not in a valid location, setup for drawing
            // from only the BG
        else {
            fetch_mode = BackgroundOnly;
            // Grab the BG tilemap address
            switch (lcdc.bg_tile_map_area) {
                case 0:
                    tilemap_bg = 0x9800;
                    break;
                case 1:
                    tilemap_bg = 0x9c00;
                    break;
            }
        }
    }
    // Initialize the fetchers based on their "mode"
    uint8_t tilemap_offset_y = 0;
    switch (fetch_mode) {
        // In this mode, the PPU should write white pixels to the screen
        case BGWinOff:
            break;
        case BackgroundOnly:
            tile_index = (scx / 8) & 0x1f;
            tilemap_offset_y = scy + ly;
            tile_line_offset = tilemap_offset_y % 8;
            tilemap_bg += tilemap_offset_y / 8 * 32;
            break;
        case WindowOnly:
            tile_index = 0;
            tilemap_offset_y = ly - wy;
            tile_line_offset_win = tilemap_offset_y % 8;
            tilemap_win += tilemap_offset_y / 8 * 32;
            break;
        case BackgroundAndWindow:
            // When both the BG and window are rendered, the fetcher first fetches BG pixels,
            // then swaps to the window at a certain point each scanline.
            // First, set up the BG values
            tile_index = (scx / 8) & 0x1f;
            tilemap_offset_y = scy + ly;
            tile_line_offset = tilemap_offset_y % 8;
            tilemap_bg += tilemap_offset_y / 8 * 32;
            // Now the window
            tilemap_offset_y = ly - wy;
            tile_line_offset_win = tilemap_offset_y % 8;
            tilemap_win += tilemap_offset_y / 8 *32;
            // Calculate when the swap from BG to window will occur
            // Swap position contains the pixel offset for the first window pixel on the screen
            swap_position = (wx - 7);
            break;
    }
}
/*
 * Key info about the fetcher and the FIFO
 * Fetcher:
 * - Operates at half the clock speed of the FIFO ~2 MHz
 * - 3 clocks are required to fetch 8 pixels
 * - The 4th clock is an idle state where the fetcher will wait until
 *   the FIFO has space for 8 pixels. This should only last 1 clock.
 * FIFO:
 *  - Operates on the dot clock ~4 MHz
 *  - Pushes pixels to the screen every clock it can
 *  - Will pause unless the FIFO contains more than 8 pixels
 *
 * We will be handling pixel pushing via the pop_pixel function. This function will be called
 * within the PPU's clock function, every clock cycle.
 * That means that everything within the fetcher clock function will operate every 2 clock
 * cycles.
 *
 */

void BG_Fetcher::clock() {
    // Only run the fetcher every 2 clock cycles.
    clock_count++;
    if (clock_count < 2 && state != Idle)
        return;
    clock_count = 0;
    // Fetcher state machine
    switch (state) {
        case GetTileId:
            uint16_t tilemap_address;
            // Check to see which tilemap to use based on the fetcher mode
            switch (fetch_mode) {
                case BackgroundOnly:
                    tilemap_address = tilemap_bg;
                    break;
                case WindowOnly:
                    tilemap_address = tilemap_win;
                    break;
                case BackgroundAndWindow:
                    // Check if we need to swap over to the window tilemap
                    if (pixel_count >= swap_position ) {
                        tilemap_address = tilemap_win;
                        tilemap_swap = true;
                    }
                    else
                        tilemap_address = tilemap_bg;
                    break;
                case BGWinOff:
                    // This mode should only return white pixels
                    tilemap_address = 0xffff;
                    break;
            }
            // Now grab the TileID
            ppu_read(tilemap_address + (tile_index & 0x1f), tile_id);
            state = GetTileLow;
            break;
        case GetTileLow:
            // Need to check if the tile data area uses signed addressing
            switch (tiledata_base_addr) {
                // 0x8000 method uses unsigned addressing
                case 0x8000:
                    // Grab the low byte of tile data from 0x8000 based on the tile id.
                    /*
                     * Tile are 16 bytes each and are stored at the base address, in this case 0x8000.
                     * By multiplying the tile id by 0x10 (16) we can get a pointer to the beginning of the
                     * tile data for the given tile id.
                     *
                     */
                    tiledata_addr = tiledata_base_addr + (tile_id * 0x10);
                    break;
                // 0x9000, also known as the 0x8800 method uses signed addressing mode
                case 0x9000:
                    // Functions the same as 0x8000, just uses signed addressing so cast signed int to the tile id
                    tiledata_addr = tiledata_base_addr + ((int8_t) tile_id * 0x10);
                    break;
            }
            // Now that we have the correct tiledata, find the correct byte for the current tile line.
            // The offset added to the tiledata address depends on if we are getting a byte from the window or from the
            // background.
            switch (fetch_mode) {
                case BackgroundOnly:
                    tiledata_addr = tiledata_addr + tile_line_offset * 2;
                    break;
                case WindowOnly:
                    tiledata_addr = tiledata_addr + tile_line_offset_win * 2;
                    break;
                case BackgroundAndWindow:
                    if (tilemap_swap)
                        tiledata_addr = tiledata_addr + tile_line_offset_win * 2;
                    else
                        tiledata_addr = tiledata_addr + tile_line_offset * 2;
                    break;
                case BGWinOff:
                    // Set an impossible address to indicate that the fetcher should fetch white pixels
                    tiledata_addr = 0xffff;
                    break;
            }
            // Now grab the low byte of the tile data
            ppu_read(tiledata_addr, tile_data_low);
            // Generate the pixel data for the low byte of the tile, and store in the pixel buffer
            for (uint8_t i = 0; i < 8; i++) {
                uint8_t low_color = tile_data_low >> i & 1;
                pixel_buffer.push_back({low_color, bgp});
            }
            state = GetTileHigh;
            break;
        case GetTileHigh:
            // We can reuse the tiledata_addr from the previous state, just add 1 to it to get the high byte
            ppu_read(tiledata_addr + 1, tile_data_high);
            // Now generate the pixel data for the high byte
            for (uint8_t i = 0; i < 8; i++) {
                uint8_t high_color = (tile_data_high >> i & 1) << 1;
                // OR the high bit with the low bit
                pixel_buffer[i].color = pixel_buffer[i].color | high_color;
            }
            state = Idle;
            break;
        case Idle:
            idle_complete = true;
            break;
    }
}

bool BG_Fetcher::ppu_read(uint16_t addr, uint8_t &data) {
    // Call the cpu_read function from the PPU
    return ppu->cpu_read(addr, data, true);
}
std::vector<Pixel_BG> BG_Fetcher::get_pixels() {
    // Create a copy of the pixel data to return to the PPU
    std::vector<Pixel_BG> out = pixel_buffer;
    // Empty the pixel array
    pixel_buffer = {};
    // Increment the pixel count by 8
    pixel_count += 8;
    state = GetTileId;
    tile_index++;
    // Reset the idle complete flag once we have grabbed the pixels in the pixel buffer
    idle_complete = false;

    return out;
}

bool BG_Fetcher::pixels_ready() {
    if (pixel_buffer.size() == 8 && idle_complete)
        return true;
    return false;
}

