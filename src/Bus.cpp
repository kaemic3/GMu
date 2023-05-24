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
    // Check timer registers
    // Div register
    else if (addr == 0xff04) {
        div = data;
    }
    // TIMA register
    else if (addr == 0xff05) {
        tima = data;
    }
    // TMA register
    else if (addr == 0xff06) {
        tma = data;
    }
    // TAC register - Only bits 0-2 used
    else if (addr == 0xff07) {
        tac = data & 0x7;
    }
    // Check if write is to the IF register
    else if (addr == 0xff0f) {
        if_reg.data = data;
    }
    // Check for write to DMA
    else if (addr == 0xff46) {
        // Writes to the DMA register are divided by 0x100
        dma = data / 0x100;
        // Change the system state
        run_dma();
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
    // TIMA register
    else if (addr == 0xff05) {
        data = tima;
    }
    // TMA register
    else if (addr == 0xff06) {
        data = tma;
    }
    // TAC register
    else if (addr == 0xff07) {
        data = tac;
    }
    // DMA register
    else if (addr == 0xff46) {
        data = dma;
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

void Bus::run_dma() {
    // Change the state of the CPU to DMA mode
    cpu.state = SM83::DMA;
}

// The CPU and PPU run at the same clock speed
// First the cpu is clocked, then the ppu
void Bus::clock() {
    // Compare LYC to LY
    if (ppu.ly == ppu.lyc && !ppu.ly_lyc_flag) {
        ppu.stat.lyc_ly_flag = 1;
        ppu.ly_lyc_flag = true;
    }
    // * VBlank *

    // See if the VBlank interrupt flag needs to be set
    if (ppu.state == DMG_PPU::VBlank && !ppu.vblank_fired) {
        if_reg.vblank = 1;
    }

    // Need to add a check to see if the interrupt has already fired since entering VBLANK
    // * LCD STAT *
    // First check which interrupt source is set
    if (ppu.stat.hblank_int_src == 1) {
        // Check if the mode flag is set to 0
        if (ppu.state == DMG_PPU::HBlank && !ppu.hblank_flag) {
            if_reg.lcd_stat = 1;
        }
    }
    else if (ppu.stat.vblank_int_src == 1) {
        if (ppu.state == DMG_PPU::VBlank && !ppu.vblank_fired) {
            if_reg.lcd_stat = 1;
        }
    }
    else if (ppu.stat.oam_int_src == 1) {
        if (ppu.state == DMG_PPU::OAMSearch && !ppu.oam_flag) {
            if_reg.lcd_stat = 1;
        }
    }
    else if (ppu.stat.lyc_int_src == 1) {
        if (ppu.stat.lyc_ly_flag == 1) {
            if_reg.lcd_stat = 1;
        }
    }

    // * Timer *

    // Check to increment the DIV register
    if (system_clock_counter % 256 == 0) {
        div++;
    }
    // First check if the timer is enabled
    if (tac & 0x4) {
        // Check to increment the TIMA register for when bit 0-1 are not set
        if (tac == 0x4) {
            // Now check to see if the register should be incremented based on the current system clock
            if (system_clock_counter % 1024 == 0) {
                // If TIMA overflows
                if (tima > 0 && 1 > UINT8_MAX - tima) {
                    // Set TIMA to value in TMA
                    tima = tma;
                    // Set the interrupt bit for the timer
                    if_reg.timer = 1;
                }
                    // No overflow
                else
                    tima++;
            }
        }
        // Check to increment TIMA register when bit 0 is set and bit 2 is set
        else if (tac == 0x5) {
            // Check to see if the register should be incremented based on the current system clock
            if (system_clock_counter % 16 == 0) {
                // If TIMA overflows
                if (tima > 0 && 1 > UINT8_MAX - tima) {
                    // Set TIMA to TMA
                    tima = tma;
                    // Set interrupt bits
                    if_reg.timer = 1;
                }
                // No overflow
                else {
                    tima++;
                }
            }
        }
        // Check to increment TIMA register when bit 1 is set and bit 2 is set
        else if (tac == 0x6) {
            // Increment check
            if (system_clock_counter % 64 == 0) {
                if (tima > 0 && 1 > UINT8_MAX - tima) {
                    tima = tma;
                    if_reg.timer = 1;
                }
                else {
                    tima++;
                }
            }
        }
        // Check to increment TIMA register when bit 0-2 are set
        else if (tac == 0x7) {
            // Increment check
            if (system_clock_counter % 256 == 0) {
                if (tima > 0 && 1 > UINT8_MAX - tima) {
                    tima = tma;
                    if_reg.timer = 1;
                }
                else {
                    tima++;
                }
            }
        }
    }

    // * Joypad *
    // Need to check the low nibble of the input registers go from 1 to 0
    // Interrupt is only fired when on of the input select bits are low

    // First check to see if at least one input select bits are not set
    if (!joypad_flag) {
        // Now check if any of the joypad buttons have been pressed
        if (joypad_action < 0x0f || joypad_directional < 0x0f) {
            if_reg.joypad = 1;
            joypad_flag = true;
        }
    }
    if (joypad_flag &&  joypad_action == 0x0f && joypad_directional == 0x0f) {
        if_reg.joypad = 0;
        joypad_flag = false;
    }

    // Clock the CPU and PPU
    cpu.clock();
    ppu.clock();

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
    if_reg.data = 0;
    tima = 0;
    tma = 0;
    tac = 0;

}
void Bus::push_pixel(uint8_t pixel, uint32_t index) {
    screen[index] = pixel;
}

void Bus::insert_cartridge(const std::shared_ptr<Cartridge> &cartridge) {
    cart = cartridge;
}