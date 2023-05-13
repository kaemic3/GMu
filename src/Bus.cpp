#include "Bus.h"

Bus::Bus() {
    // Clear RAM
    for(uint8_t i : wram) i = 0x00;
    for(uint8_t i : hram) i = 0x00;

    // Initialize the interrupt registers
    ie_reg.data = 0;
    if_reg.data = 0;

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
    // Check if write is to the IO registers
    else if (addr == 0xff00) {
        // Grab only bit 4 & 5
        uint8_t masked_data = data & 0x30;
        // Or the masked data with the current register
        // Shift the data right 4 bits to align the grabbed bits to bit 0 and 1
        masked_data = masked_data >> 4;
        joypad_input_select = masked_data;
    }
    // Check if write is to the IF register
    else if (addr == 0xff0f) {
        if_reg.data = data;
    }
    // Check if write is to the IE register
    else if (addr == 0xffff) {
        ie_reg.data = data;
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
        // When reading joypad input, we need use the 3 joypad registers to return
        // the bits the way the GB would.

        // First check the select bit
        if (joypad_input_select == 0x02) {
            // If bit 0 is reset, then return the directional buttons
            // Also reset bit 4
            data = joypad_directional  &~(1 << 4);
        }
        else if (joypad_input_select == 0x01) {
            // If bit 2 is reset, then return the action buttons
            // Also reset bit 5
            data = joypad_action &~(1 << 5);
        }
        else
            // Otherwise return 0xff
            data = 0xff;

    }
    // Check if the read is from the DIV register
    else if (addr == 0xff04) {
        data = div;
    }
    // Check if the read is for VRAM
    // TODO: Add check so the PPU can block access to VRAM
    else if (ppu.cpu_read(addr, data)) {
    }

    // If the address in not valid, return 0x00
    return data;
}

void Bus::ei() {
    ime = 1;
}

void Bus::di() {
    ime = 0;
}

// The CPU and PPU run at the same clock speed
// First the cpu is clocked, then the ppu
void Bus::clock() {
    // This function will scan the system for interrupts
    cpu.interrupt_scan();

    cpu.clock();
    ppu.clock();

    if (system_clock_counter % 256 == 0) {
        div++;
    }
    system_clock_counter++;
}

void Bus::reset() {
    cpu.reset();
    ppu.reset();
    clear_screen();

    system_clock_counter = 0;
    joypad_input_select = 0x03;
    joypad_action = 0x0f;
    joypad_directional = 0x0f;

}
void Bus::push_pixel(uint8_t pixel, uint32_t index) {
    screen[index] = pixel;
}

void Bus::insert_cartridge(const std::shared_ptr<Cartridge> &cartridge) {
    cart = cartridge;
}