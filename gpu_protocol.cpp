#include "gpu_protocol.h"
#include "uart.h"
#include "gpu.h"

namespace gpu {

  template<typename T>
  inline const T& Max(const T& a, const T& b)
  {
    return (a>b?a:b);
  }

  template<typename T>
  inline const T& Min(const T& a, const T& b)
  {
    return (a>b?b:a);
  }


  Protocol prot;
  static bool flip_screen;
  static bool cursor_on;
  static uint16_t cursor_x,cursor_y;

  void Protocol::init()
  {
    flip_screen=false;
    cursor_on=false;
    cursor_x=0;
    cursor_y=0;
    m_CursorStack.init();
    m_Pos=0;
    m_BytesLeft=0;
    m_Blink=false;
    m_Transparency=false;
    m_CursorX=0;
    m_CursorY=0;
    m_BGColor=0;
    m_FGColor=0xFFFF;
    m_Transparent=(1 << 5);
    for(int i=0;i<64;++i)
    {
      m_Handlers[i]=nullptr;
      m_CommandSizes[i]=1;
    }

    #define HANDLER(x) m_Handlers[CMD_##x]=&Protocol::Handle##x
    HANDLER(NOP);
    HANDLER(CLS);
    HANDLER(FLIP);
    HANDLER(TEXT_NEWLINE);
    HANDLER(PIXEL_CURSOR);
    HANDLER(TEXT_CURSOR);
    HANDLER(FG_COLOR);
    HANDLER(BG_COLOR);
    HANDLER(PUSH_CURSOR);
    HANDLER(POP_CURSOR);
    HANDLER(BLINK_CURSOR);
    HANDLER(FILL_RECT);
    HANDLER(HORZ_LINE);
    HANDLER(VERT_LINE);
    HANDLER(HORZ_PIXELS);
    HANDLER(TEXT);
    HANDLER(SET_SPRITE);
    HANDLER(DRAW_SPRITE);
    HANDLER(TRANSPARENT_COLOR);
    #undef HANDLER

    m_CommandSizes[CMD_NOP] = sizeof(Command);
    m_CommandSizes[CMD_CLS] = sizeof(Command_CLS);
    m_CommandSizes[CMD_FLIP] = sizeof(Command_Flip);
    m_CommandSizes[CMD_TEXT_NEWLINE] = sizeof(Command_NewLine);
    m_CommandSizes[CMD_PUSH_CURSOR] = sizeof(Command_PushCursor);
    m_CommandSizes[CMD_POP_CURSOR] = sizeof(Command_PopCursor);
    m_CommandSizes[CMD_PIXEL_CURSOR] = sizeof(Command_PixelCursor);
    m_CommandSizes[CMD_TEXT_CURSOR] = sizeof(Command_TextCursor);
    m_CommandSizes[CMD_BLINK_CURSOR] = sizeof(Command_BlinkCursor);
    m_CommandSizes[CMD_FILL_RECT] = sizeof(Command_FillRect);
    m_CommandSizes[CMD_HORZ_LINE] = sizeof(Command_HorzLine);
    m_CommandSizes[CMD_VERT_LINE] = sizeof(Command_VertLine);
    m_CommandSizes[CMD_HORZ_PIXELS] = sizeof(Command_HorzPixels);
    m_CommandSizes[CMD_TEXT] = sizeof(Command_Text);
    m_CommandSizes[CMD_FG_COLOR] = sizeof(Command_FGColor);
    m_CommandSizes[CMD_BG_COLOR] = sizeof(Command_BGColor);
    m_CommandSizes[CMD_SET_SPRITE] = sizeof(Command_SetSprite);
    m_CommandSizes[CMD_DRAW_SPRITE] = sizeof(Command_DrawSprite);
    m_CommandSizes[CMD_TRANSPARENT_COLOR] = sizeof(Command_TransparentColor);
  }


  void Protocol::XorRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
  {
    if ((x+w)>SCREEN_WIDTH || (y+h)>SCREEN_HEIGHT) return;
    for(int i=0;i<h;++i)
    {
      Color* row = get_frame_row(y+i) + x;
      for(int j=0;j<w;++j)
      {
        row[j] = ~row[j];
      }
    }
  }

  void Protocol::draw_cursor()
  {
    XorRect(cursor_x, cursor_y, 8, 16);
  }

  void Protocol::erase_cursor()
  {
    if (!m_Blink)
      cursor_on=false;
    else
    {
      if (cursor_on)
      {
        cursor_on=false;
        draw_cursor();
      }
    }
  }

  uint16_t Protocol::get_header_len()
  {
    if (Header.cmd.opcode>=64) return 1;
    return m_CommandSizes[Header.cmd.opcode];
  }

  uint16_t Protocol::check_variable_len()
  {
    switch (Header.cmd.opcode)
    {
    case CMD_HORZ_PIXELS: return Header.horz_pixels.n + sizeof(Command_HorzPixels) - m_Pos;
    case CMD_TEXT:        return Header.text.n + sizeof(Command_Text) - m_Pos;
    }
    return 0;
  }

  bool Protocol::process_command()
  {
    if (Header.cmd.opcode>=64)
    {
      return false;
    }
    Handler h = m_Handlers[Header.cmd.opcode];
    if (!h)
    {
      return false;
    }
    erase_cursor();
    (this->*h)();
    return true;
  }

  void Protocol::loop()
  {
    if (m_Blink)
    {
      static uint32_t start = 0;
      uint32_t cur = micros();
      if (start == 0) start = cur;
      if ((cur - start) > 500000)
      {
        //const char* text = "\001";
        cursor_on = !cursor_on;
        cursor_x = m_CursorX;
        cursor_y = m_CursorY;
        draw_cursor();
        start = cur;
        flip_screen = true;
      }
    }
  }

  void Protocol::add_byte(uint8_t b)
  {
    if (m_Pos == 0)
    {
      Header.cmd.opcode = b;
      m_BytesLeft = get_header_len() - 1;
      ++m_Pos;
    }
    else
    {
      uint8_t* ptr = &Header.cmd.opcode;
      ptr[m_Pos++] = b;
      ptr[m_Pos] = 0;
      --m_BytesLeft;
    }
    if (m_BytesLeft == 0) m_BytesLeft = check_variable_len();
    if (m_BytesLeft == 0)
    {
      process_command();
      m_Pos = 0;
    }
  }



  void Protocol::HandleNOP()
  {
    /*
    uart_print("Cursor: ");
    uart_print_hex_dword(m_CursorX);
    uart_print(" ");
    uart_print_hex_dword(m_CursorY);
    uart_println(" ");
    */
  }

  void Protocol::HandleCLS()
  {
    m_BGColor = 0;
    m_FGColor = 0xFFFF;
    m_CursorX = 0;
    m_CursorY = 0;
    Header.fill_rect.w = SCREEN_WIDTH;
    Header.fill_rect.h = SCREEN_HEIGHT;
    HandleFILL_RECT();
  }

  void Protocol::HandleFLIP()
  {
    if (Header.flip.double_buffer > 0)
      buffer_flip();
    else
      single_buffer();
  }

  void Protocol::HandleTEXT_NEWLINE()
  {
    new_line(m_CursorX, m_CursorY);
  }

  void Protocol::HandlePIXEL_CURSOR()
  {
    m_CursorX = Header.pixel_cursor.x;
    m_CursorY = Header.pixel_cursor.y;
  }

  void Protocol::HandleTEXT_CURSOR()
  {
    m_CursorX = Header.text_cursor.x * CHAR_WIDTH;
    m_CursorY = Header.text_cursor.y * CHAR_HEIGHT;
  }

  void Protocol::HandleFG_COLOR()
  {
    m_FGColor = Header.fg_color.color;
  }

  void Protocol::HandleBG_COLOR()
  {
    m_BGColor = Header.bg_color.color;
  }

  void Protocol::HandlePUSH_CURSOR()
  {
    m_CursorStack.push(m_CursorX, m_CursorY);
  }

  void Protocol::HandlePOP_CURSOR()
  {
    m_CursorStack.pop(m_CursorX, m_CursorY);
  }

  void Protocol::HandleBLINK_CURSOR()
  {
    m_Blink = (Header.blink_cursor.state == 1);
  }

  void Protocol::HandleFILL_RECT()
  {
    fill_rect(m_CursorX, m_CursorY,
              Header.fill_rect.w,Header.fill_rect.h,
              m_BGColor);
  }

  void Protocol::HandleHORZ_LINE()
  {
    fill_rect(m_CursorX,m_CursorY,Header.horz_line.w,1,m_FGColor);
    m_CursorX+=Header.horz_line.w;
  }

  void Protocol::HandleVERT_LINE()
  {
    vert_line(m_CursorX,m_CursorY,Header.vert_line.h,m_FGColor);
    m_CursorY+=Header.vert_line.h;
  }

  void Protocol::HandleHORZ_PIXELS()
  {
    const Color* data = (const Color*)(&Header.horz_pixels.opcode + sizeof(Command_HorzPixels));
    blit_pixels(m_CursorX, m_CursorY, data, Header.horz_pixels.n);
    m_CursorX+=Header.horz_pixels.n;
  }

  void Protocol::HandleTEXT()
  {
    const uint8_t* text = (&Header.text.opcode + sizeof(Command_Text));
    draw_str(text, Header.text.n,
             m_CursorX, m_CursorY,
             m_FGColor, m_BGColor, true);
  }

  void Protocol::HandleSET_SPRITE()
  {
    Color* ptr = get_sprite(Header.set_sprite.id);
    if (ptr)
    {
      const Color* src = reinterpret_cast<const Color*>(&Header.set_sprite.opcode + sizeof(Command_SetSprite));
      for (uint16_t i = 0; i < SPRITE_AREA; ++i)
        *ptr++ = *src++;
    }
  }

  void DrawSprite(const Color* pixels,
                  uint16_t srcx, uint16_t srcy,
                  uint16_t w, uint16_t h,
                  uint16_t dstx, uint16_t dsty)
  {
    const Color* src = pixels+srcy*SPRITE_SIZE + srcx;
    for(uint16_t i=0;i<h;++i)
    {
      Color* dst = get_frame_row(dsty+i)+dstx;
      for(uint16_t j=0;j<w;++j)
        dst[j]=src[j];
      src+=SPRITE_SIZE;
    }
  }

  void DrawTransparentSprite(const Color* pixels, Color transparent,
                             uint16_t srcx, uint16_t srcy,
                             uint16_t w, uint16_t h,
                             uint16_t dstx, uint16_t dsty)
  {
    const Color* src = pixels+srcy*SPRITE_SIZE + srcx;
    for(uint16_t i=0;i<h;++i)
    {
      Color* dst = get_frame_row(dsty+i)+dstx;
      for(uint16_t j=0;j<w;++j)
        if (src[j] != transparent)
          dst[j]=src[j];
      src+=SPRITE_SIZE;
    }
  }

  void Protocol::HandleDRAW_SPRITE()
  {
    const Color* pixels = get_sprite(Header.draw_sprite.id);
    if (!pixels) return;
    if (m_CursorX<SCREEN_WIDTH && m_CursorY<SCREEN_HEIGHT &&
        (m_CursorX+SPRITE_SIZE)<=SCREEN_WIDTH && (m_CursorY+SPRITE_SIZE)<=SCREEN_HEIGHT)
    {
      // Fully contained sprite.  Draw entire area
      if (m_Transparency)
        DrawTransparentSprite(pixels,m_Transparent,0,0,
                              SPRITE_SIZE,SPRITE_SIZE,
                              m_CursorX,m_CursorY);
      else
        DrawSprite(pixels,0,0,SPRITE_SIZE,SPRITE_SIZE,
                   m_CursorX,m_CursorY);
    }
    else
    {
      // Partially intersecting the screen area
      constexpr int16_t z=0;
      int16_t srcl=0,srct=0,srcr=SPRITE_SIZE,srcb=SPRITE_SIZE;
      uint16_t dstl,dstt;
      int16_t l=int16_t(m_CursorX);
      int16_t t=int16_t(m_CursorY);
      int16_t r=l+int16_t(SPRITE_SIZE);
      int16_t b=t+int16_t(SPRITE_SIZE);
      if (r<=0 || b<=0 || l>=SCREEN_WIDTH || b>=SCREEN_HEIGHT) return;
      dstl=uint16_t(Max(l,z));
      dstt=uint16_t(Max(t,z));
      if (l<0) srcl=-l;
      if (t<0) srct=-t;
      if (r>SCREEN_WIDTH) srcr-=(r-SCREEN_WIDTH);
      if (b>SCREEN_HEIGHT) srcb-=(b-SCREEN_HEIGHT);
      if (m_Transparency)
        DrawTransparentSprite(pixels,m_Transparent,
                              uint16_t(srcl), uint16_t(srct),
                              uint16_t(srcr-srcl), uint16_t(srcb-srct),
                              dstl,dstt);
      else
        DrawSprite(pixels,
                   uint16_t(srcl), uint16_t(srct),
                   uint16_t(srcr-srcl), uint16_t(srcb-srct),
                   dstl,dstt);
    }
  }

  void Protocol::HandleTRANSPARENT_COLOR()
  {
    m_Transparency = Header.transparent_color.enabled > 0;
    m_Transparent = Header.transparent_color.color;
  }


}


/// Bridge between the low level C code and the high level C++ Protocol
extern "C" {
	void procotol_main()
	{
    gpu::prot.init();
    gpu::prot.add_byte(gpu::CMD_CLS);
		while (true)
		{
			if (uart_available())
			{
				uint8_t c = uart_recv();
                uart_print_hex(c);
                uart_println("");
				gpu::prot.add_byte(c);
			}
      else
      {
        // idle: Handle cursor blink
        gpu::prot.loop();
      }
		}
	}
}
