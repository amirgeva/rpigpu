#include "timer.h"

#define ARM_TIMER_CTL	0x2000B408
#define ARM_TIMER_CNT	0x2000B420

void init_timer()
{
    // Bits 16:23 define the prescalar
	// Timer ticks are every:
	// 250000000 / (prescaler + 1)
    // Setting prescaler to 249, will make counter count microseconds
	
	// Disable, and then enable free running counter
    write32(ARM_TIMER_CTL, 0x00F90000);
    write32(ARM_TIMER_CTL, 0x00F90200);
}

uint32_t microSeconds()
{
    return read32(ARM_TIMER_CNT);
}

