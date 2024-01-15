#include <cassert>
#include "DMG_PPU.h"
#include "Bus.h"

DMG_PPU::DMG_PPU() {
    // Clear VRAM
    for (uint8_t i : vram) i = 0x00;
    // Clear OAM
    for (uint8_t i : oam) i = 0x00;
    // Override default constructor - maybe the fetchers should be pointers?

    bg_fetcher = BG_Fetcher(this);
    fg_fetcher = FG_Fetcher(this);
    // Default state of the PPU
    state = OAMSearch;
    // Initialize registers
    lcdc.data = 0x91;
    stat.data = 0x85;
}

// Need to add checks for when the PPU is accessing RAM directly since the
// function will need to not write anything at all
bool DMG_PPU::cpu_write(uint16_t addr, uint8_t data, bool is_dma) {
    // Check for VRAM
    // Will need to have 2 different cpu access conditions since during OAM search VRAM is accessible,
    // but OAM is not
    if(addr >= 0x8000 && addr <= 0x9fff) {
        // Check if ppu is using VRAM
        if (!cpu_access && !is_dma) {
            printf("Cannot write \"0x%x\" to 0x%x. VRAM is being used by PPU.\n",data, addr);
            return false;
        }
        // Mask the passed address so it is offset to 0x0000
        vram[addr - 0x8000] = data;
        return true;
    }
    // Check for OAM
    else if(addr >= 0xfe00 && addr <= 0xfe9f) {
        oam[addr - 0xfe00] = data;
        return true;
    }
    // Check for LCDC register
    else if (addr == 0xff40) {
        if ((data & 80) == 80) {
            ppu_on_flag = true;
        }
        else if ((data & 80) == 0) {
            ppu_on_flag = false;
        }
        lcdc.data = data;
        return true;
    }
    // Check for STAT register
    else if (addr == 0xff41) {
        // Only bits 3-6 are writable, need to mask the data write to this register
        stat.data |= data & 0b01111000;
        return true;
    }
    // Check for write to SCY register
    else if (addr == 0xff42) {
        scy = data;
        return true;
    }
    // Check for write to SCX register
    else if (addr == 0xff43) {
        scx = data;
        return true;
    }
    // Check for write to LYC register
    else if (addr == 0xff45) {
        lyc = data;
        return true;
    }
    // Check if writing to BGP
    else if (addr == 0xff47) {
        bgp = data;
        return true;
    }
    // Check if writing to OBP0
    else if (addr == 0xff48) {
        obp0 = data;
        return true;
    }
    // Check if writing to OBP1
    else if (addr == 0xff49) {
        obp1 = data;
        return true;
    }
    // Check for write to WY register
    else if (addr == 0xff4a) {
        wy = data;
        return true;
    }
    // Check for write to WX register
    else if (addr == 0xff4b) {
        wx = data;
        return true;
    }
    return false;
}

// Need to add checks for when the PPU is accessing RAM directly since the
// function will need to return 0xff
// Will need to have 2 different cpu access conditions since during OAM search VRAM is accessible,
// but OAM is not
bool DMG_PPU::cpu_read(uint16_t addr, uint8_t &data, bool is_fetcher) {

    // Check if reading VRAM
    if(addr >= 0x8000 && addr <= 0x9fff) {
        /*
         * If the PPU is using VRAM, then the CPU will not have access. However, if a pixel fetcher
         * is trying to read VRAM, it should have access. is_fetcher is default set to false, and the
         * CPU will not use that parameter. Therefore, only the fetchers can bypass read lock out,
         * as they call the function with is_fetcher set to true.
         */
        if (!cpu_access && !is_fetcher) {
            data = 0xff;
            return false;
        }
        // Mask the passed address so it is offset to 0x0000
        data = vram[addr - 0x8000];
        return true;
    }
    // Check if reading OAM
    else if(addr >= 0xfe00 && addr <= 0xfe9f) {
        // Check if PPU is using OAM
        if (!cpu_access || state == OAMSearch) {
            data = 0xff;
            return false;
        }
        data = oam[addr - 0xfe00];
        return true;
    }
    // Check for LCDC register
    else if (addr == 0xff40) {
        data = lcdc.data ;
        return true;
    }
    // Check for STAT register
    else if (addr == 0xff41) {
        data = stat.data;
        return true;
    }
    // Check for SCY register
    else if (addr == 0xff42) {
        data = scy;
        return true;
    }
    // Check for SCX register
    else if (addr == 0xff43) {
        data = scx;
        return true;
    }
    // Check for LY register read
    else if (addr == 0xff44) {
        data = (uint8_t)ly;
        return true;
    }
    // Check for LYC register read
    else if (addr == 0xff45) {
        data = lyc;
        return true;
    }
    // Check if reading the BGP
    else if (addr == 0xff47) {
        data = bgp;
        return true;
    }
    // Check if reading OBP0
    else if (addr == 0xff48) {
        data = obp0;
        return true;
    }
    // Check if reading OBP1
    else if (addr == 0xff49) {
        data = obp1;
        return true;
    }
    // Check for WY register
    else if (addr == 0xff4a) {
        data = wy;
        return true;
    }
    // Check for WX register
    else if (addr == 0xff4b) {
        data = wx;
        return true;
    }
    return false;
}
// https://blog.tigris.fr/2019/09/15/writing-an-emulator-the-first-pixel/
// This website was instrumental to the creation of this clock function

// RE-WRITE
void DMG_PPU::clock() {
    frame_complete = false;
    if (clock_count == 70224) {
        frame_complete = true;
        clock_count = 0;
    }
    if (ppu_on_flag) {
        if (bus->cpu.cycles == 0) {
            ppu_on_flag = false;
        }
        clock_count = 0;
        return;
    }
    // If screen is off give cpu access to VRAM
    // TODO - Review behaviour of GB when the screen is turned off.
    if (lcdc.lcd_ppu_enable == 0) {
        cpu_access = true;
        ly = 0;
        stat.mode_flag = 0;
        //lcdc.obj_enable = 0;
        //lcdc.bg_window_enable = 0;
        //clock_count = 0;
        state = OAMSearch;
        clock_count++;
        return;
    }
    // PPU modes
    switch (state) {
        case OAMSearch:
            // CPU has access to VRAM, not OAM
            cpu_access = true;
            // Set the stat mode flag to 2
            stat.mode_flag = 2;
            if (clock_count == 0)
                // Set the OAM mode flag, this is used to detect if the IF bit
                // needs to be set in the bus clock function.
                stat_oam_flag = true;
            if (clock_count == 2) {
                stat_oam_flag = false;
            }
            // Wait until the end of OAM search to grab the sprites from OAM
            if (clock_count == 80) {
                // Disable CPU access
                cpu_access = false;
                // Scan OAM for sprites that have the same Y position as LY.
                // First check if sprites are enabled, and if 8x16 mode is enabled
                if (lcdc.obj_enable == 1 && lcdc.obj_size != 1) {
                    // 8x8 sprites
                    // Clear the current list of scanned sprites
                    fg_fetcher.clear_sprites();
                    /*
                     * Each object in OAM is 4 bytes, with the first byte
                     * being the Y position. Sprites need to have a Y >= 16 in order
                     * to be considered, as anything lower cannot be seen on the screen.
                     */
                    for (uint8_t i = 0; i < oam.size(); i+=4) {
                        if (oam[i] >= 16) {
                            // Remove the 16 offset from the sprite so that is in line with LY
                            uint8_t current_sprite_y = oam[i] - 16;
                            /* This checks to see if the current sprite has a Y position that is equal to LY,
                             * or within +8 of its top left pixel. This is because OAM contains the Y of the top left
                             * pixel.
                             *
                             * The PPU cycles through OAMSearch, PixelTransfer, and HBlank every scanline, so this is
                             * checked every scanline. The current_sprite_y + 8 assures that the pixel data is pulled
                             * for all 8 lines of the tile.
                             */
                            if ((uint8_t)ly >= current_sprite_y && (uint8_t)ly < (current_sprite_y + 8)) {
                                /*
                                 * This line adds sprites to the scanned_sprites vector. This vector contains 10 sprites
                                 * which is all sprites that are on the current scanline. This will be used by the
                                 * sprite fetcher to grab the relevant pixel data as it fetches sprite pixels.
                                 */
                                if (fg_fetcher.get_sprite_count() < 10) {
                                    fg_fetcher.emplace_sprite(
                                            Sprite(i / 4, oam[i], oam[i + 1], oam[i + 2],
                                                    (ly - current_sprite_y) % 8,oam[i + 3]));
                                }
                            }
                        }
                    }
                    // Sort the scanned sprites in draw priority order
                    fg_fetcher.sort_sprites();
                }
                else if (lcdc.obj_enable == 1 && lcdc.obj_size == 1) {
                    // 8x16 sprites
                    fg_fetcher.clear_sprites();
                    for (uint8_t i = 0; i < oam.size(); i+=4) {
                        if (oam[i] >= 16) {
                            // Remove Y offset
                            uint8_t current_sprite_y = oam[i] - 16;
                            // Need to make sure that a 8x16 tile is acknowledged for all 16 scanlines
                            if ((uint8_t)ly >= current_sprite_y && (uint8_t)ly < (current_sprite_y + 16)) {
                                // Need to check if we are on the first or second tile
                                uint8_t current_sprite_y_flip = oam[i + 3];
                                if ((uint8_t)ly < current_sprite_y + 8) {
                                    // First tile
                                    // Now check the Y-flip bit and select the correct tile
                                    // Check if bit 6 is enabled in the attribute byte
                                    if ((current_sprite_y_flip & 0x40) == 0x40) {
                                        if (fg_fetcher.get_sprite_count() < 10) {
                                            fg_fetcher.emplace_sprite(
                                                    Sprite(i / 4, oam[i], oam[i + 1], oam[i + 2] + 1,
                                                           (ly - current_sprite_y) % 8,oam[i + 3]));
                                        }
                                    }
                                    else {
                                        if (fg_fetcher.get_sprite_count() < 10) {
                                            fg_fetcher.emplace_sprite(
                                                    Sprite(i / 4, oam[i], oam[i + 1], oam[i + 2],
                                                           (ly - current_sprite_y) % 8,oam[i + 3]));
                                        }
                                    }
                                }
                                else {
                                    // Second tile
                                    // Now check the Y-flip bit
                                    if ((current_sprite_y_flip & 0x40) == 0x40) {
                                        if (fg_fetcher.get_sprite_count() < 10) {
                                            fg_fetcher.emplace_sprite(
                                                    Sprite(i / 4, oam[i], oam[i + 1], oam[i + 2],
                                                           (ly - current_sprite_y) % 8,oam[i + 3]));
                                        }
                                    }
                                    else {
                                        if (fg_fetcher.get_sprite_count() < 10) {
                                            fg_fetcher.emplace_sprite(
                                                    Sprite(i / 4, oam[i], oam[i + 1], oam[i + 2] + 1,
                                                           (ly - current_sprite_y) % 8,oam[i + 3]));
                                        }

                                    }
                                }
                            }
                        }
                        // Sort the scanned sprites in draw priority order
                        fg_fetcher.sort_sprites();
                    }
                }
            }
            // OAM search takes 80 clock cycles to complete
            if (clock_count == 80) {
                // Prep for PixelTransfer state
                bg_fetcher.init();
                // Grab the swap position
                bg_win_swap_pos = bg_fetcher.get_swap_pos();
                // Grab the pixel offset
                win_pixel_offset = bg_fetcher.get_win_pixel_offset();
                bg_pixel_offset = bg_fetcher.get_bg_pixel_offset();
                // Clear the FIFOs
                clear_fifos();
                // Reset all
                scanline_x = 0;
                sprite_offscreen_offset = 0;
                // Assume normal pt mode
                pt_mode = Normal;
                state = PixelTransfer;
                break;
            }
            clock_count++;
            break;

        case PixelTransfer:
            // Set the stat flag
            stat.mode_flag = 3;
            if (clock_count == 80)
                stat_pixel_transfer_flag = true;
            // Check to see if the all pixels have been pushed to the screen
            if (scanline_x >= 160) {
                // Change the state of the PPU to HBlank
                stat_pixel_transfer_flag = false;
                state = HBlank;
                old_clock = clock_count;
                break;
            }

            // Execute a bg_fetcher clock
            bg_fetcher.clock();

            // Check to see if the bg_fetcher has 8 pixels, if so, grab them and start pushing to the screen buffer
            if (bg_fetcher.pixels_ready() && bg_fifo.empty()) {
                // Reset the sprite offscreen offset
                sprite_offscreen_offset = 0;
                // If the fetcher has pixels, then push them into the fifo
                // Grab the pixels from the fetcher
                std::vector<Pixel_BG> pixels = bg_fetcher.get_pixels();
                // Push the pixels to the FIFO in reverse order as they are stored in reverse
                for (int8_t i = 7; i >= 0; i--) {
                    bg_fifo.push(pixels[i]);
                }
            }
            // Determine the PixelTransfer mode
            // Check to see if there are any objects on the current scanline, and set the PT_mode
            if (fg_fetcher.get_sprite_count() > 0 && !bg_fifo.empty()) {
                // Flag is set when get_sprite_count returns 0
                bool sprite_list_empty = false;
                // First check to see if there are any sprites on the current scanline
                if (fg_fetcher.get_sprite_count() > 0) {
                    Sprite current_sprite = fg_fetcher.get_front_sprite();
                    // Now check to see if the sprite's X-position is 0
                    if (current_sprite.x_pos == 0) {
                        // Sprites with an X pos of zero can be popped off and ignored
                        // Need to loop until the front sprite.x_pos != 0
                        bool end_loop = false;
                        do {
                            fg_fetcher.pop_sprite();
                            if (fg_fetcher.get_sprite_count() == 0) {
                                sprite_list_empty = true;
                                end_loop = true;
                            }
                            else {
                                current_sprite = fg_fetcher.get_front_sprite();
                                if (current_sprite.x_pos > 0) {
                                    end_loop = true;
                                }
                            }
                        } while(!end_loop);
                    }
                    // Check to see if the sprite will have an offscreen offset at the beginning of the scanline
                    if (current_sprite.x_pos > 0 && current_sprite.x_pos < 8 && scanline_x == 0 && !sprite_list_empty) {
                        sprite_offscreen_offset = 8 - current_sprite.x_pos;
                        pt_mode = ObjFetch;
                    }
                    // Check to see if a sprite should be fetched: sprite x has an offset of 8 pixels
                    else if (current_sprite.x_pos - 8 == scanline_x && !sprite_list_empty) {
                        pt_mode = ObjFetch;
                    }
                }
            }

            switch (pt_mode) {
                case Normal:

                    /* Before trying to push pixels from the FIFO to the screen, check to see if the FIFO is empty.
                     * If it's not, then we can try to push pixels to the screen.
                     */
                    // Make sure there are pixels in the fifo
                    if (!bg_fifo.empty()) {
                        // Pop off bg offset pixels
                        for (uint8_t i = 0; i < bg_pixel_offset; i++){
                            bg_fifo.pop();
                        }
                        // Pop off any pixels needed if the window is at an offset
                        if (bg_fetcher.fetcher_mode_win()) {
                            for (uint8_t i = 0; i < win_pixel_offset; i++)
                                bg_fifo.pop();
                        }
                        // Make sure the bg_fifo is not empty before trying to push a pixel to the screen
                        if (!bg_fifo.empty()) {
                            // Now see if there are pixels in the fg_fifo
                            if (!fg_fifo.empty()) {
                                // Now we need to select the pixel that will be drawn to the screen
                                // fg pixel is not transparent and has priority over bg/win
                                if (fg_fifo.front().color != 0 && fg_fifo.front().bg_priority != 1 && fg_fifo.front().x_pos - 8 == scanline_x) {
                                    push_mode = Object;
                                }
                                // fg pixel is not transparent and does not have priority over bg/win
                                else if (fg_fifo.front().color != 0 && fg_fifo.front().bg_priority == 1 && fg_fifo.front().x_pos - 8 == scanline_x) {
                                    // Only push the fg pixel when the bg fetcher color is 0
                                    if (map_color(bg_fifo.front().color, bgp) == 0){
                                        push_mode = Object;
                                    }
                                    else
                                        push_mode = BGWin;
                                }
                                else
                                    push_mode = BGWin;
                            }
                            else
                                push_mode = BGWin;
                            switch (push_mode) {
                                case BGWin:
                                    push_pixel(map_color(bg_fifo.front().color, bg_fifo.front().palette), scanline_x + (ly * 160));
                                    bg_fifo.pop();
                                    if (!fg_fifo.empty())
                                        fg_fifo.pop();
                                    scanline_x++;
                                    break;
                                case Object:
                                    push_pixel(map_color(fg_fifo.front().color, fg_fifo.front().palette), scanline_x + (ly * 160));
                                    bg_fifo.pop();
                                    fg_fifo.pop();
                                    scanline_x++;
                                    break;
                            }

                        }
                        // Change the offset to 0 now that we have pushed the pixels to the screen
                        win_pixel_offset = 0;
                        bg_pixel_offset = 0;
                    }
                    break;

                    // Need to implement the FG_Fifo and fetcher

                    /*
                     * The FG_Fifo operates very differently to the BG_Fifo due to the way it requests pixels from
                     * its fetcher. The FG_Fetcher will begin fetching pixels only when a sprite should be drawn to the
                     * screen. This means that pixels are fetched when scanline_x is equal to a sprite's x-pos on the
                     * current scanline.
                     *
                     * When the FG_Fetcher begins fetching a slice of pixels for a sprite, it will stall the BG and FG
                     * fifos. Once pixels have been fetched, they are merged with the pixels currently in the fifo, unless
                     * it's empty. In this case they are directly pushed into the FG fifo.
                     *
                     * Once the pixels have been merged into the FG fifo, another check will need to occur.
                     *
                     * To begin a fetch, here in the PPU clock function we need to check the front of the scanned_sprite
                     * vector, which is a member of the FG_Fetcher class. We then check the X pos of the front sprite to see
                     * if it is equal to scanline_x.
                     *
                     * Note: the X-pos has an offset of +8 i.e. for the sprite to be visible, it's X must be 8 <= X <= 160.
                     * Keep in mind that the X position could be 1, and though most of the sprite will not be visible,
                     * one pixel theoretically could be, so we need to account for that possibility.
                     *
                     * Even when we fetch a pixel slice, we still need to check the sprite list to see if the next
                     * sprite has it's X pos as the same as the current scanline. This is because there could be overlapping
                     * sprites, and any transparent pixel that was initially fetched could be overwritten by a sprite that
                     * is overlapping.
                     *
                     * scanned_sprites is sorted by object priority. The sprite at the front has the highest object
                     * priority, so in the case of two sprites overlapping, the first one that was fetched will retain all
                     * non-transparent pixels.
                     *
                     */
                case ObjFetch:
                    //printf("pt_mode is ObjFetch.\n");
                    // Clock the fg_fetcher
                    fg_fetcher.clock();
                    // Check to see if the fetcher has object pixels ready
                    /* *Note: Pixels can be pushed to the fg_fifo even if it is full, however not all pixels
                     * that are fetched will be pushed to the fifo if it already has pixel data. Pixels are fetched
                     * based on object priority, so pixels that have already been fetched and pushed to the fifo will
                     * have priority over pixels that have just been fetched. This is where we will need to merge the
                     * existing pixels with the fetched ones. We will also need to see if the pt_mode needs to change
                     * from ObjFetch to Normal.
                     *
                     * Before changing back to Normal, we need to check the front of the scanned sprite list again.
                     * When using the fg_fetcher.get_pixels() function, we pop off the front sprite from the
                     * scanned_sprites deque. We can now check the front of scanned_sprites to see if the following
                     * sprite shares the same X-pos. If it does, then we need to fetch that sprite and merge the pixels
                     * accordingly. This will assure that we do not miss any sprite pixels that should be rendered over
                     * the existing sprites transparent pixels.
                     */

                    // First sprite fetched, fifo is empty
                    if (fg_fetcher.pixels_ready() && fg_fifo.empty()) {
                        // Grab pixels from the pixel buffer
                        std::vector<Pixel_FG> pixels = fg_fetcher.get_pixels();
                        // Push the pixels to the fg_fifo
                        for (int8_t i = 7; i >= 0; i--) {
                            fg_fifo.push(pixels[i]);
                        }
                        // Now check to see if there is an offscreen offset, and adjust the pixel fifo
                        if (sprite_offscreen_offset > 0) {
                            for (uint8_t i = 0; i < sprite_offscreen_offset; i++) {
                                // Pop unneeded pixels from the fg_fifo
                                fg_fifo.pop();
                            }
                        }
                    }
                    // fifo is not empty, pixels need to be merged
                    else if (fg_fetcher.pixels_ready() && !fg_fifo.empty()) {
                        // Grab pixels from the pixel buffer
                        std::vector<Pixel_FG> pixels = fg_fetcher.get_pixels();
                        // Create a copy of the fifo in vector form so the fetched pixels can be merged
                        std::vector<Pixel_FG> fifo_vector_copy = {};
                        uint8_t fg_fifo_size = (uint8_t)fg_fifo.size();
                        for (auto i = 0; i < fg_fifo_size; i++) {
                            fifo_vector_copy.push_back(fg_fifo.front());
                            fg_fifo.pop();
                        }
                        // Now merge the fetched pixels with the fifo vector
                        for (uint8_t i = 0; i < fg_fifo_size; i++) {
                            // Compare the fetched pixels with the existing fifo pixels
                            if (fifo_vector_copy[i].color == 0 && pixels[7 - i].color != 0) {
                                // Over-write the exiting pixel when it is transparent and the new one is not
                                fifo_vector_copy[i] = pixels[7 - i];
                            }
                        }
                        // Push remaining pixels to the fifo copy
                        for (auto i = 7 - static_cast<int8_t>(fg_fifo_size); i >= 0 ; i--) {
                            fifo_vector_copy.push_back(pixels[i]);
                        }
                        // Over-write the fg_fifo with the new one that has been merged with new pixels
                        for (const auto &v : fifo_vector_copy)
                            fg_fifo.push(v);
                    }
                    // Check the front sprite to see if it has the same x-pos
                    if (fg_fetcher.get_sprite_count() == 0)
                        pt_mode = Normal;
                    else if (fg_fetcher.get_sprite_count() > 0) {
                        uint8_t new_sprite_x = fg_fetcher.get_front_sprite().x_pos - 8;
                        if (new_sprite_x != scanline_x)
                            pt_mode = Normal;
                    }
                    break;
            }

            clock_count++;
            break;
        case HBlank:
            // CPU has full access to VRAM
            cpu_access = true;
            /* Set the STAT flag for HBlank
             * This is set only on the first clock of the HBlank state. After the first clock, we check
             * what the next state of the PPU will be, and set the flag accordingly. This assures that
             * any program that uses the STAT mode flag will execute the associated code at the beginning
             * of HBlank.
             */
            stat.mode_flag = 0;
            if (clock_count == old_clock) {
                stat_hblank_flag = true;
                clock_count++;
                break;
            }
            // Check what the next PPU mode will be
            //else if (clock_count == 2 && ly + 1 == 144) {
                //stat.mode_flag = 0;
            //}
            // Set the mode flag to OAM search if the next mode will be OAMSearch
            //else
                //stat.mode_flag = 2;
            // Wait one clock cycle before resetting the HBlank flag
            if (clock_count == old_clock + 2)
                stat_hblank_flag = false;
            // Check if we need to advance to a new scanline
            if (clock_count == 456) {
                clock_count = 0;
                stat_hblank_flag = false;
                ly++;
                // Reset the LY LYC flag
                stat_ly_lyc_flag = false;
                stat.lyc_ly_flag = 0;
                // If ly = 144, change mode to VBlank
                if (ly == 144) {
                    state = VBlank;
                    break;
                }
                else {
                    state = OAMSearch;
                    cpu_access = false;
                    break;
                }
            }
            clock_count++;
            break;
        case VBlank:
            cpu_access = true;
            // Set the stat flag
            stat.mode_flag = 1;
            // Set the VBlank flag, this is used to detect interrupts
            if (clock_count == 0) {
                vblank_flag = true;
                clock_count++;
                break;
            }
            // Wait 1 clock cycle before turning off the VBlank flag
            if (clock_count == 2)
                vblank_flag = false;
            // Check if VBLANK is done - VBLANK is 10 scanlines
            if (clock_count % 456 == 0 && clock_count != 0) {
                // Increment the LY register
                ly++;
                // Reset the LYC LY flag
                stat_ly_lyc_flag = false;
                stat.lyc_ly_flag = 0;
                // Check to see if VBlank is done - VBlank is from ly 144 - 153
                if (ly == 154) {
                    // Need to fix so assert below does not crash
                    assert(clock_count == 4560);
                    clock_count = 0;
                    ly = 0;
                    state = OAMSearch;
                    frame_complete = true;
                    cpu_access = false;
                    vblank_flag = false;
                    break;
                }
            }
            clock_count++;
            break;
    }
}

void DMG_PPU::reset() {
    ly = 0;
    lyc = 0;
    scx = 0;
    scy = 0;
    wx = 0;
    wy = 0;
    bgp = 0;
    obp0 = 0;
    obp1 = 0;
    scanline_x = 0;
    old_clock = 0;
    bg_win_swap_pos = 0xff;
    win_pixel_offset = 0;
    bg_pixel_offset = 0;
    sprite_offscreen_offset = 0;
    clock_count = 0;
    cpu_access = false;
    fetcher_access = false;
    stat_oam_flag = false;
    stat_pixel_transfer_flag = false;
    stat_hblank_flag = false;
    stat_ly_lyc_flag = false;
    vblank_flag = false;
    ppu_on_flag = false;
    clear_fifos();
    vram = {};
    oam = {};
    bg_fetcher.init();
    fg_fetcher.init();
    lcdc.data = 0x91;
    stat.data = 0x85;
}

void DMG_PPU::clear_fifos() {
    std::queue<Pixel_BG> bg_empty;
    std::queue<Pixel_FG> fg_empty;
    std::swap(bg_fifo, bg_empty);
    std::swap(fg_fifo, fg_empty);
}

void DMG_PPU::set_fetcher_flag(bool val) {
    fetcher_access = val;
}

void DMG_PPU::push_pixel(uint8_t pixel, uint32_t index) {
    bus->push_pixel(pixel, index);
}

uint8_t DMG_PPU::map_color(uint8_t color, uint8_t palette) {    // The GB has 4 colors, 0-3
/*     Palette is contains 4 2-bit values

     The color parameter contains the index of the pixel's
     color in the palette.
     First we multiply the color by 2, or shift it left 1 (both are the same).
     Next we shift the palette right n bits. N (the result of color << 1).
     Color will be a value of 0-3, so we need to multiply it by 2 to get the number of
     bits to shift right in palette. This effectively selects the colpr
     from the palette, and sets it as the bit 0 & 1.
     We then & the result with 3, keeping the 2 lowest bits and discarding the rest */
    return (palette >> (color << 1)) & 0b11;
}