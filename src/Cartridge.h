#ifndef GMU_CARTRIDGE_H
#define GMU_CARTRIDGE_H

#include <cstdint>
#include <vector>

class Cartridge {
public:
    Cartridge();
    ~Cartridge() = default;

    // Communication with the Main Bus
    bool cpu_write(uint16_t addr, uint8_t data);
    bool cpu_read(uint16_t addr, bool read_only);
};


#endif //GMU_CARTRIDGE_H
