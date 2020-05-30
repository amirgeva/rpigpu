#pragma once

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
#define SCREEN_BPP			16

#if SCREEN_BPP==32
	#define PIXEL_TYPE			uint32_t
#elif SCREEN_BPP==16
	#define PIXEL_TYPE			uint16_t
#else 
	#define PIXEL_TYPE			uint8_t
#endif

typedef PIXEL_TYPE Color;

