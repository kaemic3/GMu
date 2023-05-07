#include "DMG_PPU.h"
#include "Bus.h"

DMG_PPU::DMG_PPU() {
    // Clear VRAM
    for (uint8_t i : vram) i = 0x00;
    // Clear OAM
    for (uint8_t i : oam) i = 0x00;

    // Default state of the PPU
    state = OAMSearch;
    // Initialize registers
    lcdc = {0, 0 ,0 ,0 ,0 ,0, 0, 0};
    stat = {0, 0, 0, 0, 0, 0, 0};
}

// Need to add checks for when the PPU is accessing RAM directly since the
// function will need to not write anything at all
bool DMG_PPU::cpu_write(uint16_t addr, uint8_t data) {
    // Check for VRAM
    // Will need to have 2 different cpu access conditions since during OAM search VRAM is accessible,
    // but OAM is not
    if(addr >= 0x8000 && addr <= 0x9fff) {
        // Check if ppu is using VRAM
        if (!cpu_access) {
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
        lcdc.data = data;
        return true;
    }
    // Check for STAT register
    else if (addr == 0xff41) {
        stat.data = data;
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
bool DMG_PPU::cpu_read(uint16_t addr, uint8_t &data) {

    // Check if reading VRAM
    if(addr >= 0x8000 && addr <= 0x9fff) {
        // Check if PPU is using VRAM
        if (!cpu_access) {
            data = 0xff;
            return false;
        }
        // Mask the passed address so it is offset to 0x0000
        data = vram[addr & 0x8000];
        return true;
    }
    // Check if reading OAM
    else if(addr >= 0xfe00 && addr <= 0xfe9f) {
        // Check if PPU is using OAM
        if (!cpu_access) {
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
        data = ly;
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
    // Check for WYUregister
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

void DMG_PPU::clock() {
    frame_complete = false;
    // Need to make sure we are updating the stat mode flag
    switch (state) {
        case OAMSearch:
            // OAM search always takes 40 clocks to complete
            stat.mode_flag = 2;

            if (clock_count == 40) {
                // Need to get the data for upto 10 sprites on this scanline
                // Compare the Y pos of each sprite to the current scan line
                // Clear the sprite list from the fetcher

                // Need to make sure that the sprites are organized based on the draw order
                // Drawing priority for sprites is determined by their X coordinate first, and if 2 sprites
                // share the same X coordinate, then the sprite that is stored earlier in OAM will have priority
                // So, sprites should be stored in order of draw priority.

                // Check if OBJ is enabled
                if (lcdc.obj_enable == 1) {
                    fg_fetch.sprites.clear();
                    for (uint8_t i = 0; i < 160; i+=4) {
                        // Check to see if the Y value of the sprite is < 16
                        if (oam[i] >= 16) {
                            uint8_t sprite_y = oam[i] - 16;
                            if (sprite_y <= ly && ly < (sprite_y + 8)) {
                                if(fg_fetch.sprites.size() < 10) {
                                    fg_fetch.sprites.emplace_back(i/4, oam[i], oam[i + 1], oam[i + 2], (ly - sprite_y) % 8 ,oam[i + 3]);
                                }
                            }
                        }
                    }
                    // Sort the sprite list in draw priority order
                    std::sort(fg_fetch.sprites.begin(), fg_fetch.sprites.end());
                }

                // Init pixel transfer
                // Reset values for a new scanline
                pixel_count = 0;
                signed_mode = false;
                win_collision = false;
                win_collision_tile_offset = 0;
                win_collision_pos = 0xff;
                window_draw = false;
                swap_to_win = false;
                pop_request = false;

                // Set the tile data address - Window and BG share a tile data area
                switch (lcdc.bg_win_tile_data_area) {
                    // If lcdc.4 is not set, load 0x1000 into tiledata_addr and used signed addressing
                    case 0:
                        tiledata_addr = 0x1000;
                        signed_mode = true;
                        break;
                        // If lcdc.4 is set, load 0x0000 into tiledata_addr
                    case 1:
                        tiledata_addr = 0x0000;
                        break;
                }

                // The checks below are used to determine if the window needs to be rendered, and how
                // it needs to be rendered.

                // Check if the window can be ignored
                if(lcdc.win_enable == 1 && wx <= 166 && wy < 144 && (wy <= ly)) {
                    window_draw = true;
                    // Set the tilemap address
                    switch (lcdc.win_tile_map_area) {
                        case 0:
                            // If lcdc.6 not set, set the tilemap to 0x1800
                            tilemap_row_addr = 0x1800;
                            break;
                        case 1:
                            // If lcdc.6 is set, set the tilemap to 0x1c00
                            // + 1 to ignore the first tile
                            tilemap_row_addr = 0x1c00;
                            break;
                    }
                    // Draw the window ignoring the first tile in the tile map
                    if (wx == 0 || wx == 166) {
                    // Technically, wx = 0 and wx = 166 do not share the same behaviour, but
                    // for now they will. Recreating the bug will be tedious.

                        // Select the correct tilemap for the window
                        // In this case, we can completely ignore the bg for this scanline,
                        // and all future scan lines

                        // Set the tilemap address
                        tilemap_row_addr++;
                        // Now we need to add the offset for y-axis
                        tilemap_row_addr += (win_ly / 8 * 32);
                        // Determine the tile_line using only ly
                        tile_line = win_ly % 8;
                        // Set pixel_x
                        pixel_x = 0;
                    }
                    // This section determines if we need to draw the first window tile at an offset
                    else if (wx > 0 && wx < 7) {
                        // Grab the correct tilemap row, tile line, and set the pixel_x offset
                        tilemap_row_addr += (win_ly / 8 * 32);
                        tile_line = win_ly % 8;
                        pixel_x = 0;

                        // Tell the ppu to pop off 7-wx pixels before drawing to the screen.
                        // This will offset the first tile accordingly
                        pop_request = true;
                        pop_win = 7 - wx;
                    }
                    // Pre-staging for a normal window position. This is where most windows will be positioned.
                    else if (wx >= 7 && wx <= 165) {
                        // No collision if the window is at WCX = 7
                        if (wx == 7) {
                            // Now we need to add the offset for y-axis
                            tilemap_row_addr += (win_ly / 8 * 32);
                            // Determine the tile_line using only ly
                            tile_line = win_ly % 8;
                            // Set pixel_x
                            pixel_x = 0;
                        }
                        // Window tiles are being swapped to during the screen draw from BG tiles
                        // This means:
                        // - The tilemap swapped
                        // - Tile data area will be the same
                        else {
                            // Set the collision flag
                            win_collision = true;
                            // Calculate the pixel where the collision will occur
                            // This will be used to swap the tilemap over in the fetcher.
                            win_collision_pos = (wx - 7);
                            // Calculate how many pixels need to be popped off of the fifo (if any)
                            // when the collision occurs only if there is a mid-tile collision
                            if ((wx - 7) % 8 != 0)
                                win_collision_tile_offset = 8 - ((wx - 7) % 8);
                            else
                                // If the collision does not happen mid-tile, then the FIFO will not need to pop
                                // off any excess pixels
                                win_collision_tile_offset = 0;
                            // We grab where the tilemap row should be for this scanline, and save it into a
                            // member separate from tilemap_row_addr, so it can be passed over to the fetcher
                            // when it needs to swap tilemaps.
                            win_tilemap_addr = tilemap_row_addr + (win_ly / 8 * 32);

                            // Setup for BG drawing first, PPU will determine when to swap over to the window
                            // Check for BG tilemap
                            switch (lcdc.bg_tile_map_area) {
                                case 0:
                                    tilemap_row_addr = 0x1800;
                                    break;
                                case 1:
                                    tilemap_row_addr = 0x1c00;
                                    break;
                            }
                            // Setup offsets for BG
                            pixel_x = ((scx / 8) + pixel_count) & 0x1f;
                            pixel_y = scy + ly;
                            tile_line = pixel_y % 8;
                            tilemap_row_addr += (pixel_y / 8 * 32);
                        }
                    }
                }
                else {
                    // Ignore the window
                    pixel_x = (scx / 8) + pixel_count & 0x1f;
                    pixel_y = (scy + ly) & 0xff;
                    // Find the address to the current tile row in the tilemap
                    tile_line = pixel_y % 8;

                    // Check for BG tilemap
                    switch (lcdc.bg_tile_map_area) {
                        case 0:
                            tilemap_row_addr = 0x1800;
                            break;
                        case 1:
                            tilemap_row_addr = 0x1c00;
                            break;
                    }
                    tilemap_row_addr += (pixel_y / 8 * 32);
                }
                // Initialize the fetchers for this scanline
                bg_fetch.init(tilemap_row_addr, tiledata_addr, tile_line, pixel_x, signed_mode, win_tilemap_addr);
                fg_fetch.init();
                state = PixelTransfer;
            }
            break;
        case PixelTransfer:
            stat.mode_flag = 3;
            // Clock the fetchers: Only clocks every 2 PPU clocks
            bg_fetch.clock(this, swap_to_win, tile_line, pixel_x);
            fg_fetch.clock(this);

            // Check if we need to pop off pixels from the window (if wx < 7)
            if (!bg_fetch.fifo.empty() && pop_request) {
                for (uint8_t pixel = pop_win; pixel > 0; pixel--) {
                    bg_fetch.fifo.pop();
                    pop_win--;
                }
            }
            // Need to check if there was a collision between the BG and the window.
            // This will be true when there is a collision, and when the current pixel is the same as the starting
            // pixel of the window with it's +7 offset removed.
            if (!bg_fetch.fifo.empty() && win_collision && pixel_count == win_collision_pos && win_collision_tile_offset > 0) {
                for (uint8_t pixel = win_collision_tile_offset; pixel > 0; pixel--) {
                    // Pop off residual BG tiles
                    bg_fetch.fifo.pop();
                    win_collision_tile_offset--;
                }
                // Set the window swap flag
                swap_to_win = true;
                // Determine the tile_line for the window
                tile_line = win_ly % 8;
                // Set pixel_x offset
                pixel_x = 0;
            }
            // BG/Win FIFO done

            // Need to call bus pixel push function if there are any in the fifo
            if(!bg_fetch.fifo.empty()) {
                // Check if the sprite fetcher has pixels
                if (!fg_fetch.fifo.empty()) {
                    // Grab the current sprites X pos
                    uint8_t current_sprite_x = fg_fetch.fifo.front().x_pos;
                    // If the current sprite pixel is less than 8, pop it off
                    while (current_sprite_x < 8) {
                        fg_fetch.fifo.pop();
                        current_sprite_x = fg_fetch.fifo.front().x_pos;
                    }
                    // Set the current sprite x again
                    //current_sprite_x = fg_fetch.fifo.front().x_pos;
                    uint8_t current_sprite_color = map_color(fg_fetch.fifo.front().color, fg_fetch.fifo.front().palette);
                    uint8_t current_sprite_priority = fg_fetch.fifo.front().bg_priority;

                    if (current_sprite_x >= 8) {
                        // Remove the offset from the sprite X
                        current_sprite_x -= 8;
                        // Need to check if the current sprite needs to be drawn with the BG priority not set
                        // and the color not 0

                        // Push a sprite pixel to the screen if it has priority over the BG and the pixel matches the
                        // X pos. Check for if the pixel color is transparent.
                        if (current_sprite_x == pixel_count && current_sprite_color != 0 && current_sprite_priority == 0) {
                            bus->push_pixel(current_sprite_color, pixel_count + (ly * 160));
                            fg_fetch.fifo.pop();
                            bg_fetch.fifo.pop();
                            sprite_pushed = true;
                        }
                        // If the current pixel is transparent then pop it off of the fifo but don't push it to the
                        // screen
                        else if (current_sprite_x == pixel_count && current_sprite_color == 0 && current_sprite_priority == 0) {
                            // First pop the transparent pixel
                            fg_fetch.fifo.pop();
                            // First check the entire fifo and push a pixel to the screen if it's x matches the
                            // current pixel_count
                            // Create a temporary fifo that we will store the pixels that are needed still in
                            std::queue<Pixel_FG> temp_fg_fifo;
                            uint8_t size = fg_fetch.fifo.size();
                            for(uint8_t i = 0; i < size; i++) {
                                current_sprite_x = fg_fetch.fifo.front().x_pos - 8;
                                current_sprite_color = fg_fetch.fifo.front().color;
                                current_sprite_priority = fg_fetch.fifo.front().bg_priority;
                                // Check the entire sprite fifo to see if any of the pixels should be drawn where the
                                // current transparent sprite pixel is.
                                if (current_sprite_x == pixel_count && current_sprite_color != 0) {
                                    // If the BG has priority then we need to check if the BG should be drawn instead
                                    if (current_sprite_priority == 1) {
                                        uint8_t bg_color = bg_fetch.fifo.front().color;
                                        // If the BG color is 0, then sprites will have priority
                                        if (bg_color == 0) {
                                            bus->push_pixel(current_sprite_color, pixel_count + (ly * 160));
                                            fg_fetch.fifo.pop();
                                            bg_fetch.fifo.pop();
                                            sprite_pushed = true;
                                        }
                                        // Otherwise, ignore the pixel
                                        else {
                                            // We will still push that pixel to the temp fifo since it will need
                                            // to be re-evaluated
                                            temp_fg_fifo.push(fg_fetch.fifo.front());
                                            fg_fetch.fifo.pop();
                                        }
                                    }
                                    // Otherwise the sprite will have priority, so push its pixel
                                    else {
                                        bus->push_pixel(current_sprite_color, pixel_count + (ly * 160));
                                        fg_fetch.fifo.pop();
                                        bg_fetch.fifo.pop();
                                        sprite_pushed = true;
                                    }
                                }
                                // Push the sprite to the temp fifo, which will then be swapped with the real one
                                // after each pixel has been checked
                                else {
                                    // Push to the temp fifo
                                    temp_fg_fifo.push(fg_fetch.fifo.front());
                                    // Pop the original fifo
                                    fg_fetch.fifo.pop();
                                }
                            }
                            // Swap the fifos
                            std::swap(fg_fetch.fifo, temp_fg_fifo);
                        }
                        // Check if the current sprite pixel should be BG/Win based on the sprite attributes
                        // Byte 3 bit 7 is set
                        else if (current_sprite_x == pixel_count && current_sprite_color != 0 && current_sprite_priority == 1) {
                            // If the front of the BG fifo has a pixel of color 0, then push a sprite pixel,
                            // otherwise, push a bg pixel
                            uint8_t bg_color = bg_fetch.fifo.front().color;
                            if (bg_color == 0) {
                                bus->push_pixel(current_sprite_color, pixel_count + (ly * 160));
                                fg_fetch.fifo.pop();
                                bg_fetch.fifo.pop();
                                sprite_pushed = true;
                            }
                            else {
                                fg_fetch.fifo.pop();
                            }
                        }
                        // Check to see if the current X is less than pixel_count
                        else if (current_sprite_x < pixel_count) {
                            // Flag
                            bool less = true;
                            // Pop the pixel off of the fifo
                            while (less) {
                                current_sprite_x = fg_fetch.fifo.front().x_pos - 8;
                                if (current_sprite_x < pixel_count  && !fg_fetch.fifo.empty()) {
                                    fg_fetch.fifo.pop();
                                }
                                else
                                    less = false;
                            }
                            // Check to see if the new pixel (after popping off pixels that did not have draw priority)
                            // should be drawn now
                            current_sprite_x = fg_fetch.fifo.front().x_pos - 8;
                            current_sprite_color = fg_fetch.fifo.front().color;
                            current_sprite_priority = fg_fetch.fifo.front().bg_priority;
                            if (current_sprite_x == pixel_count && current_sprite_priority == 0) {
                                // Push a pixel if the x_pos is equal to the pixel_count
                                bus->push_pixel(current_sprite_color, pixel_count + (ly * 160));
                                fg_fetch.fifo.pop();
                                bg_fetch.fifo.pop();
                                sprite_pushed = true;
                            }
                        }
                    }
                }
                // Check if screen is enabled
                if (lcdc.lcd_ppu_enable != 0 && !sprite_pushed) {
                    // For now just push the color
                    // Need to map the color to a real color using the palette
                    bus->push_pixel(map_color(bg_fetch.fifo.front().color, bg_fetch.fifo.front().palette), pixel_count + (ly * 160));
                    // Pop off of the FIFO
                    bg_fetch.fifo.pop();
                }
                // Increment the position of the pixel output only if the fifo is not empty
                pixel_count++;

            }
            // Reset flag
            sprite_pushed = false;
            if (pixel_count == 160){
                state = HBlank;
            }
            break;
        case HBlank:
            stat.mode_flag = 0;
            // Check to see if the entire scanline has been completed by looking at the number of clocks
            // 456 is the number of clocks required for the ppu to output a complete scanline of pixels
            // CPU is also able to access VRAM and OAM now
            cpu_access = true;
            if (clock_count == 456) {
                // We have now reached the end of the scanline
                clock_count = 0;
                // Increment the scanline register
                ly++;
                // Check if window is drawing
                if(window_draw)
                    win_ly++;
                // If the new scanline is 144, then switch to VBlank
                if (ly == 144)
                    state = VBlank;
                else {
                    // Restart the pixel drawing process
                    state = OAMSearch;
                    cpu_access = false;
                }

            }
            break;
        case VBlank:
            // Set the stat flag
            stat.mode_flag = 1;
            // Pause and allow for CPU to access VRAM and OAM
            cpu_access = true;
            // Check to see if the scanline is complete
            if (clock_count == 456) {
                // Reset clock count for new scanline
                clock_count = 0;
                // Increment the scanline register
                ly++;
                // If we have reached the last scanline for VBlank, return to the start of the pixel drawing process
                if (ly == 153) {
                    ly = 0;
                    win_ly = 0;
                    state = OAMSearch;
                    frame_complete = true;
                    cpu_access = false;
                }
            }
            break;
        default:
            break;
    }
    // Increment the clock count at the end of the clock call
    clock_count++;
    // If screen is off give cpu access to VRAM
    if(lcdc.lcd_ppu_enable == 0)
        cpu_access = true;
}

uint8_t DMG_PPU::map_color(uint8_t color, uint8_t palette) {
    // The GB has 4 colors, 0-3
    // Palette is contains 4 2-bit values

    // The color parameter contains the index of the pixel's
    // color in the palette.
    // First we multiply the color by 2, or shift it left 1 (both are the same).
    // Next we shift the palette right n bits. N (the result of color << 1).
    // Color will be a value of 0-3, so we need to multiply it by 2 to get the number of
    // bits to shift right in palette. This effectively selects the colpr
    // from the palette, and sets it as the bit 0 & 1.
    // We then & the result with 3, keeping the 2 lowest bits and discarding the rest.
    return (palette >> (color << 1)) & 0b11;
}