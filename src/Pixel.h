
#ifndef GMU_PIXEL_H
#define GMU_PIXEL_H

#include <cstdint>
// Data the FIFO will need
struct Pixel {
    uint8_t color = 0;          // Colors on the GB are 2 bit
    uint8_t palette = 0;        // Palette maps the colors to a color index that is pre-determined
    uint8_t bg_priority = 0;    // Only for sprites
};
#endif //GMU_PIXEL_H
