#include "SM83.h"
#include "Bus.h"

SM83::SM83() {
    // to clean up the opcode table
    using op = SM83;

    opcode_lookup =
    {
            {"NOP", &op::nop, 4, 1}, {"LD BC,d16", &op::ld_bc_d16, 12, 3}, {"LD (BC),A", &op::ld_abs_bc_a, 8, 1}, {"INC BC", &op::inc_bc, 8, 1}, {"INC B", &op::inc_b, 4, 1}, {"DEC B", &op::dec_b, 4, 1}, {"LD B,d8", &op::ld_b_d8, 8, 2}, {"RLCA", &op::rlca, 4, 1}, {"LD (a16),SP", &op::ld_abs_a16_sp, 20, 3}, {"ADD HL,BC", &op::add_hl_bc, 8, 1}, {"LD A,(BC)", &op::ld_a_abs_bc, 8, 1}
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

// Add the contents of the BC register pair into the HL register pair.
// Flags:
//  - N: Reset to 0
//  - H: Set to 1 if overflow from bit 11
//  - C: Set to 1 if overflow from bit 15
uint8_t SM83::add_hl_bc() {
    // For checking L register overflow
    uint16_t l_overflow = l_reg + c_reg;
    l_reg += c_reg;
    // Check to see if L + C overflows
    if(l_overflow > 0xFF)
        h_reg++;
    // For checking for half carry
    uint8_t h_check = ((h_reg & 0xf) + (b_reg & 0xf));
    // For checking H register overflow
    uint16_t h_overflow = h_reg + b_reg;
    h_reg += b_reg;
    // Check to see if half carry needs to be enabled
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check to see if carry needs to be enabled
    if(h_overflow > 0xFF)
        setFlag(C, 1);
    else
        setFlag(C, 0);

    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Decrement the B register. Set according flags.
// Flags:
//  - Z: If result is 0
//  - N: Gets set to 1
//  - H: If bit 4 is set after subtracting 1 from B

uint8_t SM83::dec_b() {
    uint8_t h_check = ((b_reg & 0xf) - (1 & 0xf));
    b_reg--;
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check to see if bit 4 was set due to our subtraction
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    setFlag(N, 1);
    return 0;
}

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
    // Check if bit 4 in b_reg is set due to our addition
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
// Load the  8-bit data value from the absolute address in the register pair BC into register A.
uint8_t SM83::ld_a_abs_bc() {
    uint16_t lowByte = c_reg;
    uint16_t highByte = b_reg;
    // Load the absolute address into addr_abs
    addr_abs = (highByte << 8) | lowByte;
    // Read in the 8-bit data value at the address in addr_abs
    a_reg = read(addr_abs);

    return 0;
}

// Load the 16-bit value in the SP into the 16-bit little endian absolute address.
uint8_t SM83::ld_abs_a16_sp() {
    // Load the absolute address into addr_abs
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    addr_abs = (highByte << 8) | lowByte;

    // Write the SP to the absolute address in little-endian
    write(addr_abs++, sp);
    write(addr_abs, (sp >> 8));
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

// Load the B register with an immediate 8-bit data value.
uint8_t SM83::ld_b_d8() {
    b_reg = read(pc++);
    return 0;
}

// Load register pair BC with an immediate little-endian 16-bit data value.
uint8_t SM83::ld_bc_d16() {
    c_reg = read(pc++);
    b_reg = read(pc++);
    // No additional clocks, return 0
    return 0;
}

// Rotates the bits in A register left.
// Flags:
//  - Z: Reset to 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlca() {
    // If bit 7 in B is set, set the carry bit
    if(b_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    b_reg = (b_reg << 1) | (b_reg >> 7);
    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// No operation.
uint8_t SM83::nop() {
    return 0;
}



