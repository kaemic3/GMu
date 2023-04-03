#include "GMu.h"
namespace GMu {
    zWindow::zWindow() {

    }
    zWindow::~zWindow() {

    }
    zMainWindow::zMainWindow(Bus *bus) {
        p_window = nullptr;
        p_renderer = nullptr;
        p_bus = bus;
        mouse_focus = false;
        keyboard_focus = false;
        fullscreen = false;
        minimized = false;
        shown = false;
        window_id = -1;
        w_width = 0;
        w_height = 0;
    }

    zMainWindow::~zMainWindow() {
        printf("Window deleted\n");
    }

    bool zMainWindow::Init(const std::string &title, int screen_w, int screen_h) {
        // Create window
        p_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        // Check if the window was created
        if(p_window == nullptr) {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            return false;
        }
        w_title = title;
        keyboard_focus = true;
        mouse_focus = true;
        w_width = SCREEN_WIDTH;
        w_height = SCREEN_HEIGHT;
        // Create renderer
        p_renderer = SDL_CreateRenderer(p_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if(p_renderer == nullptr) {
            printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(p_window);
            p_window = nullptr;
            return false;
        }
        // Initialize renderer color
        SDL_SetRenderDrawColor(p_renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderClear(p_renderer);
        // Grab window identifier
        window_id = SDL_GetWindowID(p_window);
        // Flag as open
        shown = true;
        // Initialize viewports
        initializeViewports();
        return true;
    }

    void zMainWindow::HandleWindowEvent(SDL_Event &e) {
        // Check for event
        if(e.type == SDL_WINDOWEVENT && e.window.windowID == window_id) {
            // Update the caption flag
            bool updateCaption = false;
            switch(e.window.event) {
                // Window appeared
                case SDL_WINDOWEVENT_SHOWN:
                    shown = true;
                    break;
                    // Window disappeared
                case SDL_WINDOWEVENT_HIDDEN:
                    shown = false;
                    break;
                    // Get new dimensions and repaint
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    w_width = e.window.data1;
                    w_height = e.window.data2;
                    SDL_RenderPresent(p_renderer);
                    break;
                    // Repaint on expose
                case SDL_WINDOWEVENT_EXPOSED:
                    SDL_RenderPresent(p_renderer);
                    break;
                    // Mouse enter
                case SDL_WINDOWEVENT_ENTER:
                    mouse_focus = true;
                    updateCaption = true;
                    break;
                    // Mouse exit
                case SDL_WINDOWEVENT_LEAVE:
                    mouse_focus = false;
                    updateCaption = true;
                    break;
                    // Keyboard Focus gained
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    keyboard_focus = true;
                    updateCaption = true;
                    break;
                    // Keyboard Focus lost
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    keyboard_focus = false;
                    updateCaption = true;
                    break;
                    // Window minimized
                case SDL_WINDOWEVENT_MINIMIZED:
                    minimized = true;
                    break;
                    // Window maximized
                case SDL_WINDOWEVENT_MAXIMIZED:
                    minimized = false;
                    break;
                    // Window restored
                case SDL_WINDOWEVENT_RESTORED:
                    minimized = false;
                    break;
                    // Hide on Close
                case SDL_WINDOWEVENT_CLOSE:
                    SDL_HideWindow(p_window);
                    break;
            }
            // Update the window caption with new data
            if(updateCaption) {
                std::stringstream caption;
                caption << w_title << " ID: " << window_id << " MouseFocus: " << ((mouse_focus) ? "On" : "Off") << " Keyboard Focus: " << ((keyboard_focus) ? "On" : "Off");
                SDL_SetWindowTitle(p_window, caption.str().c_str());
            }
        }
    }

    void zMainWindow::HandleViewportEvent(GMu::ViewportEvent event) {
        // Need to be able to check the current viewport focus
        switch(event) {
            case MemoryTranslateUp:
                memory->Input(event);
                break;
            case MemoryTranslateDown:
                memory->Input(event);
                break;
        }
    }

    void zMainWindow::Focus() {
        // Restore window if needed
        if(!shown)
            SDL_ShowWindow(p_window);
        // Move window forward
        SDL_RaiseWindow(p_window);
    }

    void zMainWindow::RenderBG() {
        // Return if the window is minimized
        if(minimized)
            return;
        // Clear screen
        SDL_SetRenderDrawColor(p_renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderClear(p_renderer);
    }

    void zMainWindow::RenderViewports() {
        debug->Render();
        memory->Render();
        ppu->Render();
    }

    void zMainWindow::ShowRenderer() {
        SDL_RenderPresent(p_renderer);
    }

    void zMainWindow::UpdateViewports() {
        debug->Update();
        memory->Update();
        ppu->Update();
    }

    int zMainWindow::GetHeight() const {
        return w_height;
    }

    int zMainWindow::GetWidth() const {
        return w_width;
    }

    bool zMainWindow::HasMouseFocus() const {
        return mouse_focus;
    }

    bool zMainWindow::hasKeyboardFocus() const {
        return keyboard_focus;
    }

    bool zMainWindow::IsMinimized() const {
        return minimized;
    }


    bool zMainWindow::IsShown() const {
        return shown;
    }

    bool zMainWindow::IsFullscreen() const {
        return fullscreen;
    }

    void zMainWindow::Free() {
        if(p_window != nullptr)
            SDL_DestroyWindow(p_window);
        mouse_focus = false;
        keyboard_focus = false;
        w_width = 0;
        w_height = 0;
    }

    void zMainWindow::initializeViewports() {
        debug = std::make_unique<zDebugViewport>(this, p_bus);
        memory = std::make_unique<zMemoryViewport>(this, p_bus);
        ppu = std::make_unique<zPPUViewport>(this, p_bus);

    }
}