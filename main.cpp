#include <iostream>
#include "Bus.h"
#include "SDL_Handler.h"

// Will probably want to make a class that handles SDL stuff
// Will also need to make a class for text on the screen

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Starts up SDL and creates a window
bool init();
// Loads media
bool loadMedia();
// Frees media and shuts down SDL
void close();
// The window we'll be rendering to
SDL_Window *gWindow = nullptr;
// Our renderer
SDL_Renderer *gRenderer = nullptr;



// Need to use setup SDL to test instructions
int main(int argc, char* argv[]) {
    Bus gb;
    gb.cpu.nop();
    int t = gb.ram[0xFFFF];
    std::cout << t << "\n";
    SDL_Handler test;

    return 0;
}
