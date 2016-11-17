import os
import sio
import pio
import time
import bamboo


l = 0
r = 0

leds = 0

s=sio.Sio()


def leds_update():
    global leds, l, r, s

    v = 0
    vb = 0

    if l == 1:
        v |= sio.LEFT
    else:
        if l == 2:
            vb |= sio.LEFT

    if r == 1:
        v |= sio.RIGHT
    else:
        if r == 2:
            vb |= sio.RIGHT

    if leds & 0x4:
        v |= sio.GREEN

    if leds & 0x2:
        v |= sio.AMBER

    if leds & 0x1:
        v |= sio.RED

    s.send(s.leds(v) + map(lambda x: x | 0x80, s.leds(vb)))



leds_update()


class PipelineBamboo(bamboo.Bamboo):
    def update(self, l, percent):
        global leds

        bamboo.Bamboo.update(self, l, percent)

        if l & 2:
            if percent < 100:
                s.send(s.digit(0, str(percent / 10), False) + s.digit(1, str(percent % 10), False))
            else:
                s.send(s.digit(0, "-", False) + s.digit(1, "-", False))
        else:
            s.send(s.digit(0, " ", False) + s.digit(1, " ", False))

        if (leds & 0x2) and (not (l & 2)):
            if l & 1:
                os.system("./red &")

            if l & 4:
                os.system("./green &")

        if leds != l:
            leds = l
            leds_update()

pipeline = PipelineBamboo("PRB-NIGHTLY")
pipeline.start()

class NightlyBamboo(bamboo.Bamboo):
    def update(self, leds, percent):
        global r

        bamboo.Bamboo.update(self, leds, percent)

        if leds & 2:
            nr = 2
        else:
            if leds & 1:
                nr = 1
            else:
                nr = 0

        if r != nr:
            r = nr
            leds_update()

#nightly = NightlyBamboo("SOE-NIGHTLY")
#nightly.start()

class MyPio(pio.Pio):
    def debounce(self, x, y):
        global l, r

        if x & 0x8 and not y & 0x8:
            l = not l
            leds_update()
            os.system("./button %d %d %d %d &" % (l, r, x, y))

#        if x & 0x2 and not y & 0x2:
#            nightly.trigger()


p=MyPio()
p.run()
