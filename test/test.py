#!/usr/bin/env python3
import serial
import time

def prt(buf):
    print(buf.decode('ascii'))

# '/dev/ttyUSB0'
port = 'COM4'
ser=serial.Serial(port,460800,timeout=0.001)
ser.write(b'\x00\x00\x00\x00\x01')
#prt(ser.read(256))
ser.write(b'\x08\x1f\x00')
#prt(ser.read(256))
for x in range(200):
    if x>0:
        ser.write(b'\x08\x00\x00')
        ser.write(bytes([5,x-1,0,0,0]))
        ser.write(b'\x14\x01\x00\x1f\x00')
        ser.write(b'\x08\x1f\x00')
    ser.write(bytes([5,x,0,0,0]))
    #prt(ser.read(256))
    ser.write(b'\x14\x1f\x00\x1f\x00')
    #res=ser.read(256)
    #prt(res)
time.sleep(1)
prt(ser.read(65536))
ser.close()
