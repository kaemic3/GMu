#ifndef GMU_MAPPER_00_H
#define GMU_MAPPER_00_H
#include "Mapper.h"

class Mapper_00 : public Mapper {
public:
    explicit Mapper_00(uint8_t rom_banks, uint8_t ram_banks = 0);
    ~Mapper_00() = default;

    bool cpu_map_read(uint16_t addr, uint32_t &mapped_addr, bool &ram) override;
    bool cpu_map_write(uint16_t addr, uint8_t data) override;
    uint8_t get_current_rom_bank() override { return 0; }
};



#endif //GMU_MAPPER_00_H
