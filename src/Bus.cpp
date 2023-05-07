#include "Bus.h"

Bus::Bus() {
    // Clear RAM
    for(uint8_t i : wram) i = 0x00;
    for(uint8_t i : hram) i = 0x00;

    // connect cpu and ppu to bus
    cpu.connect_bus(this);
    ppu.connect_bus(this);
}

void Bus::clear_screen() {
    std::array<uint8_t, 160 * 144> empty = {};
    std::swap(screen, empty);
}

void Bus::cpu_write(uint16_t addr, uint8_t data) {
    // Check if the cartridge should handle this write call
    if (cart->cpu_write(addr, data)) {

    }
    // Check for writing to WRAM
    else if (addr >= 0xc000 && addr <= 0xfdff) {
        // Apply the address mask to the address so the WRAM can be "mirrored"
        // and so the offset of the passed address is 0x0000
        wram[addr & 0x1fff] = data;
    }
    // Check for writing to HRAM
    else if (addr >= 0xff80 && addr <= 0xfffe) {
        // Apply the mask to the address
       hram[addr - 0xff80] = data;
    }
    // Check if write is for IO registers
    else if (addr == 0xff00) {
        joypad_input = data;
    }
    // Check if the passed address is for VRAM
    else if (ppu.cpu_write(addr, data)) {

    }
    else {
        // If we get here, then the address is invalid
        printf("Attempt to write to an illegal address: 0x%X is not writeable.\n", addr);
    }
}

uint8_t Bus::cpu_read(uint16_t addr, bool read_only) {
    uint8_t data = 0x00;
    // Check if the cartridge should handle this read call
    if (cart->cpu_read(addr, data)) {

    }
    // Check the read is for WRAM
    else if (addr >= 0xc000 && addr <= 0xfdff) {
        // Apply the address mask to the address so the WRAM can be "mirrored"
        // and so the offset of the passed address is 0x0000
        data = wram[addr & 0x1fff];
    }
    // Check if the read is for HRAM
    else if (addr >= 0xff80 && addr <= 0xfffe) {
        // Offset the address to 0x0000
        data = hram[addr - 0xff80];
    }
    // Check if the read is for IO registers
    else if (addr == 0xff00) {
        data = joypad_input;
    }
    // Check if the read is for VRAM
    // TODO: Add check so the PPU can block access to VRAM
    else if (ppu.cpu_read(addr, data)) {
    }

    // If the address in not valid, return 0x00
    return data;
}

// The CPU and PPU run at the same clock speed
// First the cpu is clocked, then the ppu
void Bus::clock() {
    cpu.clock();
    ppu.clock();

    system_clock_counter++;
}

void Bus::reset() {
    // CPU reset not yet implemented
    cpu.reset();
    // Need to implement a PPU reset
    system_clock_counter = 0;
}
void Bus::push_pixel(uint8_t pixel, uint32_t index) {
    screen[index] = pixel;
}

void Bus::insert_cartridge(const std::shared_ptr<Cartridge> &cartridge) {
    cart = cartridge;
}