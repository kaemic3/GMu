#include <iostream>
#include "Bus.h"
#include <SDL.h>

int main(int argc, char* argv[]) {
    Bus sm83_bus;
    sm83_bus.cpu.nop();
    int t = sm83_bus.ram[0xFFFF];
    std::cout << t << std::endl;

    return 0;
}
