#include "SM83.h"
#include "Bus.h"

SM83::SM83() {
    // to clean up the opcode table
    using op = SM83;

    opcode_lookup =
    {
            {"NOP", &op::nop, 4, 1}, {"LD BC,d16", &op::ld_bc_d16, 12, 3}, {"LD (BC),A", &op::ld_abs_bc_a, 8, 1}, {"INC BC", &op::inc_bc, 8, 1}, {"INC B", &op::inc_b, 4, 1}
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
    if(cycles == 0) {
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
bool SM83::complete() {
    return cycles == 0;
}
// Returns 1 if the flag bit is set and 0 if reset
uint8_t SM83::getFlag(SM83_FLAGS f) {
    // If the specified bit is set return 1 else return 0
    return ((f_reg & f) > 0 ? 1 : 0);
}
// Sets or resets a specific flag bit in f_reg
void SM83::setFlag(SM83_FLAGS f, bool v) {
    if(v)
        f_reg |= f;     // Set the bit
    else
        f_reg &= ~f;    // Reset the bit
}

// Fetch the data located at addr_abs.
uint8_t SM83::fetch() {
    fetched = read(addr_abs);
    return fetched;
}
// Instructions in alphabetical order

// Increment the B register. Set according flags.
// Flags:
//  - Z: If result is 0
//  - N: Gets reset to 0
//  - H: If bit 4 is set after adding 1 to B

uint8_t SM83::inc_b() {
    // Used for the half carry bit
    // By &ing the B register with 0xf, we reset the high nibble. Same for 1, but the high nibble
    // for 1 is never set. It is written out here for clarity.
    // When we add these two numbers together, we can check bit 4.
    // If bit 4 is enabled, that means this addition should set the half carry bit.

    uint8_t h_check= ((b_reg & 0xf) + (1 & 0xf));

    b_reg++;
    if(b_reg == 0)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check if bit 4 in b_reg is set
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);      // If so, set the half-carry flag
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the BC register pair. If C overflows, increment the B register.
uint8_t SM83::inc_bc() {
    c_reg++;
    // Check to see if C wrapped back around to 0x00
    if (c_reg == 0x00)
        b_reg++;        // If so, increase B
    return 0;
}

// Using BC as an absolute address, load value in A into that address.
uint8_t SM83::ld_abs_bc_a() {
    uint16_t lowByte = c_reg;
    uint16_t highByte = b_reg;
    addr_abs = (highByte << 8) | lowByte;
    write(addr_abs, a_reg);
    return 0;
}

// Load register pair BC with an immediate little-endian 16-bit data value.
uint8_t SM83::ld_bc_d16() {
    c_reg = read(pc++);
    b_reg = read(pc++);
    // No additional clocks, return 0
    return 0;
}

// No operation.
uint8_t SM83::nop() {
    return 0;
}



