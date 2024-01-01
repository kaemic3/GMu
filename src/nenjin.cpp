#include "nenjin.h"
#include "nenjin_render.cpp"

// TODO(kaelan): Invert drawing to back buffer, so our coordinate system lines up with bottom left??
extern "C"
NENJIN_UPDATE_AND_RENDER(NenjinUpdateAndRender) {
    // nenjin_state acts as the structure for the permanent storage.
    Assert((sizeof(nenjin_state) <= memory->permanent_storage_size));
    nenjin_state *emulator_state = (nenjin_state *)memory->permanent_storage;
    if(!memory->is_initialized)
    {
        emulator_state->test_txt = DEBUGLoadBMP(thread, memory->DEBUGPlatformReadEntireFile, "test_text.bmp");
    }
    //Clear screen to black
    ClearBackBufferToBlack(buffer);
    // Draw test text to the center of the window.
    DrawBitmap(buffer, &emulator_state->test_txt, 1280.0f/2.0f, 720.0f/2.0f, 319, 100);
    // Draw a red square at 25.0, 50.0.
    DrawRectangle(buffer, 25.0f, 50.0f, 50.0f, 75.0f, 1.0f, 0.0f, 0.0f);
}