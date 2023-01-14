#ifndef GMU_BUS_H
#define GMU_BUS_H

#include <cstdint>
#include <array>
#include "SM83.h"
class Bus {
public:
    Bus();
    ~Bus();
    // Initialize a CPU object
    SM83 cpu;
    std::array<uint8_t, 0xFFFF> ram;
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr, bool bReadOnly = false);
};


#endif //GMU_BUS_H
