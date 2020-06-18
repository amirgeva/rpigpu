#!/usr/bin/env python3
import serial

def prt(buf):
    print(buf.decode('ascii'))

ser=serial.Serial('/dev/ttyUSB0',460800,timeout=0.1)
ser.write(b'\x00\x00\x00\x00\x00')
if True:
    prt(ser.read(256))
    ser.write(b'\x08\x1f\x00')
    prt(ser.read(256))
    ser.write(b'\x05\x00\x00\x00\x00')
    prt(ser.read(256))
    ser.write(b'\x14\xff\x00\xff\x00')
    prt(ser.read(256))
else:
    ser.write(b'\x01')
    prt(ser.read(256))
ser.close()
