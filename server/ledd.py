import array
import serial
import time
import socket
import struct

PORT = 8123
GROUP = '224.0.0.1'
TTL = 1

ai = socket.getaddrinfo(GROUP, None)[0]
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(('', PORT))
req = socket.inet_pton(socket.AF_INET, ai[4][0]) + struct.pack('=I', socket.INADDR_ANY)
s.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, req)
s.settimeout(0.5)

port = serial.Serial(
        port = "/dev/rfcomm0",
        baudrate = 9600,
        parity = "N",
        stopbits = 1,
        xonxoff = 0,
        timeout = 3,
        interCharTimeout = 0.5 
    )

port.flushInput()
port.flushOutput()
time.sleep(5)
port.flushInput()
port.flushOutput()

indexes = range(8)
state = [0 for i in indexes]
xor = [0 for i in indexes]


def diff(old):
    return sum([[(i << 4) | state[i]] if state[i] != old[i] else [] for i in indexes], [])

def patch(x):
    for i in x:
        if i & 0x80:
            xor[(i >> 4) & 7] = i & 0xf
        else:
            state[i >> 4] = i & 0xf

def send(b):
    if b == []:
        return
    b[-1] |= 0x80
    print ('out: ' + "".join(map(lambda x: format(x, "02x"), b)))
    s = array.array('B', b).tostring()
    for c in s:
        port.write(c)
        print('>>> %x' % ord(c))
        while True:
            e = port.read(1)
            if e < 0 or e == '':
                raise Exception("oops %d, %s" % (len(e), e))

            if c == e:
                break

            print "echo missmatch: %x != %x" % (ord(c), ord(e))

send(diff([0xf for i in indexes]))

while True:
    old = state[:]
    try:
        data, sender = s.recvfrom(1500)
        buf = bytearray(data)
        print (str(sender) + ' ' + "".join(map(lambda b: format(b, "02x"), buf)))
        patch(buf)
    except socket.timeout as e:
        state = map(lambda (x, y): x ^ y, zip(state, xor))
    send(diff(old))

