#include <iostream>
#include "Bus.h"
#include "SDL_Handler.h"

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int REGISTER_X_OFFSET = SCREEN_WIDTH - 240;
const int REGISTER_VALUE_OFFSET = 48;
const int REGISTER_PAIR_OFFSET = 64;

// Need to use setup SDL to test instructions
int main(int argc, char* argv[]) {
    Bus gb;
    gb.cpu.nop();
    gb.cpu.a_reg = 33;
    SDL_Handler wSDLMain;
    // Main loop flag
    bool quit = false;
    // Initialize SDL_Handler
    if(!wSDLMain.init("GMu Debug", SCREEN_WIDTH, SCREEN_HEIGHT))
        return 1;
    zText reg_a(&wSDLMain, "A:", REGISTER_X_OFFSET, 8, "yellow", "Amstrad CPC", 16);
    zText reg_a_value(&wSDLMain, gb.cpu.a_reg, false, reg_a.getX() + REGISTER_VALUE_OFFSET, reg_a.getY(), "yellow", "Amstrad CPC", 16);

    zText reg_f(&wSDLMain, "F:", reg_a_value.getX() + REGISTER_PAIR_OFFSET, reg_a.getY(), "yellow", "Amstrad CPC", 16);
    zText reg_f_value(&wSDLMain, gb.cpu.f_reg, false, reg_f.getX() + REGISTER_VALUE_OFFSET, reg_f.getY(), "yellow", "Amstrad CPC", 16);

    zText reg_b(&wSDLMain, "B:", REGISTER_X_OFFSET, 28, "yellow", "Amstrad CPC", 16);
    zText reg_b_value(&wSDLMain, gb.cpu.b_reg, false, reg_b.getX() + REGISTER_VALUE_OFFSET, reg_b.getY(), "yellow", "Amstrad CPC", 16);

    zText reg_c(&wSDLMain, "C:", reg_b_value.getX() + REGISTER_PAIR_OFFSET, reg_b.getY(), "yellow", "Amstrad CPC", 16);
    zText reg_c_value(&wSDLMain, gb.cpu.c_reg, false, reg_c.getX() + REGISTER_VALUE_OFFSET, reg_c.getY(), "yellow", "Amstrad CPC", 16);

    zText reg_d(&wSDLMain, "D:", REGISTER_X_OFFSET, 48, "yellow", "Amstrad CPC", 16);
    zText reg_d_value(&wSDLMain, gb.cpu.d_reg, false, reg_d.getX() + REGISTER_VALUE_OFFSET, reg_d.getY(), "yellow", "Amstrad CPC", 16);

    zText reg_e(&wSDLMain, "E:", reg_d_value.getX() + REGISTER_PAIR_OFFSET, reg_d.getY(), "yellow", "Amstrad CPC", 16);
    zText reg_e_value(&wSDLMain, gb.cpu.e_reg, false, reg_e.getX() + REGISTER_VALUE_OFFSET, reg_e.getY(), "yellow", "Amstrad CPC", 16);

    zText reg_h(&wSDLMain, "H:", REGISTER_X_OFFSET, 68, "yellow", "Amstrad CPC", 16);
    zText reg_h_value(&wSDLMain, gb.cpu.h_reg, false, reg_h.getX() + REGISTER_VALUE_OFFSET, reg_h.getY(), "yellow", "Amstrad CPC", 16);

    zText reg_l(&wSDLMain, "L:", reg_h_value.getX() + REGISTER_PAIR_OFFSET, reg_h.getY(), "yellow", "Amstrad CPC", 16);
    zText reg_l_value(&wSDLMain, gb.cpu.h_reg, false, reg_l.getX() + REGISTER_VALUE_OFFSET, reg_l.getY(), "yellow", "Amstrad CPC", 16);

    // Need to make a function to draw the binary version of the flag register
    // zText reg_flag

    zText pc_text(&wSDLMain, "PC:", REGISTER_X_OFFSET, 148, "yellow", "Amstrad CPC", 16);
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

        // Update screen
        wSDLMain.renderPresent();
    }
    wSDLMain.close();
    return 0;
}

