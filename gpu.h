#pragma once

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif


PIXEL_TYPE* get_frame_row(int y);
PIXEL_TYPE* get_frame_buffer();
void       	buffer_flip();
void        single_buffer();
uint16_t    get_frame_width();
uint16_t    get_frame_height();
void        fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, PIXEL_TYPE color);
void        vert_line(uint16_t x, uint16_t y, uint16_t h, PIXEL_TYPE color);
void        blit_pixels(uint16_t x, uint16_t y, const PIXEL_TYPE* data, uint16_t n);
void        frame_scroll(uint16_t rows, PIXEL_TYPE bottom_fill_color);

uint32_t micros();
uint32_t write_mailbox(uint32_t addr, uint8_t channel);
uint32_t read_mailbox(uint8_t channel);

void procotol_main();

#ifdef __cplusplus
}
#endif
