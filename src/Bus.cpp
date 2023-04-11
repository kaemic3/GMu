#include "Bus.h"

Bus::Bus() {
    // Clear RAM
    for(uint8_t i : wram) i = 0x00;
    for(uint8_t i : hram) i = 0x00;

    // connect cpu to bus
    cpu.ConnectBus(this);
}

void Bus::cpu_write(uint16_t addr, uint8_t data) {
    // Check if the cartridge should handle this write
    if (cart->cpu_write(addr, data)) {

    }
    // Check if the passed address is for WRAM
    else if(addr >= 0xc000 && addr <= 0xfdff) {
        // Apply the address mask to the address so the WRAM can be "mirrored"
        // and so the offset of the passed address is 0x0000
        wram[addr & 0x1fff] = data;
    }
    // Check if the passed address is for HRAM
    else if(addr >= 0xff80 && addr <= 0xfffe) {
        // Apply the mask to the address
       hram[addr & 0x007f] = data;
    }

    // Check if the passed address is for VRAM
   // Mask the passed address to set the offset to 0x0000
    else if (ppu.cpu_write(addr, data)) {
    }
    else {
        printf("Attempt to write to an illegal address: 0x%X is not writeable.\n", addr);
    }
}

uint8_t Bus::cpu_read(uint16_t addr, bool read_only) {
    uint8_t data = 0x00;
    if(cart->cpu_read(addr, data)) {

    }
    // Check if the passed address is for WRAM
    else if(addr >= 0xc000 && addr <= 0xfdff) {
        // Apply the address mask to the address so the WRAM can be "mirrored"
        // and so the offset of the passed address is 0x0000
        data = wram[addr & 0x1fff];
    }
    // Check if the passed address is for HRAM
    else if(addr >= 0xff80 && addr <= 0xfffe) {
        // Apply the mask to the address
        data = hram[addr & 0x007f];
    }
    // Check if the passed address is for VRAM
    // TODO: Add check so the PPU can block access to VRAM
    else if (ppu.cpu_read(addr, data)) {
    }
    // If the address in not valid, return 0
    return data;
}

void Bus::clock() {
    ppu.clock();
    cpu.clock();

    system_clock_counter++;
}

void Bus::reset() {
    // CPU reset not yet implemented
    cpu.reset();
    system_clock_counter = 0;
}

void Bus::insert_cartridge(const std::shared_ptr<Cartridge> &cartridge) {
    cart = cartridge;
}