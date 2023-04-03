#ifndef VIEWPORT_TEST_GMU_H
#define VIEWPORT_TEST_GMU_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <bitset>
#include <iomanip>
#include <memory>
#include "SM83.h"
#include "Bus.h"

namespace GMu {
    class zWindow;
    class zViewport;
    class zText;
    // Constants
    const int SCREEN_WIDTH = 1120;
    const int SCREEN_HEIGHT = 900;
    const int BORDER_OFFSET = 10;
    const int FONT_SIZE = 16;

    // Vector containing a pointer to all zWindow objects
    static std::vector<zWindow*> window_list;
    // Enum for all events that are handled on a viewport level
    enum ViewportEvent {MemoryTranslateUp, MemoryTranslateDown};
    // Define pointers for all windows
    static zWindow *main_window;
    static zWindow *debug_window;
    static zWindow *settings;
    static zWindow *rom_select;
    // Bus object for emulator
    static Bus gb;

    // Classes

    // Forward declaration of zViewport to avoid circular includes
    class zWindow {
    public:
        // Constructor and destructor
        zWindow();
        virtual ~zWindow();
        // Startup SDL and create a window with renderer
        virtual bool Init(const std::string &title, int screen_w, int screen_h) = 0;
        // Destroy window and its contents
        virtual void Free() = 0;
        // Event handler for window events
        virtual void HandleWindowEvent(SDL_Event &e) = 0;
        // Event handler for Viewport events
        virtual void HandleViewportEvent(GMu::ViewportEvent event) = 0;
        // Focuses window
        virtual void Focus() = 0;
        // Render window background
        virtual void RenderBG() = 0;
        // Render the viewports
        virtual void RenderViewports() = 0;
        // Show the contents of the window
        virtual void ShowRenderer() = 0;
        // Update the viewports
        virtual void UpdateViewports() = 0;
        // Window dimensions
        virtual int GetHeight() const = 0;
        virtual int GetWidth() const = 0;
        // Window Focus
        virtual bool HasMouseFocus() const = 0;
        virtual bool hasKeyboardFocus() const = 0;
        virtual bool IsMinimized() const = 0;
        virtual bool IsShown() const = 0;
        virtual bool IsFullscreen() const = 0;

        // members
        SDL_Window *p_window = nullptr;
        SDL_Renderer *p_renderer = nullptr;
        std::string w_title;
        int window_id;
        // Dimensions
        int w_width;
        int w_height;
        // Focus
        bool mouse_focus;
        bool keyboard_focus;
        bool fullscreen;
        bool minimized;
        bool shown;
        Bus* p_bus;

    };

    class zMainWindow : public zWindow {
    public:
        explicit zMainWindow(Bus *bus);
        ~zMainWindow() override;
        // Startup SDL and create a window with renderer
        bool Init(const std::string &title, int screen_w, int screen_h) override;
        // Destroy window and its contents
        void Free() override;
        // Event handler for window events
        void HandleWindowEvent(SDL_Event &e) override;
        // Event handler for Viewport events
        void HandleViewportEvent(GMu::ViewportEvent event) override;
        // Focuses window
        void Focus() override;
        // Render window background
        void RenderBG() override;
        // Update viewports
        void RenderViewports() override;
        // Show the contents of the window
        void ShowRenderer() override;
        // Update the viewports
        void UpdateViewports() override;
        // Window dimensions
        int GetHeight() const override;
        int GetWidth() const override;
        // Window Focus
        bool HasMouseFocus() const override;
        bool hasKeyboardFocus() const override;
        bool IsMinimized() const override;
        bool IsShown() const override;
        bool IsFullscreen() const override;

    private:
        // Call the viewport constructors
        void initializeViewports();
        // Viewport objects
        std::unique_ptr<zViewport> debug;
        std::unique_ptr<zViewport> memory;
        std::unique_ptr<zViewport> ppu;

    };


    class zViewport {
    public:
        zViewport();
        virtual ~zViewport();
        virtual void Render() = 0;
        virtual void Update() = 0;
        virtual void Input(ViewportEvent event) = 0;
        GMu::zWindow* p_window = nullptr;
        SDL_Rect viewport_rect;
        Bus *p_bus = nullptr;
    };

    // Interface for text based viewports
    class zTextViewport : public zViewport {
    public:
        // Generate TTF_Font from string
        void GenerateFont(const std::string &font, int font_size = 16);
        // Generate SDL_Color from string
        void GenerateColor(const std::string &color);
        // Generate SDL_Surface from zText
        void GenerateTextTexture(zText *text);
        // Render all text to the viewport
        void RenderText(zText *text);
        // Free a zText SDL_Texture
        void FreeTextTexture(zText* text);

        // Helper functions
        // Returns a lowercase string
        std::string ToLowerCase(const std::string &string);

        // Converts int to string in hex notation.
        // Parameters:
        // - num: Number to convert
        // - u16: True for 16-bit numbers, false for 8-bit
        static std::string ToHexString(int num, bool u16);
        // Converts int to string in binary notation
        // Parameters:
        // - num: Number to convert
        static std::string ToBinaryString(int num);

        // Static members
        static std::map<std::string, std::string> s_font_map;
        static std::map<std::string, SDL_Color> s_color_map;

        // Viewport font
        TTF_Font *viewport_font = nullptr;
        // Viewport Color
        SDL_Color viewport_font_color;

    };

    class zDebugViewport : public zTextViewport {
    public:
        zDebugViewport(GMu::zMainWindow *window, Bus *bus);
        zDebugViewport(GMu::zWindow *window, Bus *bus, int width, int height, int x, int y);
        ~zDebugViewport() override;
        // Render the contents of the viewport. NOTE: This function cannot Render a copy of the viewport SDL_RECT as it takes
        // a pointer to the rect.
        // Also note: When drawing anything to the viewport, the X,Y grid will be set according to the viewport
        // location and size.
        void Render() override;
        // Update register value text
        void Update() override;
        // Not used
        void Input(ViewportEvent event) override;
        // Initializes zText objects and generates textures for them
        void CreateText();

    private:
        // All text in viewport
        std::vector<zText*> all_text;
        // Stores text that changes during runtime, and the associated pointer to the type it will need to reference.
        std::vector<std::pair<zText*,const void*>> mutable_text;

        // Free's all texture memory
        void FreeTextures();
        // zText objects
        // Non mutable
        std::unique_ptr<zText> a_reg;
        std::unique_ptr<zText> f_reg;
        std::unique_ptr<zText> b_reg;
        std::unique_ptr<zText> c_reg;
        std::unique_ptr<zText> d_reg;
        std::unique_ptr<zText> e_reg;
        std::unique_ptr<zText> h_reg;
        std::unique_ptr<zText> l_reg;
        std::unique_ptr<zText> flags_prefix;
        std::unique_ptr<zText> flags_key;
        std::unique_ptr<zText> pc_text;
        std::unique_ptr<zText> sp_text;

        // Mutable
        std::unique_ptr<zText> a_value;
        std::unique_ptr<zText> f_value;
        std::unique_ptr<zText> b_value;
        std::unique_ptr<zText> c_value;
        std::unique_ptr<zText> d_value;
        std::unique_ptr<zText> e_value;
        std::unique_ptr<zText> h_value;
        std::unique_ptr<zText> l_value;
        std::unique_ptr<zText> f_binary;
        std::unique_ptr<zText> pc_value;
        std::unique_ptr<zText> sp_value;
    };

    class zMemoryViewport : public zTextViewport {
    public:
        zMemoryViewport(GMu::zMainWindow *window, Bus *bus);
        ~zMemoryViewport() override;

        void Render() override;
        void Update() override;
        void Input(ViewportEvent event) override;
        // Update the base address for our viewport to use
        void UpdateBaseAddress(uint16_t address);
        // Change the Y position of the text
        void TranslateYAxis(int sign);
        // Initializes zText objects and generates textures for them
        void CreateText();
        // Generate the string used to create out zText objects
        std::string CreateString(uint16_t address);
        // Update Texture
        void UpdateText();
        // Free's all texture memory
        void FreeTextures();
    private:
        // This address is the start for our loop to grab the memory values at.
        uint16_t base_address = 0x0000;
        // Store all text for this viewport
        std::vector<std::unique_ptr<zText>> memory_text_list;
    };

    // Menu bar class
    class zMenuBar : public zTextViewport {
        zMenuBar() = default;
        ~zMenuBar() = default;

        void Render() override;
        void Update() override;
        void Input(ViewportEvent event) override;
    };

    class zPPUViewport : public zViewport {
    public:
        zPPUViewport(GMu::zMainWindow *window, Bus *bus);
        ~zPPUViewport() override;

        void Render() override;
        void Update() override;
        void Input(ViewportEvent event) override;
    };


    class zText {
        friend class zTextViewport;
        friend class zDebugViewport;
        friend class zMemoryViewport;
    public:
        zText(const std::string &string, int x, int y, SDL_Color *color, TTF_Font *font, bool p16Bit = false, bool binary = false, int fontSize = 16) :
                t_string(string), t_x(x), t_y(y), t_color(color), t_font(font), t_u16(p16Bit), t_binary(binary), t_font_size(fontSize), t_width(0), t_height(0)
        {  }
        ~zText() = default;
    private:
        int t_x;
        int t_y;
        int t_width;
        int t_height;
        int t_font_size;
        std::string t_string;
        bool t_u16;
        bool t_binary;
        SDL_Color* t_color = nullptr;
        SDL_Texture *t_texture = nullptr;
        TTF_Font *t_font = nullptr;

    };

    // Functions

    // Used to initialize SDL
    static bool Init() {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) {
            printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
            return false;
        }
        // Initialize SDL_ttf
        if(TTF_Init() == -1)
            printf("Failed to initialize SDL_ttf! SDL error: %s\n", TTF_GetError());
        // Set texture filtering to linear
        if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
            printf("Warning: Linear texture filtering not enabled!");
        // Create window
        if(!GMu::window_list[0]->Init("Main", SCREEN_WIDTH, SCREEN_HEIGHT)) {
            printf("Could not create window 0!");
            return false;
        }
        return true;
    }

    // Close SDL and Free memory - need to change how windows do not get destroyed when they are closed
    static void Close() {
        // Destroy windows
        for(auto i : GMu::window_list) {
            i->Free();
            delete i;
        }
        // Quit SDL
        SDL_Quit();
        TTF_Quit();
    }

    // Load a rom file into RAM for the emulator.

    // Does not support memory bank switching (not sure if this is where that code will
    // be at).
    static void LoadBinaryFile(const std::string &file_path) {
        // Open the file with file in mode, binary mode, and set the pointer to the end of the file
        std::ifstream file_stream(file_path, std::ios::in | std::ios::binary | std::ios::ate);
        // Make sure the file_stream opens before doing any operations
        if(!file_stream.is_open()) {
            printf("Error, could not open file: %s\n", file_path.c_str());
            return;
        }
        printf("File %s has been opened.\n", file_path.c_str());
        // Create a mem_block for the data
        char *mem_block;
        // Create a local variable for the size of the file, and get the size
        std::streampos size = file_stream.tellg();
        // Allocate mem_block pointer
        mem_block = new char[size];
        // Set the file pointer to the beginning of the file
        file_stream.seekg(0, std::ios::beg);
        // Read the ROM and store in mem_block
        file_stream.read(mem_block, size);
        // Close the file
        file_stream.close();
        printf("File %s has been closed.\n", file_path.c_str());
        // Copy the contents of mem_block into gb.ram
        for (int i = 0; i < size; ++i) {
            gb.ram[i] = mem_block[i];
        }
        // Delete mem_block
    }
}
#endif //VIEWPORT_TEST_GMU_H
