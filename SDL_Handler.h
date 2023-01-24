#ifndef GMU_SDL_HANDLER_H
#define GMU_SDL_HANDLER_H

#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

class zText{
    // Initialize variables
    zText();
    // Deallocate memory
    ~zText();
    // Loads image at specified path
    bool loadFromFiles(const std::string &path);
    // Creates image from font string
    bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
    // Deallocates texture
    void free();
    // Set color modulation
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    // Set blending
    void setBlendMode(SDL_BlendMode blending);
    // Set alpha modulation
    void setAlpha(uint8_t alpha);
    // Renders texture at given point
    void render(int x, int y, SDL_Rect *clip = nullptr, double angle = 0.0, SDL_Point *center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);
    // Get image dimensions
    int getWidth();
    int getHeight();
    // Actual hardware texture
    SDL_Texture *mTexture;
    // Image dimensions
    int mWidth;
    int mHeight;
};

class SDL_Handler {
    // Need access to zText object's private members
    friend class zText;
public:
    // Constructor
    SDL_Handler();
    // Destructor
    ~SDL_Handler();

private:
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    TTF_Font *pFont;
    zText textTexture;
    // Starts up SDL and creates window
    bool init();
    // Loads media
    bool loadMedia();
    // Frees media
    void close();

};



#endif //GMU_SDL_HANDLER_H
