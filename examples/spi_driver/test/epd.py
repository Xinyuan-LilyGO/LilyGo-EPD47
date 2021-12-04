#!python3
# _*_ coding: utf-8 _*_

import spidev
from math import ceil, floor
from time import sleep, time

'''
System running Command (enable all clocks, and go to active state)
'''
CMD_SYS_RUN = 0x0001

'''
Standby Command (gate off clocks, and go to standby state)
'''
CMD_STANDBY = 0x0002

'''
Sleep Command (disable all clocks, and go to sleep state)
'''
CMD_SLEEP = 0x0003

'''
Read Register Command
'''
CMD_REG_RD = 0x0010

'''
Write Register Command
'''
CMD_REG_WR       = 0x0011

'''
Memory Burst Read Trigger Command (This command will trigger internal FIFO to
read data from memory.)
'''
CMD_MEM_BST_RD_T = 0x0012

'''
Memory Burst Read Start Command (This is only a data read command. It will read
data from internal FIFO. So, this command should be issued after MEM_BST_RD_T
command)
'''
CMD_MEM_BST_RD_S = 0x0013

'''
Memory Burst Write Command
'''
CMD_MEM_BST_WR  = 0x0014

'''
End Memory Burst Cycle
'''
CMD_MEM_BST_END = 0x0015

'''
Load Full Image Command (AEG[15:0] see Register 0x200) (Write Data Number equals
to full display size)
'''
CMD_LD_IMG      = 0x0020

'''
Load Partial Image Command (AEG[15:0] see Register 0x200) (Write Data Number
equals to partial display size according to width and height)
'''
CMD_LD_IMG_AREA = 0x0021

CMD_LD_IMF_END  = 0x0022

CMD_LD_JPEG     = 0x0030

CMD_LD_JPEG_AREA = 0x0031

CMD_LD_JPEG_END  = 0x0032


class EPD47():
    def __init__(self, spi, width, height):
        self.spi = spi
        self.width = width
        self.height = height

    def send_jpeg(self, filename, x = 0, y = 0, w = 0, h = 0):
        width = w
        height = h
        timestamp = time()
        fn_size = 0

        if w == 0:
            width = self.width
        if h == 0:
            height = self.height

        self.send_run_cmd()
        sleep(0.01) #时间还能缩小？待测试

        self.send_ld_jpeg_area_cmd(x, y, width, height, 1)
        sleep(0.02) #时间还能缩小？待测试

        with open(filename, 'rb') as fp:
            while True:
                content = fp.read(4000)
                if len(content)==0:
                    break
                # print("", len(content))
                fn_size = fn_size + len(content)
                a = list(bytes(content))
                # print(a)
                self.send_mem_bst_wr_cmd(a, len(a))
                sleep(0.02)
        sleep(0.02)
        self.send_ld_jpeg_end_cmd()
        sleep(0.02)
        print("jpeg size=", fn_size)
        print("send jpeg time: ", time() - timestamp, "s")

    def send_img(self, img_data, x = 0, y = 0, w = 0, h = 0):
        width = w
        height = h
        timestamp = time()

        self.send_run_cmd()
        sleep(0.01) #时间还能缩小？待测试

        if w == 0:
            width = self.width
        if h == 0:
            height = self.height
        self.send_ld_img_area_cmd(x, y, width, height, 1)
        sleep(0.01) #时间还能缩小？待测试

        i = 0
        while i < len(img_data):
            if (len(img_data) - i) > 4000:
                l = 4000
            else:
                l = len(img_data) - i
            self.send_mem_bst_wr_cmd(img_data[i : (i + l)], l)
            i = i + l
            sleep(0.02)

        self.send_ld_img_end_cmd()
        sleep(0.01)
        print("img size=", len(img_data))
        print("send img time: ", time() - timestamp, "s")

    def send_run_cmd(self):
        payload = []
        payload.append(0x55)
        payload.append(0x55)
        payload.append(0x00) # cmd
        payload.append(0x01)
        payload.append(0x00) # len
        payload.append(0x00)

        self.spi.writebytes(self._append_byte(payload, 4))

    def send_mem_bst_wr_cmd(self, data, len):
        payload = []
        payload.append(0x55)
        payload.append(0x55)
        payload.append(0x00) # cmd
        payload.append(0x14)
        payload.append(((len) & 0xFFFF) >> 8 & 0xFF) #len
        payload.append((len) & 0xFF)
        payload.extend(data)
        self.spi.writebytes(self._append_byte(payload, 8))

    def send_ld_img_cmd(self, mode):
        payload = []
        payload.append(0x55)
        payload.append(0x55)
        payload.append(0x00) # cmd
        payload.append(0x20)
        payload.append(0x00) #len
        payload.append(0x09)
        payload.append(mode)
        self.spi.writebytes(self._append_byte(payload, 4))

    def send_ld_img_area_cmd(self, x, y, width, height, mode):
        payload = []

        payload.append(0x55)
        payload.append(0x55)
        payload.append(0x00) # cmd
        payload.append(0x21)
        payload.append(0x00) #len
        payload.append(0x09)
        payload.append(mode)
        payload.append(x >> 8 & 0xFF)
        payload.append(x & 0xFF)
        payload.append(y >> 8 & 0xFF)
        payload.append(y & 0xFF)
        payload.append(width >> 8 & 0xFF)
        payload.append(width & 0xFF)
        payload.append(height >> 8 & 0xFF)
        payload.append(height & 0xFF)
        self.spi.writebytes(self._append_byte(payload, 4))

    def send_ld_img_end_cmd(self):
        payload = []
        payload.append(0x55)
        payload.append(0x55)
        payload.append(0x00) # cmd
        payload.append(0x22)
        payload.append(0x00) # len
        payload.append(0x00)
        self.spi.writebytes(self._append_byte(payload, 8))

    def send_ld_jpeg_cmd(self, mode):
        payload = []
        payload.append(0x55)
        payload.append(0x55)
        payload.append(0x00) # cmd
        payload.append(0x30)
        payload.append(0x00) #len
        payload.append(0x09)
        payload.append(mode)
        self.spi.writebytes(self._append_byte(payload, 4))

    def send_ld_jpeg_area_cmd(self, x, y, width, height, mode):
        payload = []
        payload.append(0x55)
        payload.append(0x55)
        payload.append(0x00) # cmd
        payload.append(0x31)
        payload.append(0x00) #len
        payload.append(0x09)
        payload.append(mode)
        payload.append(x >> 8 & 0xFF)
        payload.append(x & 0xFF)
        payload.append(y >> 8 & 0xFF)
        payload.append(y & 0xFF)
        payload.append(width >> 8 & 0xFF)
        payload.append(width & 0xFF)
        payload.append(height >> 8 & 0xFF)
        payload.append(height & 0xFF)
        self.spi.writebytes(self._append_byte(payload, 4))

    def send_ld_jpeg_end_cmd(self):
        payload = []
        payload.append(0x55)
        payload.append(0x55)
        payload.append(0x00) # cmd
        payload.append(0x32)
        payload.append(0x00) # len
        payload.append(0x00)
        self.spi.writebytes(self._append_byte(payload, 8))

    def _append_byte(self, msg, div_cnt):
        p1= len(msg) % div_cnt
        if p1 != 0:
            for i in range(div_cnt - p1):
                msg.append(0x00)
        return msg


def load_glyph(font, code_point):
    glyph_index = font.get_char_index(code_point)
    if glyph_index > 0:
        font.load_glyph(glyph_index, freetype.FT_LOAD_RENDER)
        return font
    return None

def norm_floor(val):
    return int(floor(val / (1 << 6)))

def get_char_bounds(font, codepoint, x, y, minx, miny, maxx, maxy):
    face = load_glyph(font, codepoint)
    bitmap = face.glyph.bitmap

    x1 = x + face.glyph.bitmap_left
    y1 = y + (face.glyph.bitmap_top - bitmap.rows)
    x2 = x1 + bitmap.width
    y2 = y1 + bitmap.rows

    if x1 < minx:
        minx = x1
    if y1 < miny:
        miny = y1
    if x2 > maxx:
        maxx = x2
    if y2 > maxy:
        maxy = y2
    x += norm_floor(face.glyph.advance.x)
    return x, y, minx, miny, maxx, maxy


def get_text_bounds(font, str, x, y, x1, y1, w, h):
    minx = 100000
    miny = 100000
    maxx = -1
    maxy = -1
    original_x = x

    for ch in str:
        x, y, minx, miny, maxx, maxy = get_char_bounds(font, ord(ch), x, y, minx, miny, maxx, maxy)
        print("x", x, "y", y, "minx", minx, "miny", miny, "maxx", maxx, "maxy", maxy)

    x1 = min(original_x, minx)
    w = maxx - x1
    y1 = miny
    h = maxy - miny

    return x, y, x1, y1, w, h


def draw_char(font, buffer, cursor_x, cursor_y, buf_width, buf_height, codepoint):
    # print("codepoint:", codepoint)
    face = load_glyph(font, codepoint)

    # get bitmap
    bitmap = face.glyph.bitmap

    print("bitmap.width", bitmap.width)
    print("bitmap.rows", bitmap.rows)
    print("bitmap.len", len(bitmap.buffer))

    width = bitmap.width
    height = bitmap.rows
    left = face.glyph.bitmap_left
    byte_width = (ceil(width / 2) + width % 2)
    # bitmap_size = byte_width * height

    color_lut = []
    for c in range(0, 16):
        color_lut.append(max(0, min(15, 15 + c * ceil(-15 / 15))))

    for y in range(0, height):
        yy = cursor_y - face.glyph.bitmap_top + y
        if yy < 0 or yy >= buf_height:
            continue

        start_pos = cursor_x + left
        # print("yy", yy)
        # print("start_pos", start_pos)
        # print("cursor_x", cursor_x)
        x = max(0, -start_pos)
        max_x = min(start_pos + width, buf_width * 2)
        for xx in range(start_pos, max_x):
            buf_pos = yy * buf_width + floor(xx / 2)
            old = buffer[buf_pos]
            # print("buf_pos", buf_pos)
            bm = bitmap.buffer[y * bitmap.width + x] >> 4

            if ((xx & 1) == 0):
                buffer[buf_pos] = (old & 0xF0) | color_lut[bm]
            else:
                buffer[buf_pos] = (old & 0x0F) | (color_lut[bm] << 4)

            x += 1
    cursor_x += norm_floor(face.glyph.advance.x)

    return cursor_x


def epd_print(epd, font, str, x, y):
# def epd_print(font, str, x, y):
    x1 = 0
    y1 = 0
    w = 0
    h = 0
    tmp_cur_x = x
    tmp_cur_y = y
    # font1 = freetype.Face('./Consolas.ttf')
    font.set_char_size(20 << 6, 20 << 6, 150, 150)
    tmp_cur_x, tmp_cur_y, x1, y1, w, h = get_text_bounds(font, str, tmp_cur_x, tmp_cur_y, x1, y1, w, h)

    print(tmp_cur_x, tmp_cur_y, x1, y1, w ,h)

    buf_width = 0
    buf_height = 0
    baseline_height = y - y1

    local_cursor_x = 0
    local_cursor_y = 0

    buf_width = ceil(w / 2) + w % 2
    buf_height = h
    buffer = [ 255 for i in range(0, buf_width * buf_height) ]
    local_cursor_y = buf_height - baseline_height

    cursor_x_init = local_cursor_x
    cursor_y_init = local_cursor_y

    for ch in str:
        local_cursor_x = draw_char(font, buffer, local_cursor_x, local_cursor_y, buf_width, buf_height, ord(ch))

    x += local_cursor_x - cursor_x_init
    y += local_cursor_y - cursor_y_init

    print("x", x1)
    print("y", y - h + baseline_height)
    print("w", w)
    print("h", h)

    epd.send_img(buffer, x1, y - h + baseline_height, w, h)


if __name__ == '__main__':
    import random
    import pic3
    import logo
    # import freetype

    spi = spidev.SpiDev()
    spi.open(0, 0)
    spi.max_speed_hz = 3900000
    spi.bits_per_word = 8
    spi.mode = 0b11

    epd = EPD47(spi, 960, 540)

    while(True):
        x = random.randint(0, 960-200)
        y = random.randint(0, 540-200)
        epd.send_jpeg('./logo.jpg', x, y, w = 200, h = 200)
        sleep(5)
        epd.send_jpeg('./cat.jpg',)
        sleep(5)
        x = random.randint(0, 960-200)
        y = random.randint(0, 540-200)
        epd.send_img(logo.logo_data, x, y, logo.logo_width, logo.logo_height)
        sleep(5)
        epd.send_img(pic3.pic3_data, 0, 0, pic3.pic3_width, pic3.pic3_height)
        sleep(5)

    spi.close()
