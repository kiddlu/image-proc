import mmap
import getopt
import sys
import os
import struct
import tkinter.filedialog as tk_fdiag
import numpy as np
import cv2 as cv

IMAGE_WIDTH = 1328
IMAGE_HEIGH = 1120

def get_depth_data(buf, x, y):
    return int(buf[x + y * IMAGE_WIDTH])

if __name__ == "__main__":
    try:    
        opts, args = getopt.getopt(sys.argv[1:], "w:h:", ["width=", "heigh="])    
    except getopt.GetoptError:
        print("param error")

    for o, a in opts:
        if o in ("-h", "--heigh"):    
            IMAGE_HEIGH = int(a)    
        elif o in ("-w", "--width"):    
            IMAGE_WIDTH = int(a)

    fpath = tk_fdiag.askopenfilename(filetypes=[("raw file", "*.raw"), ("data file", "*.data")])
    if fpath == '':
        sys.exit()
    
    f = open(fpath, "rb")
    mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
    buf = []
   
    for i in range(0, IMAGE_WIDTH * IMAGE_HEIGH, 1) :
        data = int.from_bytes(mm.read(2), byteorder="little")
        buf.append("%d" % data)

    image_orig = np.zeros((IMAGE_HEIGH, IMAGE_WIDTH, 1), dtype=np.uint16)
    image_show = np.zeros((IMAGE_HEIGH, IMAGE_WIDTH, 1), dtype=np.uint8)

    for y in range(0, IMAGE_HEIGH):
        for x in range(0, IMAGE_WIDTH):
            #gray
            image_orig[y, x] =  get_depth_data(buf, x, y)

    max_value = np.max(image_orig)
    if max_value == 0:
        max_value = 1024

    for y in range(0, IMAGE_HEIGH):
        for x in range(0, IMAGE_WIDTH):
            image_show[y, x] =  image_orig[y, x] / max_value * 255

    cv.namedWindow("ShowIR", cv.WINDOW_AUTOSIZE)
    cv.imshow("ShowIR", image_show)
    cv.waitKey()
    save = input("save this pic to bmp? y/n :")
    if save == "y":
        cv.imwrite("%s.bmp" % os.path.basename(fpath).split('.')[0], image_show)