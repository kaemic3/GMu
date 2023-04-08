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
    //GMu::LoadBinaryFile("../gb_snek.gb");
    // While app is running
    while(!quit) {
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
                        // Step one clock cycle
                        GMu::gb.cpu.clock();
                        break;
                    case SDLK_s:
                        GMu::gb.cpu.sp++;
                        break;
                    case SDLK_UP:
                        GMu::main_window->HandleViewportEvent(GMu::MemoryTranslateUp);
                        break;
                    case SDLK_DOWN:
                        GMu::main_window->HandleViewportEvent(GMu::MemoryTranslateDown);
                        break;
                    case SDLK_m:
                        GMu::gb.wram[0xffff]++;
                        break;
                    case SDLK_n:
                        GMu::gb.wram[0x0000]++;
                        break;
                    case SDLK_RETURN:
                        // Do an instruction
                        do { GMu::gb.cpu.clock(); } while (!GMu::gb.cpu.complete());
                        // CPU clock runs slower than system clock, so it may complete additional clock cycles.
                        // Drain those out
                        do { GMu::gb.cpu.clock(); } while (GMu::gb.cpu.complete());
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
        // Check to see if main window is closed
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