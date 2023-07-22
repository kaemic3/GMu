#ifndef GMU_MAPPER_H
#define GMU_MAPPER_H

#include <cstdint>
#include <cstdio>

// Abstract Mapper class
class Mapper {
public:
    Mapper(uint8_t rom_banks, uint8_t ram_banks);
    ~Mapper() = default;

    virtual bool cpu_map_read(uint16_t addr, uint32_t &mapped_addr) = 0;
    virtual bool cpu_map_write(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t get_current_rom_bank() = 0;

protected:
    uint8_t rom_banks = 0;
    uint8_t ram_banks = 0;
};



#endif //GMU_MAPPER_H
