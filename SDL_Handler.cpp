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

void SDL_Handler::renderText() {
    // Iterate through sTextList and call of their render functions
    for(auto text : sTextList) {
        if(!text)
            printf("Error zText is a nullptr\n");
        text->render();
    }
}

void SDL_Handler::close() {
    // Need to iterate through our list of zText objects and free them all
    for(auto text : sTextList)
        text->free();
    // Reset the TextList
    SDL_Handler::sTextList = {};
    // Destroy window and renderer
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    pRenderer = nullptr;
    pWindow = nullptr;
    // Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();
}

// Static member definitions
std::vector<zText*> SDL_Handler::sTextList = {};

// ---------------------------
// zText
// ---------------------------
zText::zText(SDL_Handler *curHandler, std::string text, int x, int y, std::string color, std::string fontType, int fontSize) {
    pHandler = curHandler; tCurText = text; tX = x; tY = y; tFontSize = fontSize;
    setColor(color);
    setFont(fontType);
    generateTexture();
    SDL_Handler::sTextList.push_back(this);
}
zText::zText(SDL_Handler *curHandler, int num, bool u16, int x, int y, std::string color, std::string fontType, int fontSize) {
    pHandler = curHandler; tCurText = toHexString(num, u16); tX = x; tY = y; tFontSize = fontSize;
    setColor(color);
    setFont(fontType);
    generateTexture();
    SDL_Handler::sTextList.push_back(this);
}

zText::zText(SDL_Handler *curHandler, int num, bool u16, bool binary, int x, int y, std::string color, std::string fontType, int fontSize) {

    pHandler = curHandler; tCurText = toHexString(num, u16, binary); tX = x; tY = y; tFontSize = fontSize;
    setColor(color);
    setFont(fontType);
    generateTexture();
    SDL_Handler::sTextList.push_back(this);
}
zText::~zText() {
    free();
}
void zText::updateText(std::string nText) {
    tCurText = nText;
    // Now generate a new text texture
    generateTexture();
}

void zText::updateText(int num, bool u16, bool binary) {
    tCurText = toHexString(num, u16, binary);
    generateTexture();
}

std::string zText::toHexString(int num, bool u16, bool binary) {
    std::stringstream stream;
    if(u16) {
        stream << std::setfill('0') << std::setw(sizeof (uint16_t) * 2) << std::hex << num;
        return stream.str();
    }
    else if(binary) {
        return std::bitset<8>(num).to_string();
    }
    else {
        stream << std::setfill('0') << std::setw(sizeof (uint8_t) * 2) << std::hex << num;
        return stream.str();
    }

}
bool zText::generateTexture() {
    // Free existing texture
    freeTexture();
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
    setDimensions(textSurface);
    // Free our surface
    SDL_FreeSurface(textSurface);

    return true;
}
// Helper function for setting dimensions private members
void zText::setDimensions(SDL_Surface* curSurface) {
    tWidth = curSurface->w;
    tHeight = curSurface->h;
}
void zText::render() {
    render(tX, tY);
}
void zText::render(int x, int y, SDL_Rect *clip, double angle, SDL_Point *center, SDL_RendererFlip flip) {
    // set rendering space and render to the screen
    SDL_Rect renderQuad = {x, y, tWidth, tHeight};
    // Update our X Y coordinates
    setPosition(x, y);
    // Set clip rendering dimensions
    if (clip) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    // Render text surface
    SDL_RenderCopyEx(pHandler->pRenderer, tTexture, clip, &renderQuad, angle, center, flip);
}
bool zText::setPosition(int x, int y) {
    // Check to see if the coordinates are the same
    if(tX == x && tY == y)
        return false;
    // If not update our members
    tX = x;
    tY=y;
    return true;

}
void zText::freeTexture() {
    // Free texture if it exists
    if(tTexture) {
        SDL_DestroyTexture(tTexture);
        tTexture = nullptr;
        tWidth = 0;
        tHeight = 0;
    }
}
void zText::free() {
    freeTexture();
    // Free font if it exists
    if(tFont) {
        TTF_CloseFont(tFont);
        tFont = nullptr;
    }
}
// Getter and setter member functions
int zText::getWidth() { return tWidth; }
int zText::getHeight() { return tHeight; }
int zText::getX() { return tX; }
int zText::getY() { return tY; }

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

bool zText::setFont(std::string font) {
    // Get the font directory
    auto it = sFontMap.find(font);
    if(it == sFontMap.end()) {
        printf("Error: Font not in sFontMap.");
        return false;
    }
    std::string fontDir = it->second;
    tFont = TTF_OpenFont(fontDir.c_str(), tFontSize);
    if(tFont == nullptr) {
        printf("Unable to open font! SDL_Error: %s\n", TTF_GetError());
        return false;
    }

    return true;
}
// Static member definitions
std::map<std::string, std::string> zText::sFontMap = {
    {"Amstrad CPC", "../Fonts/amstrad_cpc464.ttf"},
    {"Fira Code", "../Fonts/FiraCode.ttf"}
};
std::map<std::string, SDL_Color> zText::sColorMap = {
        { "yellow", {243, 243, 13} }
};
