from smbus2 import SMBus, i2c_msg
from collections import namedtuple

TouchData = namedtuple("TouchData", ["id", "state", "x", "y"])

class L58():
    def __init__(self, bus, address):
        self.bus = bus
        self.address = address
        self.touchData = []

    def begin(self):
        self.wakeup()

    def readBytes(self, write, nbytes):
        write = i2c_msg.write(self.address, write)
        read = i2c_msg.read(self.address, nbytes)
        self.bus.i2c_rdwr(write, read)
        return list(read)

    def clearFlags(self):
        self.bus.write_i2c_block_data(self.address, 0xD0, [0x00, 0xAB])

    def scanPoint(self):
        sumL = 0
        sumH = 0
        pointData = []

        buffer = self.readBytes([0xD0, 0x00], 7)
        if buffer[0] == 0xAB:
            self.clearFlags()
            return 0

        pointData.extend(buffer[0:5])

        point = buffer[5] & 0x0F
        if point == 1:
            buffer = self.readBytes( [0xD0, 0x07], 2)
            sumL = buffer[0] << 8 | buffer [1]
            pointData.extend(buffer)
        elif point > 1:
            buffer = self.readBytes([0xD0, 0x07], 5 * (point - 1) + 3)
            pointData.extend(buffer)
            sumL = pointData[5 * point + 1] << 8 | pointData[5 * point + 2]
        self.clearFlags()
        

        for i in range(0, 5 * point):
            sumH += pointData[i]

        if sumH != sumL:
            point = 0

        if (point):
            for i in range(0, point):
                if i == 0:
                    offset = 0
                else:
                    offset = 4

                id = (pointData[i * 5 + offset] >> 4) & 0x0F
                state = pointData[i * 5 + offset] & 0x0F
                if state == 0x06:
                    state = 0x07
                else:
                    state = 0x06
                y = ((pointData[i * 5 + 1 + offset] << 4) | ((pointData[i * 5 + 3 + offset] >> 4) & 0x0F))
                x = ((pointData[i * 5 + 2 + offset] << 4) | (pointData[i * 5 + 3 + offset] & 0x0F))
                touchdata = TouchData(id = id, state= state, x = x, y = y)
                self.touchData.append(touchdata)
                # print(id, state, x, y)
        else:
            point = 1
            id = pointData[0] >> 4 & 0x0F
            state = 0x06
            y = pointData[1] << 4 | pointData[3] >> 4 & 0x0F
            x = pointData[2] << 4 | pointData[3] & 0x0F
            touchdata = TouchData(id = id, state= state, x = x, y = y)
            self.touchData.append(touchdata)
            point = 1
            # print(id, state, x, y)
        return point

    def getPoint(self):
        return self.touchData.pop()

    def sleep(self):
        self.bus.write_byte_data(self.address, 0xD1, 0x05)

    def wakeup(self):
        self.bus.write_byte_data(self.address, 0xD1, 0x06)


if __name__ == '__main__':
    from time import sleep
    tp = L58(SMBus(1), 0x5A)
    tp.begin()
    while True:
        for i in range(0, tp.scanPoint()):
            data = tp.getPoint()
            print(data)
        sleep(0.1)
