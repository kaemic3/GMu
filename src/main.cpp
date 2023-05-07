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
    while(!quit) {
       // do { GMu::gb.clock(); } while (!GMu::gb.ppu.frame_complete);
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
                    case SDLK_a:
                        GMu::gb.cpu.a_reg++;
                        break;
                    case SDLK_f:
                        GMu::gb.cpu.f_reg++;
                        break;
                    case SDLK_SPACE:
                        // Run until one complete instruction has run
                        do { GMu::gb.clock(); } while (!GMu::gb.cpu.complete());
                        break;
                    case SDLK_s:
                        GMu::gb.cpu.sp++;
                        break;
                    case SDLK_p:
                        GMu::gb.cpu.pc = 0x0100;
                        break;
                    case SDLK_UP:
                        GMu::main_window->HandleViewportEvent(GMu::MemoryTranslateUp);
                        break;
                    case SDLK_DOWN:
                        GMu::main_window->HandleViewportEvent(GMu::MemoryTranslateDown);
                        break;
                    case SDLK_m:
                        GMu::gb.cpu_write(0xdfff, (GMu::gb.cpu_read(0xdfff) + 1));
                        break;
                    case SDLK_n:
                        GMu::gb.cpu_write(0xe000, (GMu::gb.cpu_read(0xc000) + 1));
                        break;
                    case SDLK_RETURN:
                        // Run until a complete frame
                        do { GMu::gb.clock(); } while (!GMu::gb.ppu.frame_complete);
                        break;
                }
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