#include "GMu.h"
#include "Disassembler.h"

// Time for the clock thread to sleep for 59.73 fps
#define FRAME_TIME_MS 16.74

int main(int argc, char *argv[]) {
    // Need to pre initialize smart pointers for all windows that can be created.
    GMu::main_window = new GMu::zMainWindow(&GMu::gb);
    // Store the window in windowList, a vector that holds pointers to all of our zWindows
    GMu::window_list.push_back(GMu::main_window);
    if(!GMu::Init()) {
        printf("Failed to initialize!\n");
        return 1;
    }
    bool quit = false;
    // Event handler
    SDL_Event e;
    // Load the example ROM into memory
    GMu::gb_cart = std::make_shared<Cartridge>("../ROMS/05-op rp.gb");
    // Load the GB boot rom into the GB
    // Need to figure out how to load this independently of the cartridge, probably
    //GMu::gb_cart->load_boot_rom();
    // Insert the cartridge into the bus
    GMu::gb.insert_cartridge(GMu::gb_cart);
    // TODO: Need to load the boot rom
    // For now set pc to 0x0100 and sp to 0xfffe
    GMu::gb.cpu.pc = 0x0100;
    GMu::gb.cpu.sp = 0xfffe;
    GMu::gb.if_reg.data = 0xe1;
    // While app is running
    bool emulation_run = false;
    std::chrono::system_clock::time_point tp_1 = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point tp_2 = std::chrono::system_clock::now();
    // Disassembler
    //Disassembler dis ("../debug_out.txt", &GMu::gb);
    while(!quit) {
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT)
                quit = true;
            // Handle window events - i needs to be a pointer
            for (auto i: GMu::window_list)
                i->HandleWindowEvent(e);
            // Get input
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_1:
                        GMu::window_list[0]->Focus();
                        break;
                    case SDLK_SPACE:
                        // Toggle the run state of the emulator
                        emulation_run = !emulation_run;
                        break;
                    case SDLK_UP:
                        GMu::main_window->HandleViewportEvent(GMu::MemoryTranslateUp);
                        break;
                    case SDLK_DOWN:
                        GMu::main_window->HandleViewportEvent(GMu::MemoryTranslateDown);
                        break;
                    case SDLK_r:
                        GMu::gb.reset();
                        break;
                    case SDLK_f:
                        // Run until the entire frame has been drawn
                        do {
                            GMu::gb.clock();
                            //if (GMu::gb.cpu.complete())
                                //dis.output_instruction(GMu::gb.cpu.return_instruction(), GMu::gb.cpu.debug_pc);
                        } while (!GMu::gb.ppu.get_frame_state());
                        break;
                    case SDLK_RETURN:
                        // Run until one complete instruction has run
                        do { GMu::gb.clock(); } while (!GMu::gb.cpu.complete());
                        //dis.output_instruction(GMu::gb.cpu.return_instruction(), GMu::gb.cpu.debug_pc);
                        break;
                    case SDLK_v:
                        do { GMu::gb.clock(); } while (!GMu::gb.ppu.get_vblank_flag() && GMu::gb.ie_reg.vblank != 1);
                        //dis.output_instruction(GMu::gb.cpu.return_instruction(), GMu::gb.cpu.debug_pc);
                    default:
                        break;
                }
            }
            // Joypad input
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                switch (e.key.keysym.sym) {
                    case SDLK_w:
                        // Up
                        GMu::gb.joypad_directional &= ~(1 << 2);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_a:
                        // Left
                        GMu::gb.joypad_directional &= ~(1 << 1);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_s:
                        // Down
                        GMu::gb.joypad_directional &= ~(1 << 3);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_d:
                        // Right
                        GMu::gb.joypad_directional &= ~(1 << 0);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_j:
                        // Select
                        GMu::gb.joypad_action &= ~(1 << 2);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_k:
                        // Start
                        GMu::gb.joypad_action &= ~(1 << 3);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_n:
                        // B
                        GMu::gb.joypad_action &= ~(1 << 1);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_m:
                        // A
                        GMu::gb.joypad_action &= ~(1 << 0);
                        GMu::gb.joypad_state_change = true;
                        break;
                }

            }
            // If the key is released, then reset the key_pressed flag
            else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_w:
                        GMu::gb.joypad_directional |= (1 << 2);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_a:
                        GMu::gb.joypad_directional |= (1 << 1);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_s:
                        GMu::gb.joypad_directional |= (1 << 3);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_d:
                        GMu::gb.joypad_directional |= (1 << 0);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_j:
                        GMu::gb.joypad_action |= (1 << 2);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_k:
                        GMu::gb.joypad_action |= (1 << 3);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_n:
                        GMu::gb.joypad_action |= (1 << 1);
                        GMu::gb.joypad_state_change = true;
                        break;
                    case SDLK_m:
                        GMu::gb.joypad_action |= (1 << 0);
                        GMu::gb.joypad_state_change = true;
                        break;
                }
            }
        }
        // Handle the timing, the system should only clock every 16.74ms (roughly, 59.73 fps).
        tp_1 = std::chrono::system_clock::now();
        std::chrono::duration<float, std::milli> elapsed_time = tp_1 - tp_2;
        if (emulation_run) {
            if (elapsed_time.count() < FRAME_TIME_MS) {
                std::chrono::system_clock::now();
                std::chrono::duration<float, std::milli> delta_ms(FRAME_TIME_MS - elapsed_time.count());
                auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
                std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration));
            }
            else {
                tp_2 = std::chrono::system_clock::now();
                do {
                    GMu::gb.clock();
                    //if (GMu::gb.cpu.complete())
                        //dis.output_instruction(GMu::gb.cpu.return_instruction(), GMu::gb.cpu.debug_pc);
                } while (!GMu::gb.ppu.get_frame_state());
                // Reset joypad state flag
                GMu::gb.joypad_state_change = false;
            }
        }

        // *** Need to Update this section to have a loop that updates all windows ***
        // Run Update on viewports
        GMu::main_window->UpdateViewports();
        // Render window backgrounds
        for (auto i: GMu::window_list)
            i->RenderBG();
        // Render our viewports
        GMu::main_window->RenderViewports();
        // Show window renderer
        GMu::window_list[0]->ShowRenderer();
        // Check to see if main window is closed - Need to change so the program kills only on the close event,
        // rather than if the window is minimized/closed
        if(!GMu::main_window->IsShown())
            quit = true;
        // Check to see if any sub windows were closed
        for(auto w : GMu::window_list) {
            if(!w->IsShown()) {
                // Free the SDL memory
                w->Free();
                // Remove the window from the vector
                GMu::window_list.erase(std::find(GMu::window_list.begin(), GMu::window_list.end(), w));
                delete w;
            }
        }
    }
    //dis.close_file();
    GMu::Close();
    return 0;
}