#ifndef GMU_SDL_HANDLER_H
#define GMU_SDL_HANDLER_H

#include <string>
#include <map>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>

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
    // Deallocate memory
    ~zText();
    // Update text
    bool updateText(std::string nText);
    // Creates image from font string - this should be a helper function
    bool generateTexture();
    // Deallocates texture
    void freeTexture();
    // Deallocates texture and font
    void free();
    // Renders texture at given point
    void render();
    void render(int x, int y, SDL_Rect *clip = nullptr, double angle = 0.0, SDL_Point *center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);
    // Load fonts list, may only be used in constructor???
    bool loadMedia();
    // Get image dimensions
    int getWidth();
    int getHeight();
    // Get current string
    std::string getString();
    // Set color
    bool setColor(std::string color);
    // Set font
    bool setFont(std::string font);
    // Getter functions for X and Y coordinates
    int getX();
    int getY();

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
    // Also static as there is no need for duplicates here, need to define in main file.
    static std::map<std::string, std::string> sFontMap;
    // Color map
    static std::map<std::string, SDL_Color> sColorMap;
};

#endif //GMU_SDL_HANDLER_H
