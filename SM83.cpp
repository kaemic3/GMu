#include "SM83.h"
#include "Bus.h"

SM83::SM83() {
    // to clean up the opcode table
    using op = SM83;

    opcode_lookup =
    {
            {"NOP", &op::nop, 4, 1}, {"LD BC,d16", &op::ld_bc_d16, 12, 3}
    };
}

SM83::~SM83() {

}

uint8_t SM83::read(uint16_t addr, bool bReadOnly) {
    return bus->read(addr, bReadOnly);
}

void SM83::write(uint16_t addr, uint8_t data) {
    bus->write(addr, data);
}

void SM83::clock() {
    // Only execute when the internal cycles count is 0
    if(cycles = 0) {
        // Read opcode - PC will be pointing to it
        opcode = read(pc);
        // Inc the PC to point to the next byte of the instruction
        pc++;
        // get the number of cycles for the instruction
        cycles = opcode_lookup[opcode].cycles;
        // Call the function pointer of the current opcode - Function returns the number of additional cycles req
        uint8_t additional_cycle = (this->*opcode_lookup[opcode].operate)();
        cycles += additional_cycle;
    }
    cycles--;
}
// Instructions in alphabetical order
uint8_t SM83::ld_bc_d16() {

    // No additional clocks, return 0
    return 0;
}
uint8_t SM83::nop() {
    return 0;
}


