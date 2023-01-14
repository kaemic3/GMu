#include "SM83.h"
#include "Bus.h"

SM83::SM83() {
    // to clean up the opcode table
    using op = SM83;

    opcode_lookup =
    {
            {"NOP", &op::nop, 4, 1}
    };
}

uint8_t SM83::read(uint16_t addr, bool bReadOnly){

}

void SM83::clock() {

}

uint8_t SM83::nop() {

}
