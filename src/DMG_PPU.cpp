#include "DMG_PPU.h"

DMG_PPU::DMG_PPU() {
    // Clear VRAM
    for (uint8_t i : vram) i = 0x00;
    // Clear OAM
    for (uint8_t i : oam) i = 0x00;

    // Default state of the PPU
    state = OAMSearch;
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
    // Check for LY register read
    else if (addr == 0xff44) {
        data = ly;
        return true;
    }
    return false;
}
// https://blog.tigris.fr/2019/09/15/writing-an-emulator-the-first-pixel/
// This website was instrumental to the creation of this clock function

void DMG_PPU::clock() {
    frame_complete = false;
    switch (state) {
        case OAMSearch:
            // Need to get the data for upto 10 sprites on this scanline
            // This always takes 40 clocks to complete
            if (clock_count == 40) {
                state = PixelTransfer;
                printf("Changing state to pixel transfer\n");
            }
            break;
        case PixelTransfer:
            // Push pixel data to the screen
            // Need to implement the pixel FIFO and pixel fetcher for the bg/win and the sprites
            state = HBlank;
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