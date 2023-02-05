#ifndef GMU_SDL_HANDLER_H
#define GMU_SDL_HANDLER_H

#include <string>
#include <cstdio>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <bitset>
#include "Bus.h"
#include <SDL.h>
#include <SDL_ttf.h>


// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int REGISTER_X_OFFSET = SCREEN_WIDTH - 240;
const int REGISTER_VALUE_OFFSET = 48;
const int REGISTER_PAIR_OFFSET = 64;
const int REGISTER_FLAG_OFFSET = 96;
const int MEMORY_BASE_OFFSET = 8;
const int MEMORY_ADDRESS_OFFSET = 48;
const int FONT_SIZE = 16;
const int LINE_OFFSET = 20;

// Forward declare zText class
class zText;
class SDL_Handler {
public:
    // Constructor
    SDL_Handler();
    // Destructor
    ~SDL_Handler();
    // Starts up SDL and creates window
    bool init(const std::string &title, const int wScreen, const int hScreen);
    // Frees media
    void close();
    // Event handler - Public for now
    SDL_Event eventHandler;
    // Checks to see if there are any events on queue
    int pollEvent();
    // Clear the screen
    void clearScreen();
    // Update screen
    void renderPresent();
    // Render all text to the window
    void renderText();
private:
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    // This vector contains pointers to all zText objects
    // Used for freeing all memory being used
    static std::vector<zText*> sTextList;
    friend class zText;
};

class zText{
public:
    // Initialize variables
    zText(SDL_Handler *curHandler, std::string text, int x, int y, std::string color, std::string fontType, int fontSize);
    zText(SDL_Handler *curHandler, int num, bool u16, int x, int y, std::string color, std::string fontType, int fontSize);
    zText(SDL_Handler *curHandler, int num, bool u16, bool binary, int x, int y, std::string color, std::string fontType, int fontSize);
    // Deallocate memory
    ~zText();
    // Update text
    void updateText(std::string nText);
    void updateText(int num, bool u16 = false, bool binary = false);
    // Creates image from font string - this should be a helper function
    bool generateTexture();
    // Deallocates texture
    void freeTexture();
    // Deallocates texture and font
    void free();
    // Renders texture at given point
    void render();
    void render(int x, int y, SDL_Rect *clip = nullptr, double angle = 0.0, SDL_Point *center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);
    // Get image dimensions
    int getWidth() const;
    int getHeight() const;
    // Get current string
    std::string getString();
    // Set color
    bool setColor(std::string color);
    // Set font
    bool setFont(std::string font);
    // Getter functions for X and Y coordinates
    int getX() const;
    int getY() const;

private:
    // Current SDL_Handler
    SDL_Handler *pHandler;
    // Image dimensions
    int tWidth;
    int tHeight;
    // Set the height and width
    void setDimensions(SDL_Surface *curSurface);
    // Text position
    int tX;
    int tY;
    // Set position
    bool setPosition(int x, int y);
    // Convert int to hex string
    std::string toHexString(int num, bool u16 = false, bool binary = false);

    // Current font
    TTF_Font *tFont;
    // Font size
    uint8_t tFontSize;
    // Font color
    SDL_Color tFontColor;

    // Current text as std::string
    std::string tCurText;
    // Text texture - needs to be initialized here, maybe want to separate the generate function from constructor
    SDL_Texture *tTexture = nullptr;
    // Available fonts - maybe make this a std::vector<std::pair<std::string, std::string>> ??
    // Also static as there is no need for duplicates here
    static std::map<std::string, std::string> sFontMap;
    // Color map
    static std::map<std::string, SDL_Color> sColorMap;
};

class zMemoryText {
public:
    zMemoryText(SDL_Handler* curHandler, const Bus &gb, int baseAddress, int x, int y, std::string color, std::string fontType, int fontSize);
    ~zMemoryText();


    // Over-write text with new values
    void update();
    // ^ Consider a re-write. Will need to change the constructor to not include text
    // that is not RAM values?? Or make 2 vectors for the first and last 8 bytes in the line??

    int getBaseNum() const;
    int getBaseX() const;
    int getBaseY() const;
private:
    int baseNum;
    int baseX;
    int baseY;
    const Bus *pGb;
    // Pointers to all text objects
    std::vector<zText*>addressLine = {};
};
#endif //GMU_SDL_HANDLER_H
