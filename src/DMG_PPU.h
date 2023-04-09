#ifndef GMU_DMG_PPU_H
#define GMU_DMG_PPU_H

#include <cstdint>
#include <array>

class DMG_PPU {
public:
    DMG_PPU();
    ~DMG_PPU() = default;

    // Communication with the main bus
    void cpu_write(uint16_t addr, uint8_t data);
    uint8_t cpu_read(uint16_t addr, bool read_only);
    // No need for ppu_r/w functions since there is no PPU bus

    // Clock function
    void clock();
private:
    // Initialize 8 KiB VRAM
    std::array<uint8_t, 8 * 1024> vram = {};
    // Initialize OAM
    std::array<uint8_t, 160> oam = {};
};


#endif //GMU_DMG_PPU_H
