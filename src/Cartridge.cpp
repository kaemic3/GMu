#include "Cartridge.h"

Cartridge::Cartridge(const std::string &directory) {

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
            rom_banks = 0x02;
            break;
        case 0x01:
            rom_banks = 0x04;
            break;
        default:
            break;
    }
    // Get RAM bank count
    switch (header.ram_size) {
        case 0x00:
            ram_banks = 0x00;
            break;
        case 0x01:
            // Un-used code assume no banks
            ram_banks = 0x00;
            break;
        default:
            break;
    }

    // Save the mapper id into the according member
    mapper_id = header.cart_type;

    // Information could be assumed by this alone... I think
    switch (mapper_id) {
        // ROM only, no mapper present
        case 0x00:
            // Point the ifstream to the beginning of the file
            input_file.seekg(0, std::ios::beg);
            // Resize the ROM - Keep in mind banks are 16 KiB
            cart_rom.resize(rom_banks * 16384);
            // Read in the ROM
            input_file.read((char*)cart_rom.data(), cart_rom.size());
            break;
        default:
            break;
    }
    // Close the ROM file

    input_file.close();
}

bool Cartridge::cpu_write(uint16_t addr, uint8_t data) {
    return false;
}

bool Cartridge::cpu_read(uint16_t addr, bool read_only) {
    return false;
}