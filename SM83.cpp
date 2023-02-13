#include "SM83.h"
#include "Bus.h"

SM83::SM83() {
    // to clean up the opcode table
    using op = SM83;

    opcode_lookup =
    {
            {"NOP", &op::nop, 4, 1}, {"LD BC,d16", &op::ld_bc_d16, 12, 3}, {"LD (BC),A", &op::ld_abs_bc_a, 8, 1}, {"INC BC", &op::inc_bc, 8, 1}, {"INC B", &op::inc_b, 4, 1}, {"DEC B", &op::dec_b, 4, 1}, {"LD B,d8", &op::ld_b_d8, 8, 2}, {"RLCA", &op::rlca, 4, 1}, {"LD (a16),SP", &op::ld_abs_a16_sp, 20, 3}, {"ADD HL,BC", &op::add_hl_bc, 8, 1}, {"LD A,(BC)", &op::ld_a_abs_bc, 8, 1}, {"DEC BC", &op::dec_bc, 8, 1}, {"INC C", &op::inc_c, 4, 1}, {"DEC C", &op::dec_c, 4, 1}, {"LD C,d8", &op::ld_c_d8, 8, 2}, {"RRCA", &op::rrca, 4, 1},
            {"STOP d8", &op::stop_d8, 4, 2}, {"LD DE,d16", &op::ld_de_d16, 12, 3}, {"LD (DE),A", &op::ld_abs_de_a, 8, 1}, {"INC DE", &op::inc_de, 8, 1}, {"INC D", &op::inc_d, 4, 1}, {"DEC D", &op::dec_d, 4, 1}, {"LD D,d8", &op::ld_d_d8, 8, 2}, {"RLA", &op::rla, 4, 1}, {"JR", &op::jr_r8, 12, 2}, {"ADD HL,DE", &op::add_hl_de, 8, 1}, {"LD A,(DE)", &op::ld_a_abs_de, 8 ,1}, {"DEC DE", &op::dec_de, 8, 1}, {"INC E", &op::inc_e, 4, 1}, {"DEC E", &op::dec_e, 4, 1}, {"LD E,d8", &op::ld_e_d8, 8, 2}, {"RRA", &op::rra, 4, 1},
            {"JR NZ,r8", &op::jr_nz_r8, 8, 2}, {"LD HL,d16", &op::ld_hl_d16, 12, 3}, {"LD (HL+),A", &op::ld_abs_hli_a, 8, 1}, {"INC HL", &op::inc_hl, 8, 1}, {"INC H", &op::inc_h, 4, 1}, {"DEC H", &op::dec_h, 4, 1}, {"LD H,d8", &op::ld_h_d8, 8, 2}, {"DAA", &op::daa, 4, 1}, {"JR Z,r8", &op::jr_z_r8, 8, 2}, {"ADD HL,HL", &op::add_hl_hl, 8, 1}, {"LD A,(HL+)", &op::ld_a_abs_hli, 8, 1}, {"DEC HL", &op::dec_hl, 8, 1}, {"INC L", &op::inc_l, 4, 1}, {"DEC L", &op::dec_l, 4, 1}, {"LD L,d8", &op::ld_l_d8, 8, 2}, {"CPL", &op::cpl, 4, 1},
            {"JR NC,r8", &op::jr_nc_r8, 8, 2}, {"LD SP,d16", &op::ld_sp_d16, 12, 3}, {"LD (HL-),A", &op::ld_abs_hld_a, 8, 1}, {"INC SP", &op::inc_sp, 8, 1}, {"INC (HL)", &op::inc_abs_hl, 12, 1}, {"DEC (HL)", &op::dec_abs_hl, 12, 1}, {"LD (HL),d8", &op::ld_abs_hl_d8, 12, 2}, {"SCF", &op::scf, 4, 1}, {"JR C,r8", &op::jr_c_r8, 8, 2}, {"ADD HL,SP", &op::add_hl_sp, 8, 1}
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
    if(l_overflow > 0xff)
        h_reg++;

    // For checking for half carry
    uint8_t h_check = ((h_reg & 0xf) + (b_reg & 0xf));
    // For checking H register overflow
    uint16_t h_overflow = h_reg + b_reg;
    h_reg += b_reg;



    // Check to see if half carry needs to be enabled
    if((h_check & 0x10) == 0x10 || h_reg == 0x10)
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

// Add the contents of the DE register pair into the HL register pair.
// Flags:
//  - N: Reset to 0
//  - H: Set to 1 if overflow from bit 11
//  - C: Set to 1 if overflow from bit 15
uint8_t SM83::add_hl_de() {
    // Check for L register overflow
    uint16_t l_overflow = l_reg + e_reg;
    l_reg += e_reg;
    if(l_overflow > 0xff)
        h_reg++;

    // Used to check for half carry
    uint8_t h_check = ((h_reg & 0xf) + (d_reg & 0xf));
    // Used to check for carry
    uint16_t h_overflow = h_reg + d_reg;
    h_reg += d_reg;

    // Check if half carry needs to be enabled
    if((h_check & 0x10) == 0x10 || h_reg == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check if carry needs to be enabled
    if(h_overflow > 0xFF)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the HL register pair into itself.
// Flags:
//  - N: Reset to 0
//  - H: Set to 1 if overflow from bit 11
//  - C: Set to 1 if overflow from bit 15
uint8_t SM83::add_hl_hl() {
    // Need to create copies of each of these before the increments.
    // After lots of testing, this seems to be the only way
    // to emulate the actual hardware properly.
    uint8_t l_old = l_reg;
    uint8_t h_old = h_reg;
    // Copy of H register
    uint16_t h_16 = h_reg;
    // Check for L register overflow
    uint16_t l_overflow = l_reg + l_old;
    l_reg += l_old;
    if(l_overflow > 0xff) {
        h_reg++;
        h_16++;
    }

    // Used to check for half carry
    uint8_t h_check = ((h_old & 0xf) + (h_old & 0xf));
    // Used to check for carry
    h_16 += h_old;
    h_reg += h_old;

    // Check if half carry needs to be enabled
    if((h_check & 0x10) == 0x10 || h_reg == 0x10 || h_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check if carry needs to be enabled
    if(h_16 > 0xFF)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// TODO: Need to fix this. Currently not working!!!!
uint8_t SM83::add_hl_sp() {
    // Create temporary variables for the H register
    uint16_t h_16 = h_reg;
    // Read in the value from the SP
    uint8_t lowByte = sp;
    uint8_t highByte = (sp >> 8);

    // Check for L overflow
    uint16_t l_overflow = l_reg + lowByte;
    l_reg += lowByte;
    if(l_overflow > 0xff) {
        h_16++;
        h_reg++;
    }
    // Check for half carry
    uint8_t h_check = (h_reg &0xf) + (highByte & 0xf);
    h_16 += highByte;
    h_reg += highByte;
    if((h_check & 0x10) == 0x10 || h_16 > 0xff || h_reg == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check for overflow
    if(h_16 > 0xff) {
        setFlag(C, 1);
    }
    else
        setFlag(C, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Get the complement of the A register, then save that into A.
// The complement of A is A with all of its bits flipped.
// Flag:
//  -N: Set to 1
//  -H: Set to 1
uint8_t SM83::cpl() {
    a_reg = ~a_reg;
    // Set sign and half carry flags
    setFlag(N, 1);
    setFlag(H, 1);
    return 0;
}

// This instruction is meant to run after an addition or subtraction on the A register.
// It is meant to adjust the value in A to conform with BCD rules.
// This article online was a great help with this particular instruction
// as it simplified BCD for me, and you can see how its influence is in my code.
// The code provided was written in Rust (I think) and I have changed it to work in C++,
// and adjusted some other things like setting/resetting flags.
// Link: https://ehaskins.com/2018-01-30%20Z80%20DAA/
// Flags:
//  -Z: Set if the value in A is 0
//  -H: Reset to 0
//  -C: Set if the value of A > 0x99 and subtraction is not being performed for adjustment
uint8_t SM83::daa() {
    // Need to add or subtract 6 when the value in each nibble is > 0x9 for least
    // significant, and 0x9 for the most significant
    int8_t correction = 0;      // Note this is a signed int
    // Check to see if the half carry flag is enabled, or the sign flag is disabled
    // and the value of the lower nibble exceeds 0x9 in register A
    if(getFlag(H) || (!getFlag(N) && (a_reg & 0xf) > 0x9))
        correction |= 0x6;
    // Check to see if the carry flag is enabled, or the sign flag is disabled
    // and the value of a_reg is greater than 0x99 (this means the high nibble is > 0x9
    if(getFlag(C) || (!getFlag(N) && a_reg > 0x99)) {
        correction |= 0x60;
        setFlag(C, 1);
    }
    else
        setFlag(C, 0);
    // Add or subtract based on sign flag
    a_reg += getFlag(N) ? -correction : correction;
    // Check zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset half carry
    setFlag(H, 0);
    return 0;
}

// Using HL as an absolute address, decrement the data at that address.
// Flag:
//  -Z: Set if result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after decrementing the data
uint8_t SM83::dec_abs_hl() {
    // Load the data that HL is pointing to
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch data
    uint8_t data = fetch();
    // Used to check half carry flag
    uint8_t h_check = (data & 0xf) - (1 &0xf);
    data--;
    // Check zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    write(addr_abs, data);
    return 0;
}

// Decrement the B register. Set according flags.
// Flags:
//  - Z: If result is 0
//  - N: Gets set to 1
//  - H: If bit 4 is set after subtracting 1 from B
uint8_t SM83::dec_b() {
    // Used to check half carry flag
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

uint8_t SM83::dec_c() {
    // Used to check half carry flag
    uint8_t h_check = ((c_reg & 0xf) - (1 & 0xf));
    c_reg--;

    // Check zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

uint8_t SM83::dec_d() {
    // Used to check half carry flag
    uint8_t h_check = ((c_reg & 0xf) - (1 & 0xf));
    d_reg--;
    // Check for zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Decrement the BC register pair.
uint8_t SM83::dec_bc() {
    if(c_reg == 0x00)
        b_reg--;
    c_reg--;
    return 0;
}

// Decrement the DE register pair.
uint8_t SM83::dec_de() {
    if(e_reg == 0x00)
        d_reg--;
    e_reg--;
    return 0;
}

// Decrement the E register.
// Flag:
//  -Z: Set if result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set to 1 after decrement

uint8_t SM83::dec_e() {
    // Used to check half carry
    uint8_t h_check = ((e_reg & 0xf) - (1 & 0xf));
    e_reg--;
    // Check zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Decrement the H register.
// Flag:
//  -Z: Set if result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set to 1 after decrement
uint8_t SM83::dec_h() {
// Used to check half carry
    uint8_t h_check = ((h_reg & 0xf) - (1 & 0xf));
    h_reg--;
    // Check zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Decrement the HL register pair.
uint8_t SM83::dec_hl() {
    if(l_reg == 0x00)
        h_reg--;
    l_reg--;
    return 0;
}

// Decrement the L register.
// Flag:
//  -Z: Set if result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set to 1 after decrement
uint8_t SM83::dec_l() {
// Used to check half carry
    uint8_t h_check = ((l_reg & 0xf) - (1 & 0xf));
    l_reg--;
    // Check zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Using HL as an absolute address, increment the data that HL points to.
// Flag:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Set if bit 4 after incrementing the data
uint8_t SM83::inc_abs_hl() {
    // Need to load the data that HL is pointing to.
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch the data
    uint8_t data = fetch();
    // Used to check half carry
    uint8_t h_check = (data & 0xf) + (1 & 0xf);
    data++;     // Increment the data
    // Check Z flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    write(addr_abs, data);
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

// Increment the C register.
// Flags:
//  -Z: Set this flag to 1 if result is 0
//  -N: Reset this flag to 0
//  -H: Set this flag to 1 if bit 4 is set after the increment
uint8_t SM83::inc_c() {
    // Used for checking half carry flag
    uint8_t h_check = ((c_reg & 0xf) + (1 &0xf));
    c_reg++;
    // Check zero
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the D register.
// Flags:
//  -Z: Set this flag to 1 if result is 0
//  -N: Reset this flag to 0
//  -H: Set this flag to 1 if bit 4 is set after the increment
uint8_t SM83::inc_d() {
    // Used to check half carry flag
    uint8_t h_check = ((d_reg & 0xf) + (1 &0xf));
    d_reg++;
    // Check zero
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the DE register pair. If E overflows increment the D register.
uint8_t SM83::inc_de() {
    e_reg++;
    // Check to see if E register wrapped back around to 0x00
    if(e_reg == 0x00)
        d_reg++;
    return 0;
}

// Increment the E register.
// Flags:
//  -Z: Set if result is 0
//  -N: Reset this flag to 0
//  -H: Set if bit 4 is set after increment
uint8_t SM83::inc_e() {

    // Used to check for half carry flag
    uint8_t h_check = ((e_reg & 0xf) + (1 & 0xf));
    e_reg++;
    // Check for zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check &0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the H register.
// Flags:
//  -Z: Set if result is 0
//  -N: Reset this flag to 0
//  -H: Set if bit 4 is set after increment
uint8_t SM83::inc_h() {
    // Used to check for half carry flag
    uint8_t h_check = ((h_reg & 0xf) + (1 & 0xf));
    h_reg++;
    // Check for zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check &0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the HL register pair. If L overflows increment the H register.
uint8_t SM83::inc_hl() {
    l_reg++;
    if(l_reg == 0x00)
        h_reg++;

    return 0;
}

// Increment the L register.
// Flags:
//  -Z: Set if result is 0
//  -N: Reset this flag to 0
//  -H: Set if bit 4 is set after increment
uint8_t SM83::inc_l() {
    // Used to check for half carry flag
    uint8_t h_check = ((l_reg & 0xf) + (1 & 0xf));
    l_reg++;
    // Check for zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check &0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the SP.
uint8_t SM83::inc_sp() {
    // Since the SP is an uint16_t, there is nothing really to do here.
    sp++;
    return 0;
}

// Jump to an address -128 - +127 memory addresses relative to the current position
// of the PC.
// Note: If you are trying to jump to a memory address relative to the address of
// the JR opcode, then you will need to subtract 2 from the offset.
// This is because JR is a 2 byte opcode, and we increment the PC after we read
// in the offset.
uint8_t SM83::jr_r8() {
    // Use a signed 8-bit int
    int8_t offset = read(pc++);
    pc += offset;
    return 0;
}

// Jump to an address -128 - +127 memory address relative to the current position
// of the PC only if the carry flag is set. Add 4 clock cycles if the condition is
// met.
uint8_t SM83::jr_c_r8() {
    // Use a signed 8-bit int
    int8_t offset = read(pc++);
    if(!getFlag(C))
        return 0;
    pc += offset;
    return 4;
}

// Jump to an address -128 - +127 memory address relative to the current position
// of the PC only if the carry flag is not set. Add 4 clock cycles if the condition is
// met.
uint8_t SM83::jr_nc_r8() {
    // Use a signed 8-bit int
    int8_t offset = read(pc++);
    if(getFlag(C))
        return 0;
    pc += offset;
    return 4;
}

// Jump to an address -128 - +127 memory address relative to the current position
// of the PC only if the zero flag is not set. Add 4 clock cycles if the condition is
// met.
uint8_t SM83::jr_nz_r8() {
    // Used a signed 8-bit int
    int8_t offset = read(pc++);
    // Check if zero flag is set
    if(getFlag(Z))
        return 0;
    pc += offset;
    return 4;
}

// Jump to an address -128 - +127 memory address relative to the current position
// of the PC only if the zero flag is set. Add 4 clock cycles if the condition is
// met.
uint8_t SM83::jr_z_r8() {
    int8_t offset = read(pc++);
    // Check if the Z flag is not set
    if(!getFlag(Z))
        return 0;
    pc +=offset;
    return 4;
}

// Load the 8-bit data value from the absolute address in the register pair BC into register A.
uint8_t SM83::ld_a_abs_bc() {
    uint16_t lowByte = c_reg;
    uint16_t highByte = b_reg;
    // Load the absolute address into addr_abs
    addr_abs = (highByte << 8) | lowByte;
    // Read in the 8-bit data value at the address in addr_abs

    a_reg = fetch();
    return 0;
}
// Load the 8-bit data value from the absolute address in the register pair DE into register A.
uint8_t SM83::ld_a_abs_de() {
    uint16_t lowByte = e_reg;
    uint16_t highByte = d_reg;
    // Load the absolute address into addr_abs
    addr_abs = (highByte << 8) | lowByte;
    a_reg = fetch();
    return 0;
}

// Load the 8-bit data value from the absolute address in the register pair HL into register A
// then, increment the HL register pair.
uint8_t SM83::ld_a_abs_hli() {
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    // Load the absolute address into addr_abs
    addr_abs = (highByte << 8) | lowByte;
    a_reg = fetch();
    inc_hl();
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

//  Using DE as an absolute address, load value in A into that address.
uint8_t SM83::ld_abs_de_a() {
    uint16_t lowByte = e_reg;
    uint16_t  highByte = d_reg;
    addr_abs = (highByte << 8) | lowByte;
    write(addr_abs, a_reg);
    return 0;
}

// Using HL as an absolute address, load the immediate 8-bit data into that address.
uint8_t SM83::ld_abs_hl_d8() {
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = read(pc++);
    write(addr_abs, data);
    return 0;
}

// Using HL as an absolute address, load the value in A into that address, then increment the HL register pair.
uint8_t SM83::ld_abs_hli_a() {
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    write(addr_abs, a_reg);
    inc_hl();
    return 0;
}

// Using HL as an absolute address, load the value in A into that address, then decrement the HL register pair.
uint8_t SM83::ld_abs_hld_a() {
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    write(addr_abs, a_reg);
    dec_hl();
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

// Load the C register with the immediate 8-bit data value.
uint8_t SM83::ld_c_d8() {
    c_reg = read(pc++);
    return 0;
}

// Load the D register with the immediate 8-bit data value.
uint8_t SM83::ld_d_d8() {
    d_reg = read(pc++);
    return 0;
}

// Load the DE register pair with the immediate 16-bit data value.
uint8_t SM83::ld_de_d16() {
    e_reg = read(pc++);
    d_reg = read(pc++);
    return 0;
}

// Load E register with the immediate 8-bit data value.
uint8_t SM83::ld_e_d8() {
    e_reg = read(pc++);
    return 0;
}
// Load H register with the immediate 8-bit data value.
uint8_t SM83::ld_h_d8() {
    h_reg = read(pc++);
    return 0;
}

// Load the HL register pair with the immediate 16-bit value.
uint8_t SM83::ld_hl_d16() {
    l_reg = read(pc++);
    h_reg = read(pc++);
    return 0;
}

// Load L register with the immediate 8-bit data value.
uint8_t SM83::ld_l_d8() {
    l_reg = read(pc++);
    return 0;
}

// Load the SP with the immediate 16-bit value.
uint8_t SM83::ld_sp_d16() {
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Create 16-bit data from low and high bytes read in from PC
    uint16_t data = (highByte << 8) | lowByte;
    sp = data;
    return 0;
}

// Rotates the bits in A register left. If the carry is enabled, that bit is fed into
// bit 9.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rla() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = a_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        a_reg = (a_reg << 1);           // Shift left 1
        a_reg |= 0x01;                  // Set bit 0
    }
    else
        a_reg = (a_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset rest of flags
    setFlag(Z, 0);
    setFlag(H, 0);
    setFlag(N, 0);
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
    if(a_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    a_reg = (a_reg << 1) | (a_reg >> 7);

    // Reset the rest of the flags
    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in A register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rra() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = a_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        a_reg = (a_reg >> 1);       // Shift right one
        a_reg |= (1 << 7);              // Enable bit 7
    }
    else
        a_reg = (a_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset other flags
    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotate the contents of register A right one.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrca() {
    // If bit 0 is set, set the carry bit
    if(a_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);

    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    a_reg = (a_reg >> 1) | (a_reg << 7);

    // Reset the rest of the flags
    setFlag(Z, 0);
    setFlag(H, 0);
    setFlag(N, 0);

    return 0;
}
// Set the carry flag. Reset the sign and half carry flags.
uint8_t SM83::scf() {
    setFlag(C, 1);
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

uint8_t SM83::stop_d8() {
    // Need to pause the Game Boy when this opcode is run
    // To exit stop check link: https://gbdev.io/pandocs/Reducing_Power_Consumption.html?highlight=stop#using-the-stop-instruction
    return 0;
}

// No operation.
uint8_t SM83::nop() {
    return 0;
}



