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
        vram[addr & 0x1fff] = data;
        return true;
    }
    // Check for OAM
    else if(addr >= 0xfe00 && addr <= 0xfe9f) {
        oam[addr & 0x009f] = data;
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
        data = vram[addr & 0x1fff];
        return true;
    }
    // Check if reading OAM
    else if(addr >= 0xfe00 && addr <= 0xfe9f) {
        // Check if PPU is using OAM
        if (!cpu_access) {
            data = 0xff;
            return false;
        }
        data = oam[addr & 0x009f];
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
            stat.mode_flag = 0;
            // Need to get the data for upto 10 sprites on this scanline
            if (clock_count == 40) {
                // Init pixel transfer
                pixel_count = 0;
                pixel_x = scx / 8;
                pixel_y = scy + ly;
                // Get the line of pixels to be rendered
                tile_line = pixel_y % 8;
                // Find the address to the current tile row in the tilemap

                // Need to determine if the current scanline should render window, or bg

                // For now assume 0x9800 is the tilemap
                // Need to mask the address:
                //  - Tile map is 0x7ff in size, and it starts 0x1800 from starting point of VRAM 0x8000
                tilemap_row_addr = (0x9800 & 0x7ff + 0x1800) + (pixel_y / 8 * 32); // <- ly / 8 * 32 gives us the Y position
                fetch.init(tilemap_row_addr, tile_line, pixel_x);

                state = PixelTransfer;
            }
            break;
        case PixelTransfer:
            // Clock the fetcher: Only clocks every 2 PPU clocks
            fetch.clock(this);

            // Need to call bus pixel push function if there are any in the fifo
            if(!fetch.fifo.empty()) {
                // Check if screen is enabled
                if (lcdc.lcd_ppu_enable != 0) {
                    // For now just push the color
                    // Need to map the color to a real color using the palette
                    bus->push_pixel(map_color(fetch.fifo.front().color, fetch.fifo.front().palette), pixel_count + (ly * 160));
                    // Pop off of the FIFO
                    fetch.fifo.pop();
                }

                // Increment the position of the pixel output
                pixel_count++;
            }
            if (pixel_count == 160){
                state = HBlank;
            }
            break;
        case HBlank:
            // Check to see if the entire scanline has been completed by looking at the number of clocks
            // 456 is the number of clocks required for the ppu to output a complete scanline of pixels
            // CPU is also able to access VRAM and OAM now
            cpu_access = true;
            if (clock_count == 456) {
                // We have now reached the end of the scanline
                clock_count = 0;
                // Increment the scanline register
                ly++;
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
