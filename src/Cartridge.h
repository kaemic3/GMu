#ifndef GMU_CARTRIDGE_H
#define GMU_CARTRIDGE_H

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include "Mapper_00.h"
#include "Mapper_01.h"

class Cartridge {
public:
    Cartridge(const std::string &directory);
    ~Cartridge() = default;

    // Communication with the Main Bus
    bool cpu_write(uint16_t addr, uint8_t data);
    bool cpu_read(uint16_t addr, uint8_t &data);

    // Load boot rom
    void load_boot_rom(const std::string &directory = "../ROMs/dmg.bin");
    // Helper function for memory viewport
    // Will probably need to use the mapper to get the correct data from the rom
    uint8_t viewport_get_data(uint16_t addr);
    uint8_t get_rom_bank();
//private:
    // Containers for the rom and ram. They are vectors since their size is unknown until the
    // cartridge header information is obtained.
    std::vector<uint8_t> cart_rom;
    std::vector<uint8_t> cart_ram;

    // These values keep track of the mapper information needed to do proper bank switching.
    uint8_t mapper_id = 0x00;
    uint8_t rom_banks = 0x00;
    uint8_t ram_banks = 0x00;

    // Pointer to the mapper for this cartridge
    std::shared_ptr<Mapper> p_mapper = nullptr;

};


#endif //GMU_CARTRIDGE_H
