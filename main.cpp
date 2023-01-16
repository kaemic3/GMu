#include <iostream>
#include "Bus.h"
#include <SDL.h>

// Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Need to use setup SDL to test instructions
int main(int argc, char* argv[]) {
    Bus gb;
    gb.cpu.nop();
    int t = gb.ram[0xFFFF];
    std::cout << t << "\n";
    // This is the window we will be rendering to
    SDL_Window *window = nullptr;

    // The surface contained by the window
    SDL_Surface *screenSurface = nullptr;
    // Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0)
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
    else {
        // Create window
        window = SDL_CreateWindow("GMu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if( window == nullptr )
            std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        else {
            // Get window surface
            screenSurface = SDL_GetWindowSurface(window);
            // Fill the surface white
            SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
            // Update the surface
            SDL_UpdateWindowSurface(window);
            // Hack to get the window to stay up
            SDL_Event e; bool quit = false; while( quit == false ){ while( SDL_PollEvent( &e ) ){ if( e.type == SDL_QUIT ) quit = true; } }
        }
    }
    // Destroy window
    SDL_DestroyWindow(window);
    // Quit SDL subsystems
    SDL_Quit();

    return 0;
}
