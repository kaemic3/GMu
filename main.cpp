#include <iostream>
#include "Bus.h"
#include "SDL_Handler.h"

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int REGISTER_X_OFFSET = 1040;

// Need to use setup SDL to test instructions
int main(int argc, char* argv[]) {
    Bus gb;
    gb.cpu.nop();
    gb.cpu.a_reg = 33;
    SDL_Handler wSDLMain;
    // Need to create some text objects here
    // Main loop flag
    bool quit = false;
    // Initialize SDL_Handler
    if(!wSDLMain.init("GMu Debug", SCREEN_WIDTH, SCREEN_HEIGHT))
        return 1;
    zText test(&wSDLMain, "Test text :3", 8, 8, "yellow", "Amstrad CPC", 16);
    zText test2(&wSDLMain, "nyahko go brrrrrrrrrrrrrrrrrrr", 8, 28, "yellow", "Amstrad CPC", 16);
    zText test3(&wSDLMain, "$0000:", 8, 48, "yellow", "Amstrad CPC", 16);
    zText reg_a(&wSDLMain, "A :", REGISTER_X_OFFSET, 8, "yellow", "Amstrad CPC", 16);
    zText reg_a_value(&wSDLMain, gb.cpu.a_reg, false, reg_a.getX() + 48, reg_a.getY(), "yellow", "Amstrad CPC", 16);
    zText pc_text(&wSDLMain, "PC:", REGISTER_X_OFFSET, 28, "yellow", "Amstrad CPC", 16);
    zText pc_value(&wSDLMain, gb.cpu.pc, true, pc_text.getX() + 48, pc_text.getY(), "yellow", "Amstrad CPC", 16);
    // Main loop
    while(!quit) {
        // Only run if there are events on the queue
        while(wSDLMain.pollEvent() != 0) {
            // User requests quit
            if(wSDLMain.eventHandler.type == SDL_QUIT) {
                quit = true;
                break;
            }
            if(wSDLMain.eventHandler.type == SDL_KEYDOWN) {
                switch(wSDLMain.eventHandler.key.keysym.sym) {
                    case SDLK_SPACE:
                        gb.cpu.pc++;
                        break;
                    case SDLK_RETURN:
                        gb.cpu.pc += 0x100;
                        break;
                    default:
                        break;
                }
            }
        }
        // Run after we check for events
        // Need a clear screen function
        wSDLMain.clearScreen();
        pc_value.updateText(gb.cpu.pc, true);
        wSDLMain.renderText();
        //test.render(5, 5);
        //test2.render();
        // Update screen
        wSDLMain.renderPresent();
    }
    wSDLMain.close();
    return 0;
}

