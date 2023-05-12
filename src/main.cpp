#include "GMu.h"

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
    GMu::gb_cart = std::make_shared<Cartridge>("../ROMs/gb_snek.gb");
    // Load the GB boot rom into the GB
    // Need to figure out how to load this independently of the cartridge, probably
    //GMu::gb_cart->load_boot_rom();
    // Insert the cartridge into the bus
    GMu::gb.insert_cartridge(GMu::gb_cart);
    // TODO: Need to load the boot rom
    // For now set pc to 0x0100 and sp to 0xfffe
    GMu::gb.cpu.pc = 0x0100;
    GMu::gb.cpu.sp = 0xfffe;
    // While app is running

    bool emulation_run = false;
    float residual_time = 0.0f;
    std::chrono::system_clock::time_point tp_1 = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point tp_2 = std::chrono::system_clock::now();

    while(!quit) {
        // Handle timing
        // Pulled from the pixel game engine
        tp_2 = std::chrono::system_clock::now();

        std::chrono::duration<float> elapsed_time = tp_2 - tp_1;
        tp_1 = tp_2;

        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT)
                quit = true;
            // Handle window events - i needs to be a reference
            for (auto i: GMu::window_list)
                i->HandleWindowEvent(e);
            // Pull up window
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
                    case SDLK_w:
                        // Up
                        GMu::gb.joypad_input = 0xff;
                        GMu::gb.joypad_input &= ~(1 << 2 | 1 << 4);
                        break;
                    case SDLK_a:
                        // Left
                        GMu::gb.joypad_input = 0xff;
                        GMu::gb.joypad_input &= ~(1 << 1 | 1 << 4);
                        break;
                    case SDLK_s:
                        // Down
                        GMu::gb.joypad_input = 0xff;
                        GMu::gb.joypad_input &= ~(1 << 3 | 1 << 4);
                        break;
                    case SDLK_d:
                        // Right
                        GMu::gb.joypad_input = 0xff;
                        GMu::gb.joypad_input &= ~(1 << 0 | 1 << 4);
                        break;
                    case SDLK_j:
                        // Select
                        GMu::gb.joypad_input = 0xff;
                        GMu::gb.joypad_input = ~(1 << 2 | 1 << 5);
                        break;
                    case SDLK_k:
                        // Start
                        GMu::gb.joypad_input = 0xff;
                        GMu::gb.joypad_input &= ~(1 << 3 | 1 << 5);
                        break;
                    case SDLK_n:
                        // B
                        GMu::gb.joypad_input = 0xff;
                        GMu::gb.joypad_input &= ~(1 << 1 | 1 << 5);
                        break;
                    case SDLK_m:
                        // A
                        GMu::gb.joypad_input = 0xff;
                        GMu::gb.joypad_input &= ~(1 << 0 | 1 << 5);
                        break;
                    case SDLK_f:
                        // Run until the entire frame has been drawn
                        do { GMu::gb.clock(); } while (!GMu::gb.ppu.frame_complete);
                        break;
                    case SDLK_RETURN:
                        // Run until one complete instruction has run
                        do { GMu::gb.clock(); } while (!GMu::gb.cpu.complete());
                        break;
                    default:
                        //GMu::gb.joypad_input = 0xff;
                        break;
                }
            }
        }

        //do { GMu::gb.clock(); } while (!GMu::gb.cpu.complete());
        if (emulation_run) {
            if (residual_time > 0.0f) {
                residual_time -= elapsed_time.count();
            }
            else {
                residual_time += (1.0f / 29.7f) - elapsed_time.count();
                do {
                    GMu::gb.clock();
                } while (!GMu::gb.ppu.frame_complete);
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
    GMu::Close();
    return 0;
}