#ifndef GMU_MAPPER_01_H
#define GMU_MAPPER_01_H

#include "Mapper.h"

class Mapper_01 : public Mapper {
public:
    explicit Mapper_01(uint8_t rom_banks, uint8_t ram_banks);
    ~Mapper_01() = default;

    bool cpu_map_read(uint16_t addr, uint32_t &mapper_addr) override;
    bool cpu_map_write(uint16_t addr, uint8_t data) override;
    uint8_t get_current_rom_bank() override { return (uint8_t) reg.rom_bank_number; };
private:
    // This flag should be set to true when the cartridge uses the RAM bank number register to expand the available ROM
    bool high_rom_cartridge = false;

    struct Mapper_Registers {
        uint8_t rom_bank_number : 5;
        uint8_t ram_rom_2bit    : 2;
        uint8_t bank_mode       : 1;
        uint8_t ram_enable      : 1;
    } reg;
    // Store the addresses to the according ROM and RAM - these are defined to support the largest ROM and RAM sizes.
    struct Mapper_Addresses {
        uint32_t rom_address : 21;
        uint16_t ram_address : 15;
    } addresses;
};


#endif //GMU_MAPPER_01_H
