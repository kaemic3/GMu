
#ifndef GMU_PIXEL_H
#define GMU_PIXEL_H

#include <cstdint>
// Data the FIFO will need
struct Pixel_BG {
    uint8_t color = 0;          // Colors on the GB are 2 bit
    uint8_t palette = 0;        // Palette maps the colors to a color index that is pre-determined
};

struct Pixel_FG {
    uint8_t color = 0;
    uint8_t palette = 0;
    uint8_t bg_priority = 0;
    uint8_t x_pos = 0;
};
#endif //GMU_PIXEL_H
