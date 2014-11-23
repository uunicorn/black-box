import array
import serial
import time
import re
import socket
import struct

PORT = 8124
GROUP = '224.0.0.1'
TTL = 1

ai = socket.getaddrinfo(GROUP, None)[0]
sock = socket.socket(ai[0], socket.SOCK_DGRAM)
ttl_bin = struct.pack('@i', TTL)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl_bin)

port = serial.Serial()
port.baudrate = 9600
port.port = "/dev/rfcomm1"
port.parity = "N"
port.stopbits = 1
port.xonxoff = 0
port.timeout = 3
port.open()

port.flushInput()
port.flushOutput()
time.sleep(5)
port.flushInput()
port.flushOutput()

def send_at_command(command):
    port.write(command+"\r")

def read_command_response():
    print(port.read(100).decode('ascii').strip())
    print("\n")

def debounce(x, y):
    print ">>> %d -> %d\n" % (x, y)
    sock.sendto(struct.pack('HH', x, y), (ai[4][0], PORT))

send_at_command("AT+PIN=10")
read_command_response()

send_at_command("AT+DEBOUNCE=10")
read_command_response()

port.timeout = None

for line in port:
    m = re.match('^\+CHANGED=(\d+),(\d+)', line)
    if m:
        debounce(int(m.groups()[0]), int(m.groups()[1]))
