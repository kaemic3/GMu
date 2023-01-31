#include <iostream>
#include <iomanip>
#include "Bus.h"
#include "SDL_Handler.h"

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int REGISTER_X_OFFSET = 1040;

// Used to convert decimal value to hex string
std::string toHex(int num) {
    std::stringstream stream;
    stream << std::hex << num;
    return stream.str();
}
// Need to use setup SDL to test instructions
int main(int argc, char* argv[]) {
    Bus gb;
    gb.cpu.nop();
    int t = gb.ram[0xFFFF];
    gb.cpu.a_reg = 33;
    std::cout << t << "\n";
    SDL_Handler wSDLMain;
    // Need to create some text objects here
    // Main loop flag
    bool quit = false;
    // Initialize SDL_Handler
    if(!wSDLMain.init("GMu Debug", SCREEN_WIDTH, SCREEN_HEIGHT))
        return 1;
    zText test(&wSDLMain, "Test text :3", 5, 5, "yellow", "Amstrad CPC", 16);
    zText test2(&wSDLMain, "nyakomode go brrrrrrrrrrrrrrrrrrr", 5, 25, "yellow", "Amstrad CPC", 16);
    zText test3(&wSDLMain, "$0000:", 5, 45, "yellow", "Amstrad CPC", 16);
    zText reg_a(&wSDLMain, "A: ", REGISTER_X_OFFSET, 5, "yellow", "Amstrad CPC", 16);
    zText reg_a_contents(&wSDLMain, toHex(gb.cpu.a_reg), reg_a.getX() + 32, reg_a.getY(), "yellow", "Amstrad CPC", 16);
    // Main loop
    while(!quit) {
        // Only run if there are events on the queue
        while(wSDLMain.pollEvent() != 0) {
            // User requests quit
            if(wSDLMain.eventHandler.type == SDL_QUIT)
                quit = true;
        }
        // Run after we check for events
        // Need a clear screen function
        wSDLMain.clearScreen();
        wSDLMain.renderText();
        //test.render(5, 5);
        //test2.render();
        // Update screen
        wSDLMain.renderPresent();
    }
    wSDLMain.close();
    return 0;
}

