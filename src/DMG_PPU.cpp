#include "DMG_PPU.h"

DMG_PPU::DMG_PPU() {
    // Clear VRAM
    for (uint8_t i : vram) i = 0x00;
    // Clear OAM
    for (uint8_t i : oam) i = 0x00;
}

void DMG_PPU::cpu_write(uint16_t addr, uint8_t data) {

}

uint8_t DMG_PPU::cpu_read(uint16_t addr, bool read_only) {
    uint8_t data = 0x00;

    return data;
}