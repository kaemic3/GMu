#ifndef GMU_DMG_PPU_H
#define GMU_DMG_PPU_H

#include <cstdint>
#include <array>

class DMG_PPU {
public:
    DMG_PPU();
    ~DMG_PPU() = default;

    // Communication with the main bus
    bool cpu_write(uint16_t addr, uint8_t data);
    bool cpu_read(uint16_t addr, uint8_t &data);
    // No need for ppu_r/w functions since there is no PPU bus

    // Clock function
    void clock();

    // An enum that contains the different states of the PPU
    enum PPUState {
        OAMSearch,
        PixelTransfer,
        HBlank,
        VBlank
    } state;

    // Current scanline
    // uint32_t to make it easier to draw to the debug viewport
    uint32_t ly = 0;
private:
    // Initialize 8 KiB VRAM
    std::array<uint8_t, 8 * 1024> vram = {};
    // Initialize OAM
    std::array<uint8_t, 160> oam = {};

    // Clock count
    uint32_t clock_count = 0;
};


#endif //GMU_DMG_PPU_H
