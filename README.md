# Raspberry Pi Bare Metal Graphics Adapter

## Goal:
Kernel for running a raspberry pi (zero) as a serially accessible HDMI graphics adapter.

This allows any home brew machine to get a graphics adapter with HDMI out.

## Interface:

Accessing the frame buffer is done via a protocol over the serial port.  Sprites are stored in the pi's memory, so the bandwidth required from the serial should not be a bottleneck.

The protocol details are in the [wiki](https://github.com/amirgeva/rpigpu/wiki/Protocol)

## Building:

Run the genmk.py  script to generate the Makefile and other required files

Then, Run: make

## Connecting:

On the pi-zero board, connect the uart on the three pins marked in the image below

![Raspberry PI Zero UART](https://raw.githubusercontent.com/amirgeva/rpigpu/master/board.png)


