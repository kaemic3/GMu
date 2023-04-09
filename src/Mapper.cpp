#include "Mapper.h"

Mapper::Mapper(uint8_t rom_banks, uint8_t ram_banks) {
    this->rom_banks = rom_banks;
    this->ram_banks = ram_banks;
}

Mapper_00::Mapper_00(uint8_t rom_banks, uint8_t ram_banks) : Mapper(rom_banks, ram_banks){

}

bool Mapper_00::cpu_map_read(uint16_t addr, uint32_t &mapped_addr) {
    // Check if the address is within the cartridge ROM memory range
    if (addr >= 0x0000 && addr <= 0x7fff) {
        // In mapper 0, the addr does not need to be mapped, so we can return the same addr
        mapped_addr = addr;
        return true;
    }
    // Check if the address is within cartridge RAM
    else if (addr >= 0xa000 && addr <= 0xbfff) {
        // Mapper 0 does not have ram so return false
        printf("Error: Mapper 0 does not have external RAM.\n");
        return false;
    }
    return false;
}

bool Mapper_00::cpu_map_write(uint16_t addr, uint32_t &mapped_addr) {
    // Mapper 0 has no ROM bank switching or RAM banks, so this function will always return false
    // Check if the address is within the cartridge ROM memory range
    if (addr >= 0x0000 && addr <= 0x7fff) {
        printf("Error: Mapper 0 does not have bank switching or external RAM.\nCannot write to address: 0x%x.\n",addr);
    }
        // Check if the address is within cartridge RAM
    else if (addr >= 0xa000 && addr <= 0xbfff) {
        // Mapper 0 does not have ram so return false
        printf("Error: Mapper 0 does not have bank switching or external RAM.\nCannot write to address: 0x%x.\n",addr);
    }
    return false;
}
