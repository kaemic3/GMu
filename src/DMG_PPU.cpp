#include "DMG_PPU.h"

DMG_PPU::DMG_PPU() {
    // Clear VRAM
    for (uint8_t i : vram) i = 0x00;
    // Clear OAM
    for (uint8_t i : oam) i = 0x00;
}

// Need to add checks for when the PPU is accessing RAM directly since the
// function will need to not write anything at all
bool DMG_PPU::cpu_write(uint16_t addr, uint8_t data) {
    if(addr >= 0x8000 && addr <= 0x9fff) {
        // Mask the passed address so it is offset to 0x0000
        vram[addr & 0x1fff] = data;
        return true;
    }
    else if(addr >= 0xfe00 && addr <= 0xfe9f) {
        oam[addr & 0x009f] = data;
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
    }
    return false;
}

void DMG_PPU::clock() {

}