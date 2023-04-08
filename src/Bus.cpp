#include "Bus.h"

Bus::Bus() {
    // Clear RAM
    for(uint8_t i : wram) i = 0x00;

    // connect cpu to bus
    cpu.ConnectBus(this);
}

void Bus::cpu_write(uint16_t addr, uint8_t data) {
    // Check if the passed address is for WRAM
    if(addr >= 0xc000 && addr <= 0xfdff)
        // Apply the address mask to the address so the WRAM can be "mirrored"
        // and so the offset of the passed address is 0x0000
        wram[addr & 0x1fff] = data;
    // Check if the passed address is for VRAM
    else if (addr >= 0x8000 && addr <= 0x9fff) {
        // Mask the passed address so it is offset to 0x0000
        ppu.cpu_write(addr & 0x1fff, data);
    }
}

uint8_t Bus::cpu_read(uint16_t addr, bool read_only) {
        // Check if the passed address is for WRAM
        if(addr >= 0xc000 && addr <= 0xfdff)
            // Apply the address mask to the address so the WRAM can be "mirrored"
            // and so the offset of the passed address is 0x0000
            return wram[addr & 0x1fff] ;
        // Check if the passed address is for VRAM
        else if (addr >= 0x8000 && addr <= 0x9fff) {
            // Mask the passed address so it is offset to 0x0000
            return ppu.cpu_read(addr & 0x1fff, read_only);
        }

        // If the address in not valid, return 0
        return 0x00;
}

void Bus::insert_cartridge(std::shared_ptr<Cartridge> &cartridge) {
    cart = cartridge;
}