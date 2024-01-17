#include "Cartridge.h"

Cartridge::Cartridge() {
    cart_rom = {};
    cart_ram = {};
    #if 0
   // GB cartridge header information from https://gbdev.io/pandocs/The_Cartridge_Header.html.
   // The cartridge header is located at the address range 0x0100 - 0x014f
    struct Cart_Header {
        // *May move this out of the scope of the constructor*

        // 0x0100-0x0103 - Designated for the entry point of the ROM
        uint8_t entry_point[4];

        // 0x0104-0x0133 - Nintendo logo dump
        // Can implement a check of this region to see if the dump matches the boot ROM.
        // TODO: Implement the boot ROM
        uint8_t nintendo_logo[0x30];

        // 0x0134-0x143 - Title of ROM
        // Some notes on the pan docs mention that this area has different meaning on later ROM carts. Specifically, for
        // the CGB. I will not at this point be adding support for the CGB, so this does not matter to me right now.
        char title[0x10];

        // 0x0144-0x0145 - New licensee code
        char licensee_code[2];

        // 0x0146 - SGB flag
        uint8_t sgb_flag;

        // 0x147 - Cartridge type
        uint8_t cart_type;

        // 0x0148 - ROM size
        uint8_t rom_size;

        // 0x0149 - RAM size
        uint8_t ram_size;

        // 0x014a - Destination code
        uint8_t dest_code;

        // 0x014b - old licensee code
        uint8_t old_licensee_code;

        // 0x014c - Mask ROM version number
        uint8_t rom_version_number;

        // 0x014d - Header checksum
        // Check pan docs for implementation of the checksum algorithm
        uint8_t checksum;

        // 0x014e-0x014f - Global checksum
        // Not used most of the time - check pan docs
        uint16_t global_checksum;

    } header;

    // Create an ifstream to read in the ROM file
    std::ifstream input_file;
    // Open the file in binary mode
    input_file.open(directory, std::ifstream::binary);
    // If this check fails, then the program will most likely not start at all.
    if (!input_file.is_open()) {
        printf("Error opening ROM: %s. Aborting cartridge construction.\n", directory.c_str());
        return;
    }
    // First we need to get the header information from the ROM
    input_file.seekg(0x100, std::ios::beg);
    // Read in the header information into the struct
    input_file.read( (char*)&header, sizeof(header));

    // Get ROM bank count
    switch (header.rom_size) {
        case 0x00:
            rom_banks = 2;
            break;
        case 0x01:
            rom_banks = 4;
            break;
        case 0x02:
            rom_banks = 8;
            break;
        case 0x03:
            rom_banks = 16;
            break;
        case 0x04:
            rom_banks = 32;
            break;
        case 0x05:
            rom_banks = 64;
            break;
        case 0x06:
            rom_banks = 128;
            break;
        default:
            break;
    }
    // Get RAM bank count
    // Each case refers to the code specified in the pan docs
    switch (header.ram_size) {
        case 0x00:
            ram_banks = 0x00;
            break;
        case 0x01:
            // Un-used code assume no banks
            ram_banks = 0x00;
            break;
        case 0x02:
            ram_banks = 1;
            break;
        case 0x03:
            ram_banks = 4;
        default:
            break;
    }

    // Point the ifstream to the beginning of the file
    input_file.seekg(0, std::ios::beg);
    // Resize the ROM - Keep in mind banks are 16 KiB <- Need to double-check this
    cart_rom.resize(rom_banks * 16384);
    // Read in the ROM
    input_file.read((char*)cart_rom.data(), cart_rom.size());


    // Save the mapper id into the according member
    mapper_id = header.cart_type;
    // Load the correct mapper
    switch (mapper_id) {
        // ROM only, no mapper present
        case 0x00:
            // Construct the appropriate mapper according to the mapper_id
            p_mapper = std::make_shared<Mapper_00>(rom_banks, ram_banks);
            break;
        case 0x01:
            p_mapper = std::make_shared<Mapper_01>(rom_banks, ram_banks);
            break;
        case 0x02:
            p_mapper = std::make_shared<Mapper_01>(rom_banks, ram_banks);
            break;
        case 0x03:
            p_mapper = std::make_shared<Mapper_01>(rom_banks, ram_banks);
            break;
        default:
            break;
    }

    // Close the ROM file
    input_file.close();
    #endif
}


void Cartridge::CreateCartridge(memory_arena *arena, debug_platform_read_entire_file *DEBUGPlatformReadEntireFile, 
                                debug_platform_find_rom_file *DEBUGPlatformFindROMFile, debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory) {
    char file_name[260] = "";
    char *file_p = (char *)file_name;
    DEBUGPlatformFindROMFile(&file_p);
    CreateCartridge(arena, DEBUGPlatformReadEntireFile, DEBUGPlatformFreeFileMemory, file_name);

}
void Cartridge::CreateCartridge(memory_arena *arena, debug_platform_read_entire_file *DEBUGPlatformReadEntireFile,
                                debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory, char *file_name) {
    struct Cart_Header {
            // *May move this out of the scope of the constructor*

            // 0x0100-0x0103 - Designated for the entry point of the ROM
            uint8_t entry_point[4];

            // 0x0104-0x0133 - Nintendo logo dump
            // Can implement a check of this region to see if the dump matches the boot ROM.
            // TODO: Implement the boot ROM
            uint8_t nintendo_logo[0x30];

            // 0x0134-0x143 - Title of ROM
            // Some notes on the pan docs mention that this area has different meaning on later ROM carts. Specifically, for
            // the CGB. I will not at this point be adding support for the CGB, so this does not matter to me right now.
            char title[0x10];

            // 0x0144-0x0145 - New licensee code
            char licensee_code[2];

            // 0x0146 - SGB flag
            uint8_t sgb_flag;

            // 0x147 - Cartridge type
            uint8_t cart_type;

            // 0x0148 - ROM size
            uint8_t rom_size;

            // 0x0149 - RAM size
            uint8_t ram_size;

            // 0x014a - Destination code
            uint8_t dest_code;

            // 0x014b - old licensee code
            uint8_t old_licensee_code;

            // 0x014c - Mask ROM version number
            uint8_t rom_version_number;

            // 0x014d - Header checksum
            // Check pan docs for implementation of the checksum algorithm
            uint8_t checksum;

            // 0x014e-0x014f - Global checksum
            // Not used most of the time - check pan docs
            uint16_t global_checksum;

    } *header;
    // Get the ROM file.
    debug_read_file_result read_result = DEBUGPlatformReadEntireFile(file_name);
    // Seek ahead 0x100 bytes.
    header = (Cart_Header *)((u8 *)read_result.contents + 0x100);

    // Get ROM bank count
    switch (header->rom_size) {
        case 0x00:
            rom_banks = 2;
            break;
        case 0x01:
            rom_banks = 4;
            break;
        case 0x02:
            rom_banks = 8;
            break;
        case 0x03:
            rom_banks = 16;
            break;
        case 0x04:
            rom_banks = 32;
            break;
        case 0x05:
            rom_banks = 64;
            break;
        case 0x06:
            rom_banks = 128;
            break;
        default:
            break;
    }
    // Get RAM bank count
    // Each case refers to the code specified in the pan docs
    switch (header->ram_size) {
        case 0x00:
            ram_banks = 0x00;
            break;
        case 0x01:
            // Un-used code assume no banks
            ram_banks = 0x00;
            break;
        case 0x02:
            ram_banks = 1;
            break;
        case 0x03:
            ram_banks = 4;
        default:
            break;
    }

    // Resize the ROM - Keep in mind banks are 16 KiB <- Need to double-check this
    cart_rom.resize(rom_banks * 16384);
    // Resize the RAM
    cart_ram.resize(ram_banks * 16384);
    // Read in the ROM
    u8 *cart_rom_p = (u8 *)cart_rom.data();
    u8 *rom_p = (u8 *)read_result.contents;
    // Copy the ROM file contents into the vector.
    for(s32 rom_index = 0; rom_index < cart_rom.size(); ++rom_index)
    {
        *cart_rom_p++ = *rom_p++;
    }

    // Save the mapper id into the according member
    mapper_id = header->cart_type;
    // Load the correct mapper
    switch (mapper_id) {
        // ROM only, no mapper present
        case 0x00:
            // Construct the appropriate mapper according to the mapper_id
            p_mapper = new Mapper_00(rom_banks, ram_banks);
            break;
        case 0x01:
            p_mapper = new Mapper_01(rom_banks, ram_banks);
            break;
        case 0x02:
            p_mapper = new Mapper_01(rom_banks, ram_banks);
            break;
        case 0x03:
            p_mapper = new Mapper_01(rom_banks, ram_banks);
            break;
        default:
            break;
    }
    // Free the file.
    DEBUGPlatformFreeFileMemory(read_result.contents);
}

bool Cartridge::cpu_write(uint16_t addr, uint8_t data) {
    // Check if the mapper needs to handle the write call
    if(p_mapper->cpu_map_write(addr, data)) {
        return true;
    }
    return false;
}
bool Cartridge::cpu_read(uint16_t addr, uint8_t &data) {
    uint32_t mapped_addr = 0;
    // Check if the mapper needs to handle the read call
    bool ram = false;
    if(p_mapper->cpu_map_read(addr, mapped_addr, ram)) {
        if(!ram)
        {
            data = cart_rom[mapped_addr];
            return true;
        }
        else
        {
            data = cart_ram[mapped_addr];
            return true;
        }
    }
    return false;
}

void Cartridge::load_boot_rom(const std::string &directory) {
    std::ifstream boot_rom;
    // Open the boot rom in binary mode
    boot_rom.open(directory, std::ifstream::binary);
    boot_rom.read((char*)cart_rom.data(), 0x100);
}

// May not be needed?? Should implement something in the Bus that the front end can grab rather than
// directly access the Cartridge class
uint8_t Cartridge::viewport_get_data(uint16_t addr) {
    uint8_t data = 0x00;

    return data;
}

uint8_t Cartridge::get_rom_bank() {
    return p_mapper->get_current_rom_bank();
}