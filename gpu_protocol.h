#pragma once

#include "gpu.h"
#include "defs.h"
#include "font.h"
#include "sprites.h"

namespace gpu {


  constexpr uint8_t CMD_NOP = 0;
  constexpr uint8_t CMD_CLS = 1;
  constexpr uint8_t CMD_FLIP = 2;
  constexpr uint8_t CMD_TEXT_NEWLINE = 4;
  constexpr uint8_t CMD_PIXEL_CURSOR = 5;
  constexpr uint8_t CMD_TEXT_CURSOR = 6;
  constexpr uint8_t CMD_FG_COLOR = 7;
  constexpr uint8_t CMD_BG_COLOR = 8;
  constexpr uint8_t CMD_PUSH_CURSOR = 9;
  constexpr uint8_t CMD_POP_CURSOR = 10;
  constexpr uint8_t CMD_BLINK_CURSOR = 11;
  constexpr uint8_t CMD_FILL_RECT = 20;
  constexpr uint8_t CMD_HORZ_LINE = 21;
  constexpr uint8_t CMD_VERT_LINE = 22;
  constexpr uint8_t CMD_HORZ_PIXELS = 23;
  constexpr uint8_t CMD_TEXT = 30;
  constexpr uint8_t CMD_SET_SPRITE = 40;
  constexpr uint8_t CMD_DRAW_SPRITE = 41;


#pragma pack(push,1)

  struct Command
  {
    uint8_t opcode;
  };

  struct Command_CLS
  {
    uint8_t opcode;
  };

  struct Command_NewLine
  {
    uint8_t opcode;
  };

  struct Command_Flip
  {
    uint8_t opcode;
    uint8_t double_buffer;
  };

  struct Command_PushCursor
  {
    uint8_t opcode;
  };

  struct Command_PopCursor
  {
    uint8_t opcode;
  };

  struct Command_BlinkCursor
  {
    uint8_t opcode;
    uint8_t state;
  };

  struct Command_PixelCursor
  {
    uint8_t opcode;
    uint16_t x, y;
  };

  struct Command_TextCursor
  {
    uint8_t opcode;
    uint8_t x, y;
  };

  struct Command_FillRect
  {
    uint8_t opcode;
    uint16_t w, h;
  };

  struct Command_HorzLine
  {
    uint8_t opcode;
    uint16_t w;
  };

  struct Command_VertLine
  {
    uint8_t opcode;
    uint16_t h;
  };

  struct Command_HorzPixels
  {
    uint8_t opcode;
    uint8_t n;
  };

  struct Command_Text
  {
    uint8_t opcode;
    uint8_t n;
  };

  struct Command_FGColor
  {
    uint8_t opcode;
    Color   color;
  };

  struct Command_BGColor
  {
    uint8_t opcode;
    Color   color;
  };

  struct Command_SetSprite
  {
    uint8_t  opcode;
    uint16_t id;
  };

  struct Command_DrawSprite
  {
    uint8_t  opcode;
    uint16_t id;
  };

  class CursorStack
  {
    enum { SIZE = 32 };
    uint16_t m_Data[SIZE];
    uint16_t m_Pos;
  public:
    void init()
    {
      m_Pos=SIZE;
    }

    void push(uint16_t x, uint16_t y)
    {
      if (m_Pos > 0)
      {
        m_Data[--m_Pos] = y;
        m_Data[--m_Pos] = x;
      }
    }

    void pop(uint16_t& x, uint16_t& y)
    {
      if (m_Pos < SIZE)
      {
        x = m_Data[m_Pos++];
        y = m_Data[m_Pos++];
      }
    }
  };

  class Protocol
  {
    CursorStack   m_CursorStack;
    uint16_t      m_Pos, m_BytesLeft;
    uint16_t      m_CursorX, m_CursorY;
    Color         m_FGColor, m_BGColor;
    bool          m_Blink;
    typedef void (Protocol::*Handler)();
    Handler       m_Handlers[64];
    uint16_t      m_CommandSizes[64];

    union {
      Command             cmd;
      Command_CLS         cls;
      Command_Flip        flip;
      Command_NewLine     newline;
      Command_PixelCursor pixel_cursor;
      Command_TextCursor  text_cursor;
      Command_PushCursor  push_cursor;
      Command_PopCursor   pop_cursor;
      Command_BlinkCursor blink_cursor;
      Command_FillRect    fill_rect;
      Command_HorzLine    horz_line;
      Command_VertLine    vert_line;
      Command_HorzPixels  horz_pixels;
      Command_Text        text;
      Command_FGColor     fg_color;
      Command_BGColor     bg_color;
      Command_SetSprite   set_sprite;
      Command_DrawSprite  draw_sprite;
      uint8_t             buffer[SPRITE_SIZE*SPRITE_SIZE*sizeof(Color) + sizeof(Command_SetSprite)];
    } Header;

    uint16_t get_header_len();

    uint16_t check_variable_len();

    void XorRect(int x, int y, int w, int h);
    void erase_cursor();
    bool process_command();

    void HandleNOP();
    void HandleCLS();
    void HandleFLIP();
    void HandleTEXT_NEWLINE();
    void HandlePIXEL_CURSOR();
    void HandleTEXT_CURSOR();
    void HandleFG_COLOR();
    void HandleBG_COLOR();
    void HandlePUSH_CURSOR();
    void HandlePOP_CURSOR();
    void HandleBLINK_CURSOR();
    void HandleFILL_RECT();
    void HandleHORZ_LINE();
    void HandleVERT_LINE();
    void HandleHORZ_PIXELS();
    void HandleTEXT();
    void HandleSET_SPRITE();
    void HandleDRAW_SPRITE();

  public:
    void init();
    void loop();
    void add_byte(uint8_t b);
  };

  extern Protocol prot;

#pragma pack(pop)

}
