#include "font.h"
#include "gpu.h"
#include "uart.h"

namespace gpu 
{

    static const uint8_t* get_char_data(uint8_t c)
    {
      static const uint8_t data[] = {
        #include "font.inl"
      };
      //static const int n = sizeof(data) / m_CharHeight;
      //constexpr int n = sizeof(data) / CHAR_HEIGHT;
      //if (c >= n) return nullptr;
      return &data[c * CHAR_HEIGHT];
    }

    void draw_str(const uint8_t* text, uint8_t n,
                  uint16_t& x, uint16_t& y,
                  Color fg, Color bg, bool progress)
    {
      uint16_t ax=x,ay=y;
      for (uint8_t i=0;i<n;++i)
        draw(text[i], x, y, fg, bg, true);
      if (!progress)
      {
        uart_println("Resetting Cursor");
        x=ax;
        y=ay;
      }
    }

    void draw(uint8_t c, uint16_t& x, uint16_t& y, 
              Color fg, Color bg, bool progress)
    {
      const uint8_t* ptr = get_char_data(c);
      if (c == 10)
      {
        new_line(x, y);
        return;
      }
      if (ptr)
      {
        for (uint16_t i = 0; i < CHAR_HEIGHT; ++i, ++ptr)
        {
          Color* dst = get_frame_row(y+i) + x;
          for (uint16_t j = 0; j < CHAR_WIDTH; ++j)
            dst[CHAR_WIDTH - j - 1] = (*ptr & (1 << j)) ? fg : bg;
        }
      }
      if (progress)
        move_cursor_forward(x, y);
    }

    void new_line(uint16_t& x, uint16_t& y)
    {
      x = 0;
      y += CHAR_HEIGHT;
      if (y >= SCREEN_HEIGHT)
      {
        frame_scroll(CHAR_HEIGHT,0);
        y -= CHAR_HEIGHT;
      }
    }

    void move_cursor_forward(uint16_t& x, uint16_t& y)
    {
      x += CHAR_WIDTH;
      if (x >= SCREEN_WIDTH)
      {
        new_line(x, y);
      }
    }







} // namespace gpu 
