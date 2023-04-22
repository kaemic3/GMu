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
    if(addr >= 0x8000 && addr <= 0x9fff) {
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
bool DMG_PPU::cpu_read(uint16_t addr, uint8_t &data) {
    // Check if reading VRAM
    if(addr >= 0x8000 && addr <= 0x9fff) {
        // Mask the passed address so it is offset to 0x0000
        data = vram[addr & 0x1fff];
        return true;
    }
    // Check if reading OAM
    else if(addr >= 0xfe00 && addr <= 0xfe9f) {
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
    //if (lcdc.data == 0x00 || lcdc.lcd_ppu_enable == 0)
        //return;
    frame_complete = false;
    // Need to make sure we are updating the stat mode flag
    switch (state) {
        case OAMSearch:
            stat.mode_flag = 0;
            // Need to get the data for upto 10 sprites on this scanline
            // This always takes 40 clocks to complete
            if (clock_count == 40) {
                // Init pixel transfer
                x = 0;
                // Get the line of pixels to be rendered
                tile_line = ly % 8;
                // Find the address to the current tile row in the tilemap
                // For now assume 0x9800 is the tilemap
                // Need to mask the address:
                //  - Tile map is 0x7ff in size, and it starts 0x1800 from starting point of VRAM 0x8000
                tilemap_row_addr = (0x9800 & 0x7ff + 0x1800) + (ly / 8 * 32); // <- ly / 8 * 32 gives us the Y position
                fetch.init(tilemap_row_addr, tile_line);

                state = PixelTransfer;
            }
            break;
        case PixelTransfer:
            // Clock the fetcher: Only clocks every 2 PPU clocks
            fetch.clock(this);

            // Need to call bus pixel push function if there are any in the fifo
            if(!fetch.fifo.empty()) {
                // For now just push the color
                bus->push_pixel(fetch.fifo.front().color, x + (ly * 160));
                // Pop off of the FIFO
                fetch.fifo.pop();
                // Increment the position of the pixel output
                x++;
            }
            if (x == 160){
                state = HBlank;
            }
            break;
        case HBlank:
            // Check to see if the entire scanline has been completed by looking at the number of clocks
            // 456 is the number of clocks required for the ppu to output a complete scanline of pixels
            // CPU is also able to access VRAM and OAM now
            if (clock_count == 456) {
                // We have now reached the end of the scanline
                clock_count = 0;
                // Increment the scanline register
                ly++;
                // If the new scanline is 144, then switch to VBlank
                if (ly == 144)
                    state = VBlank;
                else
                    // Restart the pixel drawing process
                    state = OAMSearch;
            }
            break;
        case VBlank:
            // Pause and allow for CPU to access VRAM and OAM

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
                }
            }
            break;
        default:
            break;
    }
    // Increment the clock count at the end of the clock call
    clock_count++;
}

void DMG_PPU::clear_fifo(std::queue<Pixel> &q) {
    // Make an empty queue
    std::queue<Pixel> empty;
    // Swap the contents of the queues
    std::swap(q, empty);
}
