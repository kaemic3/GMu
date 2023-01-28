#include "SDL_Handler.h"
#include <cstdio>

SDL_Handler::SDL_Handler() {
    pWindow = nullptr;
    pRenderer = nullptr;
}

SDL_Handler::~SDL_Handler() {

}

bool SDL_Handler::init(const std::string &title, const int wScreen, const int hScreen) {
    // Initialize SDL subsystems using guards instead of nested if

    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO < 0)) {
        printf("Failed to initialize SDL! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    // Initialize SDL_ttf
    if(TTF_Init() == -1) {
        printf("Failed to initialize SDL_ttf! SDL Error: %s\n", TTF_GetError());
    }
    // Set texture filtering
    if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        printf("Warning: Linear texture filtering is not enabled");
    // Create window
    pWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, wScreen, hScreen, SDL_WINDOW_SHOWN);
    if(pWindow == nullptr) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    // Create V-Synced renderer for our window
    pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (pRenderer == nullptr) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    // Set renderer color
    SDL_SetRenderDrawColor(pRenderer, 0, 2, 107, 0xFF);
    return true;
}

int SDL_Handler::pollEvent() {
    return SDL_PollEvent(&eventHandler);
}
void SDL_Handler::clearScreen() {
    SDL_SetRenderDrawColor(pRenderer, 0, 2, 107, 0xFF);
    SDL_RenderClear(pRenderer);
}
void SDL_Handler::renderPresent() {
    SDL_RenderPresent(pRenderer);
}
zText::zText(SDL_Handler *curHandler, std::string text, int x, int y, std::string color, std::string fontType, int fontSize) {
    pHandler = curHandler; tCurText = text; tX = x; tY = y;
}
zText::~zText() {

}
// Return true if color is in the ColorMap
// Return False if the color is not in the ColorMap
bool zText::setColor(std::string color) {
    // Need to find the text color in our ColorMap
}

std::map<std::string, std::string> zText::sFontMap = {
    {"Amstrad CPC", "../Fonts/amstrad_cpc464.ttf"},
    {"Fira Code", "../Fonts/FiraCode.ttf"}
};
std::map<std::string, SDL_Color> zText::sColorMap = {
        { "Yellow", {243, 243, 13} }
};
