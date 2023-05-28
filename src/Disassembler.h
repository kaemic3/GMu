#ifndef GMU_DISASSEMBLER_H
#define GMU_DISASSEMBLER_H
#include "GMu.h"

class Disassembler {
public:
    Disassembler(const std::string &file);
    ~Disassembler() = default;

    // Write the current instruction with the PC addr to the output file
    void output_instruction(const std::vector<std::string> &instruction, uint16_t pc_location);
    void open_file(const std::string &file);
    void close_file();

private:
    std::ofstream output;
};


#endif //GMU_DISASSEMBLER_H
