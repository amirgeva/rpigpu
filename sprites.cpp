#include "sprites.h"

namespace gpu
{
    constexpr uint32_t SPRITE_BUFFER_SIZE = uint32_t(SPRITE_AREA) * SPRITE_COUNT;
    static Color* SpriteData = (Color*)0x10000;

    Color* get_sprite(uint32_t id)
    {
        if (id >= SPRITE_COUNT) return nullptr;
        return &SpriteData[id * SPRITE_AREA];
    }

    void clear_sprite(uint16_t id)
    {
        Color* ptr = get_sprite(id);
        if (ptr)
            for (uint16_t i = 0; i < SPRITE_AREA; ++i)
                *ptr++ = 0;
    }

} // namespace gpu