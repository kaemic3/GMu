#include <iostream>
#include "Bus.h"
#include "SDL_Handler.h"


// Need to use setup SDL to test instructions
int main(int argc, char* argv[]) {
    Bus gb;

    gb.cpu.a_reg = 0x10;
    gb.cpu.b_reg = 0x01;
    gb.cpu.c_reg = 0x32;
    gb.cpu.h_reg = 0x80;
    gb.cpu.l_reg = 0x00;
    gb.cpu.d_reg = 0xef;
    gb.cpu.e_reg = 0x2f;

    gb.cpu.sp = 0xfffe;
    gb.cpu.sp = 0xffff;
    gb.ram[0x0000] = 0xb8;

    gb.ram[0x7fff] = 0x33;
    gb.ram[0x8000] = 0xe9;




    SDL_Handler wSDLMain;
    // Main loop flag
    bool quit = false;
    // Initialize SDL_Handler
    if(!wSDLMain.init("GMu Debug", SCREEN_WIDTH, SCREEN_HEIGHT))
        return 1;

    // Debug text
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

    zText reg_flag(&wSDLMain, "Flags:", REGISTER_X_OFFSET, 108, "yellow", "Amstrad CPC", 16);
    zText reg_flag_value(&wSDLMain, gb.cpu.f_reg, false, true, reg_flag.getX() + REGISTER_FLAG_OFFSET, reg_flag.getY(), "yellow", "Amstrad CPC", 16);
    zText reg_flag_key(&wSDLMain, "ZNHC----", reg_flag_value.getX(), reg_flag_value.getY() + 20, "yellow", "Amstrad CPC", 16);

    zText pc_text(&wSDLMain, "PC:", REGISTER_X_OFFSET, 168, "yellow", "Amstrad CPC", 16);
    zText pc_value(&wSDLMain, gb.cpu.pc, true, pc_text.getX() + 48, pc_text.getY(), "yellow", "Amstrad CPC", 16);
    zText sp_text(&wSDLMain, "SP:", REGISTER_X_OFFSET, 188, "yellow", "Amstrad CPC", 16);
    zText sp_value(&wSDLMain, gb.cpu.sp, true, sp_text.getX() + REGISTER_VALUE_OFFSET, sp_text.getY(), "yellow", "Amstrad CPC", 16);
    zText clock_text(&wSDLMain, "Cycles: ", REGISTER_X_OFFSET, sp_value.getY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    zText clock_value(&wSDLMain, gb.cpu.cycles, false, clock_text.getX() + clock_text.getWidth(), sp_value.getY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);

    // Text to show RAM contents
    zMemoryText test0(&wSDLMain, gb, 0x0000, MEMORY_BASE_OFFSET, MEMORY_BASE_OFFSET, "yellow", "Amstrad CPC", 16);
    zMemoryText test1(&wSDLMain, gb, 0x0010, MEMORY_BASE_OFFSET, test0.getBaseY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    zMemoryText test2(&wSDLMain, gb, 0x0020, MEMORY_BASE_OFFSET, test1.getBaseY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    zMemoryText test3(&wSDLMain, gb, 0x0030, MEMORY_BASE_OFFSET, test2.getBaseY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    zMemoryText test4(&wSDLMain, gb, 0x0040, MEMORY_BASE_OFFSET, test3.getBaseY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    zMemoryText test5(&wSDLMain, gb, 0x0050, MEMORY_BASE_OFFSET, test4.getBaseY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    zMemoryText test6(&wSDLMain, gb, 0x0060, MEMORY_BASE_OFFSET, test5.getBaseY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    zMemoryText test7(&wSDLMain, gb, 0x0070, MEMORY_BASE_OFFSET, test6.getBaseY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);

    zMemoryText test8(&wSDLMain, gb, 0x7FF0, MEMORY_BASE_OFFSET, test7.getBaseY() + LINE_OFFSET + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    zMemoryText test9(&wSDLMain, gb, 0x8000, MEMORY_BASE_OFFSET, test8.getBaseY() + LINE_OFFSET, "yellow", "Amstrad CPC", 16);
    // Main loop
    while(!quit) {
        // Only run if there are events on the queue
        while(wSDLMain.pollEvent() != 0) {
            // User requests quit
            if(wSDLMain.eventHandler.type == SDL_QUIT) {
                quit = true;
                break;
            }
            // Get keyboard input
            if(wSDLMain.eventHandler.type == SDL_KEYDOWN) {
                switch(wSDLMain.eventHandler.key.keysym.sym) {
                    case SDLK_SPACE:
                        // Step one clock cycle
                        gb.cpu.clock();
                        break;
                    case SDLK_RETURN:
                        // Step to next opcode
                        gb.cpu.cycles = 0;
                        gb.cpu.clock();
                        break;
                    case SDLK_RSHIFT:
                        gb.cpu.sp++;
                        break;
                    case SDLK_LSHIFT:
                        gb.cpu.sp += 0x100;
                        break;
                    case SDLK_a:
                        gb.cpu.a_reg++;
                        break;
                    case SDLK_f:
                        gb.cpu.f_reg++;
                        break;
                    case SDLK_b:
                        gb.cpu.b_reg++;
                        break;
                    case SDLK_c:
                        gb.cpu.c_reg++;
                        break;
                    case SDLK_d:
                        gb.cpu.d_reg++;
                        break;
                    case SDLK_e:
                        gb.cpu.e_reg++;
                        break;
                    case SDLK_h:
                        gb.cpu.h_reg++;
                        break;
                    case SDLK_l:
                        gb.cpu.l_reg++;
                        break;
                    default:
                        break;
                }
            }
        }
        // Run after we check for events
        // Need a clear screen function
        wSDLMain.clearScreen();
        // Update debug text
        reg_a_value.updateText(gb.cpu.a_reg);
        reg_f_value.updateText(gb.cpu.f_reg);
        reg_b_value.updateText(gb.cpu.b_reg);
        reg_c_value.updateText(gb.cpu.c_reg);
        reg_d_value.updateText(gb.cpu.d_reg);
        reg_e_value.updateText(gb.cpu.e_reg);
        reg_h_value.updateText(gb.cpu.h_reg);
        reg_l_value.updateText(gb.cpu.l_reg);
        reg_flag_value.updateText(gb.cpu.f_reg, false, true);
        pc_value.updateText(gb.cpu.pc, true);
        sp_value.updateText(gb.cpu.sp, true);
        clock_value.updateText(std::to_string(gb.cpu.cycles));
        test0.update();
        test1.update();
        test2.update();
        test3.update();
        test4.update();
        test5.update();
        test6.update();
        test7.update();
        test8.update();
        test9.update();
        // Render the text
        wSDLMain.renderText();

        // Update screen
        wSDLMain.renderPresent();
    }
    wSDLMain.close();
    return 0;
}

