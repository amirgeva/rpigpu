#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"

// Initialize UART to 460800 N81
void init_uart();

// Block and receive a single character
uint8_t  uart_recv();

// Check if data is available
uint32_t uart_available();

// Send a single character
void uart_send(uint8_t ch);

// Wait until outgoing FIFO is clear
void uart_flush();

// Send text characters
void uart_print(const char* text);

// Send text characters and newline
void uart_println(const char* text);

// Send the hex representation of this byte
void uart_print_hex(uint8_t c);

// Send the hex representation of these 4 bytes
void uart_print_hex_dword(uint32_t v);


/** Test if incoming buffer is empty */
uint32_t uart_buffer_empty();

/** Test if incoming buffer is full */
uint32_t uart_buffer_full();

/** Read from incoming FIFO queue.
    Returns -1 if no input is available. */
int uart_buffer_read();


#ifdef __cplusplus
}
#endif
