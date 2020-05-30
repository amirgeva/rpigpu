#pragma once

#include "defs.h"

namespace gpu {

  constexpr uint32_t CHAR_WIDTH=8;
  constexpr uint32_t CHAR_HEIGHT=16;

  void draw_str(const uint8_t* text, uint8_t n, 
                uint16_t& x, uint16_t& y, 
                Color fg, Color bg, bool progress);
  void draw(uint8_t c, uint16_t& x, uint16_t& y,
            Color fg, Color bg, bool progress);
  void new_line(uint16_t& x, uint16_t& y);
  void move_cursor_forward(uint16_t& x, uint16_t& y);

}
