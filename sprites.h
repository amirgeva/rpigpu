#pragma once

#include "defs.h"

#define SPRITE_COUNT 160

namespace gpu
{
  constexpr uint32_t SPRITE_SIZE = 32;
  constexpr uint32_t SPRITE_AREA = (SPRITE_SIZE*SPRITE_SIZE);

  Color* get_sprite(uint32_t id);
  void   clear_sprite(uint16_t id);

}
