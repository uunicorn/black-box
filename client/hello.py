import os
import sio

s=sio.Sio()


for i in range(0,20):
    s.send(s.leds(1 << (i % 8)) + s.digit(0, str(i / 10), False) + s.digit(1, str(i % 10), False))

