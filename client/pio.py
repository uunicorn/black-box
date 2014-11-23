import array
import time
import socket
import struct

PORT = 8124
GROUP = '224.0.0.1'
TTL = 1


class Pio(object):
    def __init__(self):
        ai = socket.getaddrinfo(GROUP, None)[0]
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('', PORT))
        req = socket.inet_pton(socket.AF_INET, ai[4][0]) + struct.pack('=I', socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, req)

    def debounce(self, x, y):
        print ">>> %d -> %d\n" % (x, y)

    def run(self):
        while True:
            data, sender = self.sock.recvfrom(1500)
            buf = bytearray(data)
            print (str(sender) + ' ' + "".join(map(lambda b: format(b, "02x"), buf)))
            (x, y) = struct.unpack('HH', data)
            self.debounce(x, y)
