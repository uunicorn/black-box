import array
import time
import re
import socket
import struct

PORT = 8123
GROUP = '224.0.0.1'
TTL = 1

A = 0x2
B = 0x1
C = 0x40
D = 0x4
E = 0x10
F = 0x80
G = 0x8
H = 0x20

#    B
#  ----- 
# |     |
# |A    | C
# |  D  |
#  -----
# |     |
# |E    | F
# |  G  |
#  -----  [H] 

sign = { 
    ' ' : 0,
    '-' : D,
    '0' : A|B|C|E|F|G,
    '1' : C|F,
    '2' : B|C|D|E|G,
    '3' : B|C|D|F|G,
    '4' : A|D|C|F,
    '5' : B|A|D|F|G,
    '6' : B|A|D|E|F|G,
    '7' : B|C|F,
    '8' : A|B|C|D|E|F|G,
    '9' : A|B|C|D|F|G,
    'A' : A|B|C|D|E|F,
    'B' : A|D|E|F|G,
    'C' : A|B|E|G,
    'D' : C|D|E|F|G,
    'E' : A|B|D|E|G,
    'F' : A|B|D|E
}

LEFT=0x01
RIGHT=0x40
RED=0x04
AMBER=0x02
GREEN=0x80

class Sio(object):
    def __init__(self):
        self.ai = socket.getaddrinfo(GROUP, None)[0]
        self.sock = socket.socket(self.ai[0], socket.SOCK_DGRAM)
        ttl_bin = struct.pack('@i', TTL)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl_bin)

    def send(self, b):
        self.sock.sendto(array.array('B', b).tostring(), (self.ai[4][0], PORT))

    def byte2nibles(self, v):
        return [v & 0xf, (v >> 4) | 0x40]

    def leds(self, v):
        return self.byte2nibles(v)

    def digit(self, n, digit, dot):
        v = sign[digit]
        if dot:
            v |= H
        return map(lambda x: x | [0x20, 0x30][n], self.byte2nibles(v))
