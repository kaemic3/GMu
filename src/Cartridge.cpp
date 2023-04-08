#include "Cartridge.h"

Cartridge::Cartridge() {

}

bool Cartridge::cpu_write(uint16_t addr, uint8_t data) {
    return false;
}

bool Cartridge::cpu_read(uint16_t addr, bool read_only) {
    return false;
}