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
    uint16_t sp = 0x0000;
    uint16_t pc = 0x0000;

    // Interrupts
    // https://gbdev.io/pandocs/Interrupts.html

    uint8_t ime = 0x00;     // Interrupt master enable flag - 0 =  Disable all interrupts, 1 = Enable all interrupts in IE reg
    uint8_t ie_reg = 0x00;  // Interrupt enable register @ 0xFFFF
    uint8_t if_reg = 0x00;

    // Use f_reg for flags
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
    uint8_t nop();           uint8_t add_hl_bc();     uint8_t add_hl_de();  uint8_t add_hl_hl(); uint8_t cpl(); uint8_t daa();   uint8_t dec_b();              uint8_t dec_bc();           uint8_t dec_c();           uint8_t dec_d();           uint8_t dec_de();  uint8_t dec_e(); uint8_t dec_h(); uint8_t dec_hl(); uint8_t dec_l();
    uint8_t inc_b();         uint8_t inc_bc();        uint8_t inc_c();           uint8_t inc_d();              uint8_t inc_de();           uint8_t inc_e();   uint8_t inc_h(); uint8_t inc_hl();  uint8_t inc_l();     uint8_t jr_r8();              uint8_t jr_nz_r8();    uint8_t jr_z_r8();      uint8_t ld_a_abs_bc();
    uint8_t ld_a_abs_de(); uint8_t ld_a_abs_hli(); uint8_t ld_abs_hli_a(); uint8_t ld_abs_a16_sp(); uint8_t ld_abs_bc_a();     uint8_t ld_abs_de_a();        uint8_t ld_b_d8();          uint8_t ld_bc_d16();       uint8_t ld_c_d8();         uint8_t ld_d_d8();
    uint8_t ld_de_d16();     uint8_t ld_e_d8();    uint8_t ld_h_d8();   uint8_t ld_hl_d16();    uint8_t ld_l_d8();    uint8_t rla();              uint8_t rlca();            uint8_t rra();             uint8_t rrca();            uint8_t stop_d8();

    // For illegal opcodes
    uint8_t xxx();

    // Clock function
    void clock();
    // Helper function to tell when an instruction has finished
    bool complete();
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
    uint8_t getFlag(SM83_FLAGS f);                  // Used to check a specific flag
    void setFlag(SM83_FLAGS f, bool v);             // Used to set a specific flag, bool v is used to determine set or reset

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
