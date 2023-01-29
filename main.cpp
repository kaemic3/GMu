#include <iostream>
#include "Bus.h"
#include "SDL_Handler.h"

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Need to use setup SDL to test instructions
int main(int argc, char* argv[]) {
    Bus gb;
    gb.cpu.nop();
    int t = gb.ram[0xFFFF];
    std::cout << t << "\n";
    SDL_Handler wSDLMain;
    // Need to create some text objects here
    // Main loop flag
    bool quit = false;
    // Initialize SDL_Handler
    if(!wSDLMain.init("GMu Debug", SCREEN_WIDTH, SCREEN_HEIGHT))
        return 1;
    zText test(&wSDLMain, "Test text :3", 0, 0, "yellow", "Amstrad CPC", 16);
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
        test.render(1000, 40);
        // Update screen
        wSDLMain.renderPresent();
    }
    wSDLMain.close();
    return 0;
}
