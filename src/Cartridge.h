#ifndef GMU_CARTRIDGE_H
#define GMU_CARTRIDGE_H

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include "Mapper_00.h"
#include "Mapper_01.h"
#include "nenjin_platform.h"
struct memory_arena;
class Cartridge {
public:
    Cartridge();
    ~Cartridge() { cart_ram.clear(); cart_rom.clear();}

    // Communication with the Main Bus
    bool cpu_write(uint16_t addr, uint8_t data);
    bool cpu_read(uint16_t addr, uint8_t &data);

    // Create Cartridge
    void CreateCartridge(memory_arena *arena, debug_platform_read_entire_file *DEBUGPlatformReadEntireFile, 
                         debug_platform_find_rom_file *DEBUGPlatfromFindROMFile, 
                         debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory);
    void CreateCartridge(memory_arena *arena, debug_platform_read_entire_file *DEBUGPlatformReadEntireFile, 
    debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory, char *file_name);
    // Free Cartridge

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
