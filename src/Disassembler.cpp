#include "Disassembler.h"

Disassembler::Disassembler(const std::string &file, Bus* bus) {
    output.open(file, std::ios::out | std::ios::trunc);
    if (!output.is_open())
        printf("Error, could not open file.\n");
    p_bus = bus;
}

void Disassembler::output_instruction(const std::vector<std::string> &instruction, uint16_t pc_location) {
    // Padding value
    int padding_end = 50;
    int padding_begin = 15;
    // String stream
    std::stringstream text;
    // Return if no file is open
    if (!output.is_open()) {
        printf("Error, no file is open.\n");
        return;
    }
    if (instruction.size() == 3) {
        // Write the current mnemonic and PC to the file
        text << "$" << std::hex << pc_location << ":";
    }
    else {
        // For prefixed opcodes
        text << "$" << std::hex << pc_location << ":$CB ";
    }
    // Write rest of the output text to the stringstream
    int string_size = text.str().size();
    text << std::setw(padding_begin - string_size) << "$" << std::hex << std::stoul(instruction[0]) << ": " << instruction[1];
    string_size = text.str().size();
    text << std::setw(padding_end - string_size) << " BYTES:" << instruction[2] <<
         "  LY: " << std::dec << p_bus->ppu.ly << "   STAT: " << std::hex << std::stoul(std::to_string(p_bus->ppu.stat.data)) <<
         "  LCDC: " << std::bitset<8>(p_bus->ppu.lcdc.data) << " " << std::hex << std::stoul(std::to_string(p_bus->ppu.lcdc.data)) <<
         "  ROM_BANK: " << std::to_string(p_bus->cart->get_rom_bank()) <<
         "  SP: " << std::hex << std::stoul(std::to_string(p_bus->cpu.sp))<< "\n";

    output << text.str();
}

void Disassembler::open_file(const std::string &file) {
    output.open(file, std::ios::out | std::ios::app | std::ios::trunc);
}

void Disassembler::close_file() {
    output.close();
}