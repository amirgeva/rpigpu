#include "gpu.h"
#include "uart.h"
#include "timer.h"

uint32_t write_mailbox(uint32_t addr, uint8_t channel)
{
	// Assume message is 16 byte aligned,
	// and that channel is a valid value 0-15
	const uint32_t mailbox = 0x2000B880;
	uint32_t status, value = addr + channel;
	// Wait for mailbox to be empty
	while (1)
	{
		status = read32(mailbox+0x18);
		if ((status & 0x80000000) == 0) break;
	}
	write32(mailbox + 0x20,value);
	return 0;
}

uint32_t read_mailbox(uint8_t channel)
{
	const uint32_t mailbox = 0x2000B880;
	uint32_t status, value=0;
	do
	{
		// Wait for response
		while (1)
		{
			status = read32(mailbox+0x18);
			if ((status & 0x40000000)==0) break;
		}
		value = read32(mailbox);
	} while ((value&0x0F) != channel);
	return value;
}



// Pointer to screen pixels (filled on init)
static PIXEL_TYPE*	frame_buffer;
static uint32_t  	pitch;
static uint32_t		y_offset;

PIXEL_TYPE* get_frame_row(int y)
{
	uint8_t* ptr = (uint8_t*)frame_buffer;
	if (y<0 || y>=SCREEN_HEIGHT) return 0;
	ptr+=((y+y_offset)*pitch);
	return (PIXEL_TYPE*)ptr;
}

PIXEL_TYPE* get_frame_buffer()
{
	return get_frame_row(0);
}

uint16_t    get_frame_width()
{
	return SCREEN_WIDTH;
}

uint16_t    get_frame_height()
{
	return SCREEN_HEIGHT;
}


void buffer_flip()
{
	// Make it 16 byte aligned (GPU requirement)
	volatile uint32_t msg[32] __attribute__ ((aligned (16)));
	// Add 0x40000000 to make GPU flush its output back to our buffer
	uint32_t msgaddr=((uint32_t)msg) + 0x40000000;
	int i=0;
	msg[i++]=32*4;		// Buffer size
	msg[i++]=0;			// Request=0
	msg[i++]=0x48009;	// Set Virtual Offset Tag
	msg[i++]=8;			// Parameters size (x,y 32bit each)
	msg[i++]=0;			// Request=0
	msg[i++]=0;			// offset x = 0  we're offseting just y
	msg[i++]=y_offset;	// offset y
	msg[i++]=0;			// end tag
	for(;i<32;++i) msg[i]=0;
	write_mailbox(msgaddr,8); // Send to channel 8
	read_mailbox(8);
	y_offset = SCREEN_HEIGHT-y_offset;
}

void single_buffer()
{
	if (y_offset==0)
		buffer_flip();
	y_offset=0;
}

void init_screen()
{
	// Make it 16 byte aligned (GPU requirement)
	volatile uint32_t msg[32] __attribute__ ((aligned (16)));
	// Add 0x40000000 to make GPU flush its output back to our buffer
	uint32_t msgaddr=((uint32_t)msg) + 0x40000000;
	// Fill screen init message
	int i=0;
	msg[i++]=SCREEN_WIDTH;
	msg[i++]=SCREEN_HEIGHT;
	msg[i++]=SCREEN_WIDTH;
	msg[i++]=2*SCREEN_HEIGHT;
	msg[i++]=0;
	msg[i++]=SCREEN_BPP;
	msg[i++]=0;
	msg[i++]=0;
	msg[i++]=0;
	msg[i++]=0;
	write_mailbox(msgaddr,1);
	read_mailbox(1);
	pitch = *((uint32_t*)(msgaddr+0x10));
	uint32_t* fb_ptr = (uint32_t*)(msgaddr+0x20);
	frame_buffer = (PIXEL_TYPE*)(*fb_ptr);
	y_offset=0;
}

void procotol_main();

void print_val(const char* prefix, uint32_t value)
{
	uart_print(prefix);
	uart_print_hex_dword(value);
	uart_println(" ");
}

void gpu_main()
{
	uint32_t x,y;

	init_uart();
    init_timer();
	uart_println("Init Screen");
	init_screen();

	print_val("FrameBuffer: ",(uint32_t)frame_buffer);
	print_val("Pitch=",pitch);
	print_val("y_offset=",y_offset);
	
	uart_println("Clear FrameBuffer");
	for(y=0;y<SCREEN_HEIGHT;++y)
	{
		PIXEL_TYPE* row=get_frame_row(y);
		for(x=0;x<SCREEN_WIDTH;++x)
			row[x]=0xF800;
	}
	uart_println("Fill Rect");
	fill_rect(0,0,200,100,0x1F);

	procotol_main();
}

uint32_t micros()
{
	return microSeconds();
}

void vert_line(uint16_t x, uint16_t y, uint16_t h, PIXEL_TYPE color)
{
	uint16_t i;
	if (x>=SCREEN_WIDTH || y>=SCREEN_HEIGHT || (y+h)>SCREEN_HEIGHT)
		return;
	for(i=0;i<h;++i)
	{
		PIXEL_TYPE* row=get_frame_row(y+i)+x;
		*row=color;
	}
}

void blit_pixels(uint16_t x, uint16_t y, const PIXEL_TYPE* data, uint16_t n)
{
	uint16_t i;
	if (x>=SCREEN_WIDTH || y>=SCREEN_HEIGHT || (x+n)>SCREEN_HEIGHT)
		return;
	PIXEL_TYPE* row=get_frame_row(y)+x;
	for(i=0;i<n;++i)
		row[i]=data[i];
}

void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, PIXEL_TYPE color)
{
	uint16_t i,j;
	if (x>=SCREEN_WIDTH || y>=SCREEN_HEIGHT || (x+w)>SCREEN_WIDTH || (y+h)>SCREEN_HEIGHT)
	{
		uart_println("Invalid rect");
		return;
	}
	for(i=0;i<h;++i)
	{
		PIXEL_TYPE* row=get_frame_row(y+i)+x;
		for(j=0;j<w;++j)
			row[j]=color;
	}
}

void frame_scroll(uint16_t rows, PIXEL_TYPE bottom_fill_color)
{
	uint16_t i,n,x;
	if (rows>=SCREEN_HEIGHT) return;
	n = SCREEN_HEIGHT - rows;
	for(i=0;i<n;++i)
	{
		PIXEL_TYPE* dst = get_frame_row(i);
		PIXEL_TYPE* src = get_frame_row(i+rows);
		for(x=0;x<SCREEN_WIDTH;++x)
			*dst++ = *src++;
	}
	for(i=n;i<SCREEN_HEIGHT;++i)
	{
		PIXEL_TYPE* dst = get_frame_row(i);
		for(x=0;x<SCREEN_WIDTH;++x)
			*dst++ = bottom_fill_color;
	}
}