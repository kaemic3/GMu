#include "GMu.h"

namespace GMu {
    zViewport::zViewport() {
    }

    zViewport::~zViewport() {

    }

    void zTextViewport::GenerateFont(const std::string &font, int font_size) {
        // Grab the font directory from the font map
        auto it = s_font_map.find(font);
        if(it == s_font_map.end()) {
            printf("Error: Font not found.");
            return;
        }
        std::string font_dir = it->second;
        viewport_font = TTF_OpenFont(font_dir.c_str(), font_size);
        if(viewport_font == nullptr)
            printf("Unable to open font! SDL error: %s\n", TTF_GetError());
    }

    void zTextViewport::GenerateColor(const std::string &color) {
        // Grab the color from the color map
        auto it = s_color_map.find(color);
        if(it == s_color_map.end()) {
            printf("Error: Color not found.");
            return;
        }
        viewport_font_color = it->second;
    }

    void zTextViewport::GenerateTextTexture(zText *text) {
        SDL_Surface *text_surface = TTF_RenderText_Solid(viewport_font, text->t_string.c_str(), viewport_font_color);
        if (text_surface == nullptr) {
            printf("Unable to Render text surface! SDL error: %s\n", TTF_GetError());
            return;
        }
        // Set renderer viewport
        SDL_RenderSetViewport(p_window->p_renderer, &viewport_rect);
        // Create texture from the surface
        text->t_texture = SDL_CreateTextureFromSurface(p_window->p_renderer, text_surface);
        if(text->t_texture == nullptr) {
            printf("Unable to generate texture from rendered text! SDL error: %s\n", TTF_GetError());
            return;
        }
        // Set dimension members for zText object
        text->t_width = text_surface->w;
        text->t_height = text_surface->h;
        // Free the surface
        SDL_FreeSurface(text_surface);
    }

    void zTextViewport::RenderText(zText *text) {
        // Set the rendering space for the text texture
        SDL_Rect render_space = {text->t_x, text->t_y, text->t_width, text->t_height};
        // Render text
        SDL_RenderCopy(p_window->p_renderer, text->t_texture, nullptr, &render_space);
    }

    void zTextViewport::FreeTextTexture(zText *text) {
        // Free the texture if it exists
        if(text->t_texture) {
            SDL_DestroyTexture(text->t_texture);
            text->t_texture = nullptr;
            text->t_width = 0;
            text->t_height = 0;
        }
    }

// Helper functions
    std::string zTextViewport::ToLowerCase(const std::string &string) {
        std::string lower_case = string;
        for(char &c : lower_case)
            c = tolower(c);
        return lower_case;
    }
    std::string zTextViewport::ToHexString(int num, bool u16) {
        std::stringstream stream;
        if(u16) {
            stream << std::setfill('0') << std::setw(sizeof (uint16_t) * 2) << std::hex << num;
            return stream.str();
        }
        else {
            stream << std::setfill('0') << std::setw(sizeof (uint8_t) * 2) << std::hex << num;
            return stream.str();
        }
    }

    std::string zTextViewport::ToBinaryString(int num) {
        return std::bitset<8>(num).to_string();
    }


// Static member definitions
    std::map<std::string, std::string> zTextViewport::s_font_map = {
            {"amstrad cpc", "../Fonts/amstrad_cpc464.ttf"},
            {"fira code", "../Fonts/FiraCode.ttf"}
    };
    std::map<std::string, SDL_Color> zTextViewport::s_color_map = {
            { "purple", {73, 71, 134, 0xff} }
    };

    zDebugViewport::zDebugViewport(zMainWindow *window, Bus *bus) {
        p_window = window;
        p_bus = bus;
        viewport_rect.w = 460;
        viewport_rect.h = 144 * 4 + BORDER_OFFSET * 2;
        viewport_rect.x = 660;
        viewport_rect.y = 0;
        // Set default font
        GenerateFont("amstrad cpc");
        GenerateColor("purple");
        CreateText();
    }

    zDebugViewport::zDebugViewport(zWindow *window, Bus *bus, int width, int height, int x, int y) {
        p_window = window;
        p_bus = bus;
        viewport_rect.w = width;
        viewport_rect.h = height;
        viewport_rect.x = x;
        viewport_rect.y = y;
        CreateText();
    }

    zDebugViewport::~zDebugViewport() {
        // Free all textures
        FreeTextures();
        printf("All textures freed\n");
        TTF_CloseFont(viewport_font);
        printf("Font freed\n");
    }

    void zDebugViewport::Render() {
        SDL_RenderSetViewport(p_window->p_renderer, &viewport_rect);
        // Outline
        SDL_SetRenderDrawColor(p_window->p_renderer, 31, 41, 41, 0xff);
        SDL_Rect outline = {0, 0, viewport_rect.w, viewport_rect.h};
        SDL_RenderFillRect(p_window->p_renderer, &outline);
        // Background
        SDL_Rect background = {10, 10, outline.w-20, outline.h-20}; // h is -10 to make it look better
        SDL_SetRenderDrawColor(p_window->p_renderer, 196, 190, 187, 0xff);
        SDL_RenderFillRect(p_window->p_renderer, &background);
        // Render text
        for (auto t : all_text)
            RenderText(t);

    }

    void zDebugViewport::Update() {
        // Update mutable text
        for(auto t : mutable_text) {
            // Free old texture if it exists
            FreeTextTexture(t.first);
            // Check for 16-bit value
            if(t.first->t_u16)
                t.first->t_string = ToHexString(*((uint16_t *) t.second), true);\
        // Check for binary
            else if(t.first->t_binary)
                t.first->t_string = ToBinaryString(*((uint8_t *) t.second));
                // 8-bit hex
            else if(!t.first->t_binary && !t.first->t_u16)
                t.first->t_string = ToHexString(*(uint8_t *) t.second, false);
                // Error, should be impossible to get here
            else {
                printf("Error:void* in mutable_text vector for zText: %s is unknown.\n",t.first->t_string.c_str());
                return;
            }
            // Render text surface
            GenerateTextTexture(t.first);
        }
    }

    void zDebugViewport::Input(ViewportEvent event) {

    }

    void zDebugViewport::CreateText() {
        // A register
        a_reg = std::make_unique<zText>("A:", BORDER_OFFSET + FONT_SIZE, BORDER_OFFSET + FONT_SIZE, &viewport_font_color, viewport_font);
        all_text.push_back(a_reg.get());
        GenerateTextTexture(a_reg.get());

        a_value = std::make_unique<zText>(ToHexString(p_bus->cpu.a_reg, false), a_reg->t_x + (FONT_SIZE * 2), a_reg->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(a_value.get());
        mutable_text.emplace_back(a_value.get(), &p_bus->cpu.a_reg);
        GenerateTextTexture(a_value.get());

        // F register - Hex
        f_reg = std::make_unique<zText>("F:", a_value->t_x + (FONT_SIZE * 4), a_value->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(f_reg.get());
        GenerateTextTexture(f_reg.get());

        f_value = std::make_unique<zText>(ToHexString(p_bus->cpu.f_reg, false), f_reg->t_x + (FONT_SIZE * 2), f_reg->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(f_value.get());
        mutable_text.emplace_back(f_value.get(), &p_bus->cpu.f_reg);
        GenerateTextTexture(f_value.get());

        // B register
        b_reg = std::make_unique<zText>("B:", BORDER_OFFSET + FONT_SIZE, BORDER_OFFSET + (FONT_SIZE * 2) + FONT_SIZE, &viewport_font_color, viewport_font);
        all_text.push_back(b_reg.get());
        GenerateTextTexture(b_reg.get());

        b_value = std::make_unique<zText>(ToHexString(p_bus->cpu.b_reg, false), b_reg->t_x + (FONT_SIZE * 2), b_reg->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(b_value.get());
        mutable_text.emplace_back(b_value.get(), &p_bus->cpu.b_reg);
        GenerateTextTexture(b_value.get());

        // C register
        c_reg = std::make_unique<zText>("C:", b_value->t_x + (FONT_SIZE * 4), b_value->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(c_reg.get());
        GenerateTextTexture(c_reg.get());

        c_value = std::make_unique<zText>(ToHexString(p_bus->cpu.c_reg, false), c_reg->t_x + (FONT_SIZE * 2), c_reg->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(c_value.get());
        mutable_text.emplace_back(c_value.get(), &p_bus->cpu.c_reg);
        GenerateTextTexture(c_value.get());

        // D register
        d_reg = std::make_unique<zText>("D:", BORDER_OFFSET + FONT_SIZE, BORDER_OFFSET + (FONT_SIZE * 4) + FONT_SIZE, &viewport_font_color, viewport_font);
        all_text.push_back(d_reg.get());
        GenerateTextTexture(d_reg.get());

        d_value = std::make_unique<zText>(ToHexString(p_bus->cpu.d_reg, false), d_reg->t_x + (FONT_SIZE * 2), d_reg->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(d_value.get());
        mutable_text.emplace_back(d_value.get(), &p_bus->cpu.d_reg);
        GenerateTextTexture(d_value.get());

        // E register
        e_reg = std::make_unique<zText>("E:", d_value->t_x + (FONT_SIZE * 4), d_value->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(e_reg.get());
        GenerateTextTexture(e_reg.get());

        e_value = std::make_unique<zText>(ToHexString(p_bus->cpu.e_reg, false), e_reg ->t_x + (FONT_SIZE * 2), e_reg->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(e_value.get());
        mutable_text.emplace_back(e_value.get(), &p_bus->cpu.e_reg);
        GenerateTextTexture(e_value.get());

        // H register
        h_reg = std::make_unique<zText>("H:", BORDER_OFFSET + FONT_SIZE, BORDER_OFFSET + (FONT_SIZE * 6) + FONT_SIZE, &viewport_font_color, viewport_font);
        all_text.push_back(h_reg.get());
        GenerateTextTexture(h_reg.get());

        h_value = std::make_unique<zText>(ToHexString(p_bus->cpu.h_reg, false), h_reg->t_x + (FONT_SIZE * 2), h_reg->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(h_value.get());
        mutable_text.emplace_back(h_value.get(), &p_bus->cpu.h_reg);
        GenerateTextTexture(h_value.get());

        // L register
        l_reg = std::make_unique<zText>("L:", h_value->t_x + (FONT_SIZE * 4), h_value->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(l_reg.get());
        GenerateTextTexture(l_reg.get());

        l_value = std::make_unique<zText>(ToHexString(p_bus->cpu.l_reg, false), l_reg->t_x + (FONT_SIZE * 2), l_reg->t_y, &viewport_font_color, viewport_font);
        all_text.push_back(l_value.get());
        mutable_text.emplace_back(l_value.get(), &p_bus->cpu.l_reg);
        GenerateTextTexture(l_value.get());

        // Flags - Binary
        flags_prefix = std::make_unique<zText>("Flags:", BORDER_OFFSET + FONT_SIZE, BORDER_OFFSET + (FONT_SIZE * 8) + FONT_SIZE, &viewport_font_color, viewport_font);
        all_text.push_back(flags_prefix.get());
        GenerateTextTexture(flags_prefix.get());

        f_binary = std::make_unique<zText>(ToBinaryString(p_bus->cpu.f_reg), flags_prefix->t_x + (flags_prefix->t_string.size() * FONT_SIZE), flags_prefix->t_y, &viewport_font_color, viewport_font, false, true);
        all_text.push_back(f_binary.get());
        mutable_text.emplace_back(f_binary.get(), &p_bus->cpu.f_reg);
        GenerateTextTexture(f_binary.get());

        // Flags key
        flags_key = std::make_unique<zText>("ZNHC----", f_binary->t_x, f_binary->t_y + (FONT_SIZE), &viewport_font_color, viewport_font);
        all_text.push_back(flags_key.get());
        GenerateTextTexture(flags_key.get());

        // PC
        pc_text = std::make_unique<zText>("PC:", BORDER_OFFSET + FONT_SIZE, flags_key->t_y + (FONT_SIZE * 2), &viewport_font_color, viewport_font);
        all_text.push_back(pc_text.get());
        GenerateTextTexture(pc_text.get());

        pc_value = std::make_unique<zText>(ToHexString(p_bus->cpu.pc, true), pc_text->t_x + (FONT_SIZE * 3), pc_text->t_y, &viewport_font_color, viewport_font, true);
        all_text.push_back(pc_value.get());
        mutable_text.emplace_back(pc_value.get(), &p_bus->cpu.pc);
        GenerateTextTexture(pc_value.get());

        // SP
        sp_text = std::make_unique<zText>("SP:", BORDER_OFFSET + FONT_SIZE, pc_value->t_y + (FONT_SIZE * 2), &viewport_font_color, viewport_font);
        all_text.push_back(sp_text.get());
        GenerateTextTexture(sp_text.get());

        sp_value = std::make_unique<zText>(ToHexString(p_bus->cpu.sp, true), sp_text ->t_x + (FONT_SIZE * 3), sp_text->t_y, &viewport_font_color, viewport_font, true);
        all_text.push_back(sp_value.get());
        mutable_text.emplace_back(sp_value.get(), &p_bus->cpu.sp);

        // SP grouping

    }


    void zDebugViewport::FreeTextures() {
        for(auto t : all_text)
            FreeTextTexture(t);
    }

    zMemoryViewport::zMemoryViewport(zMainWindow *window, Bus *bus) {
        p_window = window;
        p_bus = bus;
        viewport_rect.x = 0;
        viewport_rect.y = 144 * 4 + 20;
        viewport_rect.w = SCREEN_WIDTH;
        viewport_rect.h = SCREEN_HEIGHT - 144 * 4 - 20;
        GenerateColor("purple");
        GenerateFont("amstrad cpc");
        CreateText();
    }

    zMemoryViewport::~zMemoryViewport() {
        FreeTextures();
        printf("Memory textures freed\n");
    }

    void zMemoryViewport::Render() {
        SDL_RenderSetViewport(p_window->p_renderer, &viewport_rect);
        // Outline
        SDL_SetRenderDrawColor(p_window->p_renderer, 31, 41, 41, 0xff);
        SDL_Rect outline = {0, 0, viewport_rect.w, viewport_rect.h};
        SDL_RenderFillRect(p_window->p_renderer, &outline);
        // Background
        SDL_Rect background = {10, 10, outline.w-20, outline.h-20};
        SDL_SetRenderDrawColor(p_window->p_renderer, 196, 190, 187, 0xff);
        SDL_RenderFillRect(p_window->p_renderer, &background);
        // Render text
        for(auto &t : memory_text_list) {
            RenderText(t.get());
        }

    }


    void zMemoryViewport::Update() {
        // Call create text to get updated memory
        FreeTextures();
        // Update textures
        UpdateText();
    }

    void zMemoryViewport::Input(ViewportEvent event) {
        switch(event) {
            case MemoryTranslateUp:
                TranslateYAxis(1);
                break;
            case MemoryTranslateDown:
                TranslateYAxis(-1);
                break;
            default:
                printf("Error in viewport input. No case found");
                break;
        }
    }

    void zMemoryViewport::UpdateBaseAddress(uint16_t address) {
        base_address = address;
    }

    void zMemoryViewport::TranslateYAxis(int sign) {
        if(sign == 1) {
            base_address -= 0x10;
        }

        else if(sign == -1) {
            base_address += 0x10;
        }

        else {
            printf("Error, no input");
        }


    }

    void zMemoryViewport::CreateText() {
        // Check if the base address is > 0x2000 (8KiB)
        if(base_address > 0x2000 - 0x110)
            base_address = 0x2000 - 0x110;
        // Return if base address is not divisible by 0x10
        if(base_address % 0x10 != 0)
            return;
        int line_number = 0;
        // Generate 16 lines of ram text
        for(size_t i = base_address; i < base_address + 0x110 ; i += 0x10) {
            std::string temp;
            temp = CreateString(i);
            memory_text_list.emplace_back(std::make_unique<zText>(temp, BORDER_OFFSET + FONT_SIZE, BORDER_OFFSET + ( FONT_SIZE  * line_number) + FONT_SIZE / 2 , &viewport_font_color, viewport_font));
            line_number++;
        }
        // Generate Textures
        for(auto &t : memory_text_list)
            GenerateTextTexture(t.get());
    }

    std::string zMemoryViewport::CreateString(uint16_t address) {
        std::stringstream stream;
        stream << "$" << ToHexString(address, true) << ": ";
        for (auto i = 0; i < 0x10; ++i) {
            stream << ToHexString(p_bus->wram[address + i], false) << " ";
        }
        return stream.str();
    }
    // Currently broken, will need to reimplement when the memory map
    // has been implemented properly
    void zMemoryViewport::UpdateText() {
        // Check if the base address is > 0x2000 (8KiB)
        if(base_address > (0x2000 - 0x110))
            base_address = 0x2000 - 0x110;
        // Return if the base address is divisible by 0x10
        if(base_address % 0x10)
            return;
        int index = 0;
        for(size_t i  = base_address; i < base_address + 0x110; i += 0x10) {
            std::string temp;
            temp = CreateString(i);
            memory_text_list[index]->t_string = temp;
            GenerateTextTexture(memory_text_list[index].get());
            index++;
        }
    }

    void zMemoryViewport::FreeTextures() {
        for(auto &t : memory_text_list)
            FreeTextTexture(t.get());
    }

    zPPUViewport::zPPUViewport(zMainWindow *window, Bus *bus) {
        p_window = window;
        p_bus = bus;
        viewport_rect.x = 0;
        viewport_rect.y = 0;
        viewport_rect.w = 160 * 4 + 20;
        viewport_rect.h = 144 * 4 + 20;
    }

    zPPUViewport::~zPPUViewport() {

    }

    void zPPUViewport::Render() {
        SDL_RenderSetViewport(p_window->p_renderer, &viewport_rect);
        // Outline
        SDL_SetRenderDrawColor(p_window->p_renderer, 31, 41, 41, 0xff);
        SDL_Rect outline = {0, 0, viewport_rect.w, viewport_rect.h};
        SDL_RenderFillRect(p_window->p_renderer, &outline);
        // Background
        SDL_Rect background = {10, 10, outline.w-20, outline.h-20};
        SDL_SetRenderDrawColor(p_window->p_renderer, 139, 208, 125, 0xff);
        SDL_RenderFillRect(p_window->p_renderer, &background);

    }

    void zPPUViewport::Update() {

    }

    void zPPUViewport::Input(ViewportEvent event) {

    }
}
