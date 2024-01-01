#include "nenjin.h"
extern "C"
NENJIN_UPDATE_AND_RENDER(NenjinUpdateAndRender) {
    // nenjin_state acts as the structure for the permanent storage.
    Assert((sizeof(nenjin_state) <= memory->permanent_storage_size));
    nenjin_state *emulator_state = (nenjin_state *)memory->permanent_storage;
    if(!memory->is_initialized)
    {

    }
    u32 *pixels = (u32 *)buffer->memory;
    u32 color = 0xff00ffff;
    for(u32 y = 0; y < (u32)buffer->height; ++y)
    {
        for(u32 x = 0; x < (u32)buffer->width; ++x)
        {
            *pixels++ = color;
        }
    }
}