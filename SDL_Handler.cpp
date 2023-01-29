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

// ---------------------------
// zText
// ---------------------------
zText::zText(SDL_Handler *curHandler, std::string text, int x, int y, std::string color, std::string fontType, int fontSize) {
    pHandler = curHandler; tCurText = text; tX = x; tY = y;
    setColor(color);
}
zText::~zText() {

}
// Return true if color is in the ColorMap
// Return False if the color is not in the ColorMap
bool zText::setColor(std::string color) {
    // Need to find the text color in our ColorMap
    auto it = sColorMap.find(color);
    if(it == sColorMap.end()) {
        printf("Error: Color not in sColorMap.");
        return false;
    }
    // Save the color into tFontColor
    tFontColor = it->second;
    return true;
}

bool zText::setFont(std::string font, int size) {
    // Get the font directory
    auto it = sFontMap.find(font);
    if(it == sFontMap.end()) {
        printf("Error: Font not in sFontMap.");
        return false;
    }
    std::string fontDir = it->second;
    tFont = TTF_OpenFont(fontDir.c_str(), size);

    return true;
}

bool zText::generateTexture() {
    // Free existing texture
    free();
    // Render text surface
    SDL_Surface *textSurface = TTF_RenderText_Solid(tFont, tCurText.c_str(), tFontColor);
    if (textSurface == nullptr) {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }
    // Create texture from surface
    tTexture = SDL_CreateTextureFromSurface(pHandler->pRenderer, textSurface);
    if (tTexture == nullptr) {
        printf("Unable to generate texture from rendered text! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }
    // Get dimensions
    tWidth = textSurface->w;
    tHeight = textSurface->h;
    // Free our surface
    SDL_FreeSurface(textSurface);

    return true;
}

void zText::render(int x, int y, SDL_Rect *clip, double angle, SDL_Point *center, SDL_RendererFlip flip) {
    // set rendering space and render to the screen
    SDL_Rect renderQuad = {x, y, tWidth, tHeight};
    // Set clip rendering dimensions
    if (clip) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    // Render text surface
    SDL_RenderCopyEx(pHandler->pRenderer, tTexture, clip, &renderQuad, angle, center, flip);
}

void zText::free() {
    // Free texture if it exists
    if(tTexture) {
        SDL_DestroyTexture(tTexture);
        tTexture = nullptr;
        tWidth = 0;
        tHeight = 0;
    }
}
std::map<std::string, std::string> zText::sFontMap = {
    {"Amstrad CPC", "../Fonts/amstrad_cpc464.ttf"},
    {"Fira Code", "../Fonts/FiraCode.ttf"}
};
std::map<std::string, SDL_Color> zText::sColorMap = {
        { "Yellow", {243, 243, 13} }
};
