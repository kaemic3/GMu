#ifndef GMU_BUS_H
#define GMU_BUS_H

#include <cstdint>
#include <array>
#include <memory>
#include "SM83.h"
#include "DMG_PPU.h"
#include "Cartridge.h"



class Bus {
public:
    Bus();
    ~Bus() = default;

    // Bus connections
    SM83 cpu;
    DMG_PPU ppu;
    std::shared_ptr<Cartridge> cart = nullptr;

    // Initialize ram - Note need the {} for it to default initialize
    std::array<uint8_t, 8 * 1024> wram{};
    std::array<uint8_t, 127> hram{};

    // Screen data
    std::array<uint8_t, 160 * 144> screen{};
    // Clear the screen
    void clear_screen();
    // Helper functions for reading and writing RAM
    void cpu_write(uint16_t addr, uint8_t data);
    uint8_t cpu_read(uint16_t addr, bool read_only = false);
    // Helper functions to enable and disable interrupts
    void ei();
    void di();

    // Interrupts?? Should they go here??
    // https://gbdev.io/pandocs/Interrupts.html
    uint8_t ime = 0x00;         // Interrupt master enable flag - 0 =  Disable all interrupts, 1 = Enable all interrupts in IE reg
    union Interrupt_Enable {    // Interrupt enable register @ 0xFFFF
        struct {
            uint8_t vblank     : 1;
            uint8_t lcd_stat    : 1;
            uint8_t timer       : 1;
            uint8_t serial      : 1;
            uint8_t joypad      : 1;
            uint8_t unused      : 3;

        };
        uint8_t data;
    } ie_reg;
    // Interrupt flag register
    union Interrupt_Flag {
        struct {
            uint8_t vblank     : 1;
            uint8_t lcd_stat    : 1;
            uint8_t timer       : 1;
            uint8_t serial      : 1;
            uint8_t joypad      : 1;
            uint8_t unused      : 3;
       };
        uint8_t data;
    } if_reg;


    // Misc IO registers
    // Joypad - The joypad is split into 3 different registers. This is due to the fact that the Game boy needs
    // to treat the directional input differently from the action buttons. Though to the programmer, this register
    // is mapped to just 8 bits of memory, register 0xff00

    // Only the first 2 bits are used. They represent bit 4 & 5 of the joypad register.
    uint8_t joypad_input_select = 0x03;

    // Both sets of buttons only use the lower 4 bits.
    // This register is updated by the user and represents the directional inputs
    uint8_t joypad_directional = 0x0f;
    // This register is updated by the user and represents the action inputs
    uint8_t joypad_action = 0x0f;

    // Joypad interrupt flag
    bool joypad_flag = false;

    // TODO Finish implementing serial communication
    // Serial data registers
    // SB - 0xff01
    uint8_t sb = 0x00;
    // SC - 0xff02
    // Bit 0 - Shift clock (0 = External, 1 = Internal)
    // Bit 1 - Clock Speed (only used on CGB: 0 = Normal, 1 = Fast)
    // Bit 7 - Transfer Start Flag (0 = No transfer, 1 = Transfer requested/in progress
    uint8_t sc = 0x00;

    // Timer Registers
    // 0xff04
    uint8_t div = 0x00;
    // 0xff05
    uint8_t tima = 0x00;
    // 0xff06
    uint8_t tma = 0x00;
    // 0xff07 - Bit 0-1 Input Clock select, Bit 2 - Timer enable
    uint8_t tac = 0x00;

    // Total clock count
    uint32_t system_clock_counter = 0;
    // System functions
    void clock();
    void reset();
    // Push a pixel to the screen array
    void push_pixel(uint8_t pixel, uint32_t index);
    // Function to load a cartridge into the Bus class
    void insert_cartridge(const std::shared_ptr<Cartridge> &cartridge);
};


#endif //GMU_BUS_H
