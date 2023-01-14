#ifndef GMU_SM83_H
#define GMU_SM83_H

#include <cstdint>
#include <string>
#include <vector>

// Forward declare a bus class
class Bus;

class SM83 {
public:
    SM83();
    ~SM83();


    // SM83 registers
    uint8_t a_reg = 0x00;
    uint8_t f_reg = 0x00;
    uint8_t b_reg = 0x00;
    uint8_t c_reg = 0x00;
    uint8_t d_reg = 0x00;
    uint8_t e_reg = 0x00;
    uint8_t h_reg = 0x00;
    uint8_t l_reg = 0x00;


    // 16 bit registers
    uint16_t SP = 0x0000;
    uint16_t PC = 0x0000;

    // Interrupts
    // https://gbdev.io/pandocs/Interrupts.html

    uint8_t ime = 0x00;     // Interrupt master enable flag - 0 =  Disable all interrupts, 1 = Enable all interrupts in IE reg
    uint8_t ie_reg = 0x00;  // Interrupt enable register @ 0xFFFF
    uint8_t if_reg = 0x00;




    enum SM83_FLAGS{
        //
        C = (1 << 4),       // Carry bit
        H = (1 << 5),       // Half Carry bit
        N = (1 << 6),       // Subtraction bit
        Z = (1 << 7)        // Zero bit

    };
    // Connect the bus pointer of the CPU to an actual bus object
    void ConnectBus(Bus *n) { bus = n; }

    // Need to write functions for each opcode
    // https://gbdev.io/gb-opcodes/optables/
    uint8_t nop();

    // For illegal opcodes
    uint8_t xxx();

    // Clock function
    void clock();
    // Reset function
    void reset();
    // Interrupt function
    void interrupt();

    // Fetching
    uint8_t fetch();        // Will read from the address in addr_abs
    uint8_t fetched = 0x00;
    // Address storage
    uint16_t addr_abs = 0x0000;
    uint16_t addr_rel = 0x0000;
    // Instructions
    uint8_t opcode = 0x00;
    uint8_t cycles = 0x00;

private:
    // Pointer to a bus object
    Bus *bus = nullptr;
    // Read and write functions
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr, bool bReadOnly = false);

    // Access flags
    uint8_t GetFlag(SM83_FLAGS f);
    void SetFlag(SM83_FLAGS f);

    // Instruction struct
    struct INSTRUCTION {
        std::string mnemonic;
        uint8_t(SM83::*operate)(void) = nullptr;    // Function pointer to corresponding opcode
        uint8_t cycles = 0x00;                      // Number of clock cycles the instruction requires
        // For instructions that are conditional branching, add cycles in the function
        uint8_t bytes = 0x00;                       // Number of bytes this instruction needs
    };
    // Use initializer list to initialize the opcode table
    std::vector<INSTRUCTION> opcode_lookup;
};



#endif //GMU_SM83_H
