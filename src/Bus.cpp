#include "Bus.h"

Bus::Bus() {
    // Clear RAM
    for(uint8_t i : wram) i = 0x00;

    // connect cpu to bus
    cpu.ConnectBus(this);
}

Bus::~Bus() {
}

void Bus::cpu_write(uint16_t addr, uint8_t data) {
    // Check if the passed address is valid
    if(addr >= 0x0000 && addr <= 0xFFFF)
        wram[addr] = data;
}

uint8_t Bus::cpu_read(uint16_t addr, bool bReadOnly) {
        // Check if the passed address is valid
        if(addr >= 0x0000 && addr <= 0xFFFF)
            return wram[addr] ;
        // If the address in not valid, return 0
        return 0x00;
}