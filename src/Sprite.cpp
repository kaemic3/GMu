
#include "Sprite.h"

// Overload < operator for sorting the list
bool Sprite::operator < (const Sprite &s) const {
    // The sprite with the lower X pos will go first
    if (x_pos < s.x_pos) {
        return true;
    }
    // If the X pos is the same, store the sprite with the lower sprite id first
    if (x_pos == s.x_pos) {
        if (sprite_id < s.sprite_id)
            return true;
        else
            return false;
    }
    return false;
}
