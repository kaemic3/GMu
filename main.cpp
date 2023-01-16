#include <iostream>
#include "Bus.h"
#include <SDL.h>
// Will probably want to make a class that handles SDL stuff
// Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Starts up SDL and creates a window
bool init();
// Loads media
bool loadMedia();
// Frees media and shuts down SDL
void close();
// The window we'll be rendering to
SDL_Window *gWindow = nullptr;
// The surface contained by the window
SDL_Surface *gScreenSurface = nullptr;
// The image we will load and show on the screen
SDL_Surface *gImage = nullptr;

bool init() {
    // Initialization flag
    bool success = true;

    // Initialize SDL
    if( SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        success = false;
    }
    else {
        // Create window
        gWindow = SDL_CreateWindow("GMu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(gWindow == nullptr){
            std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
            success = false;
        }
        else{
            // Get window surface
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }
    return success;
}

bool loadMedia(){
    // Loading success flag
    bool success = true;
    // Load splash image
    gImage = SDL_LoadBMP("../x.bmp");
    if(gImage == nullptr){
        std::cout << "Unable to load image! SDL_Error: " << SDL_GetError() << "\n";
        success = false;
    }
    return success;
}

void close() {
    // Deallocate surface
    SDL_FreeSurface(gImage);
    // Destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    // Quit SDL subsystems
    SDL_Quit();
}

// Need to use setup SDL to test instructions
int main(int argc, char* argv[]) {
    Bus gb;
    gb.cpu.nop();
    int t = gb.ram[0xFFFF];
    std::cout << t << "\n";
    // Start up SDL and create a window
    if(!init())
        std::cout << "Failed to initialize!\n";
    else {
        // Load media
        if (!loadMedia())
            std::cout << "Failed to load media\n";
        else {
            // Main loop flag
            bool quit = false;
            // Event handler
            SDL_Event e;

            // Main app loop
            while(!quit) {
                // Handle events on event queue
                while(SDL_PollEvent(&e) != 0) {
                    // User requests quit
                    if(e.type == SDL_QUIT)
                        quit = true;
                }
                // Apply image
                SDL_BlitSurface(gImage, nullptr, gScreenSurface, nullptr);
                // Update the surface
                SDL_UpdateWindowSurface(gWindow);
            }
        }
    }
    close();

    return 0;
}
