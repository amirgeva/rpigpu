#include "uart.h"

#define AUX_MU_IO_REG	0x20215040
#define AUX_MU_LSR_REG	0x20215054

uint8_t uart_recv()
{
    while(1)
    {
        if (read32(AUX_MU_LSR_REG)&0x01) break;
    }
    return read32(AUX_MU_IO_REG)&0xFF;
}

uint32_t uart_available()
{
	return ((read32(AUX_MU_LSR_REG)&0x01)!=0);
}

void uart_send(uint8_t ch)
{
    while(1)
    {
        if (read32(AUX_MU_LSR_REG)&0x20) break;
    }
    write32(AUX_MU_IO_REG,ch);
}

void uart_flush()
{
	// Wait until outgoing FIFO is clear
    while(1)
    {
        if ((read32(AUX_MU_LSR_REG)&0x100)==0) break;
    }
}

void uart_print(const char* text)
{
	for(;*text;++text)
		uart_send(*text);
}

void uart_println(const char* text)
{
	uart_print(text);
	uart_print("\r\n");
}

void uart_print_hex(uint8_t c)
{
	static const uint8_t* hex=(const uint8_t*)("0123456789ABCDEF");
	uart_send(hex[(c>>4)&15]);
	uart_send(hex[c&15]);
}

void uart_print_hex_dword(uint32_t v)
{
	int i;
	for(i=24;i>=0;i-=8)
	{
		uint8_t c=(v>>i)&255;
		uart_print_hex(c);
	}
}

void delay_cycles(int n)
{
	while (--n>=0) cycle_delay();
}

void disable_gpio_pull_up_down()
{
#define GPPUD			0x20200094
#define GPPUDCLK0		0x20200098
    write32(GPPUD,0);
	delay_cycles(150);
	write32(GPPUDCLK0,0xC000); // One bit for 14, and one for 15
	delay_cycles(150);
    write32(GPPUDCLK0,0);
}

uint32_t set_bits(uint32_t word, int bits, int offset, uint32_t value)
{
	uint32_t mask = ((1<<bits)-1);
	value &= mask;
	word &= ~(mask << offset);
	word |= (value << offset);
	return word;
}

void init_uart()
{
#define AUX_ENABLES		0x20215004
#define AUX_MU_IER_REG	0x20215044
#define AUX_MU_IIR_REG	0x20215048
#define AUX_MU_LCR_REG	0x2021504C
#define AUX_MU_MCR_REG	0x20215050
#define AUX_MU_CNTL_REG	0x20215060
#define AUX_MU_BAUD_REG	0x20215068

    uint32_t fsel=0;
	// Enable mini uart
    write32(AUX_ENABLES,1);
	// Disable interrupts
    write32(AUX_MU_IER_REG,0);
	// Disable hardware handshaking, and disable Tx,Rx for setup
    write32(AUX_MU_CNTL_REG,0);
	// Set to 8 bit mode (re errata)
    write32(AUX_MU_LCR_REG,3);
	// Control RTS policy
    write32(AUX_MU_MCR_REG,0);
	// Clear FIFOs
    write32(AUX_MU_IIR_REG,6);
	// Set baud rate to 460800   Round((250000000/(8*BAUD))-1)
    write32(AUX_MU_BAUD_REG,67);
	
#define GPFSEL1			0x20200004
    fsel=read32(GPFSEL1);
	// GPIO 14 function has 3 bits, starting at bit 12
	// Set it to 2, which is alternate function 5
	fsel=set_bits(fsel, 3, 12, 2);
	// GPIO 15 function has 3 bits, starting at bit 15
	// Set it to 2, which is alternate function 5
	fsel=set_bits(fsel, 3, 15, 2);
    write32(GPFSEL1,fsel);
	disable_gpio_pull_up_down();
	
	// Enable Tx, Rx
    write32(AUX_MU_CNTL_REG,3);
}

