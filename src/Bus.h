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
    // Helper functions for reading and writing RAM
    void cpu_write(uint16_t addr, uint8_t data);
    uint8_t cpu_read(uint16_t addr, bool read_only = false);
    // Function to load a cartridge into the Bus class
    void insert_cartridge(std::shared_ptr<Cartridge> &cartridge);

    // Total clock count
    uint32_t system_clock_count = 0;
    // System functions
    void clock();
    void reset();
};


#endif //GMU_BUS_H
