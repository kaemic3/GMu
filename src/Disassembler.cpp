#include "Disassembler.h"

Disassembler::Disassembler(const std::string &file) {
    output.open(file, std::ios::out | std::ios::trunc);
    if (!output.is_open())
        printf("Error, could not open file.\n");
}

void Disassembler::output_instruction(const std::vector<std::string> &instruction, uint16_t pc_location) {
    // Return if no file is open
    if (!output.is_open()) {
        printf("Error, no file is open.\n");
        return;
    }
    if (instruction.size() == 3)
    // Write the current mnemonic and PC to the file
        output <<  "$" << std::hex << pc_location << ":    $" << std::hex << std::stoul(instruction[0]) << ": " << instruction[1] << " BYTES:" << instruction[2] << "\n";
    else
        output << "$" << std::hex << pc_location << ":$CB $" << std::hex << std::stoul(instruction[0]) << ": " << instruction[1] << " BYTES:" << instruction[2] << "\n";
}

void Disassembler::open_file(const std::string &file) {
    output.open(file, std::ios::out | std::ios::app | std::ios::trunc);
}

void Disassembler::close_file() {
    output.close();
}