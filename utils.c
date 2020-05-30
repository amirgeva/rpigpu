#include "utils.h"


void write32(uint32_t address, uint32_t value)
{
	uint32_t* ptr = (uint32_t*)address;
	*ptr = value;
}

uint32_t read32(uint32_t address)
{
	uint32_t* ptr = (uint32_t*)address;
	return *ptr;
}

void cycle_delay()
{
}
