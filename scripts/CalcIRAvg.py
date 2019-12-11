import mmap
import getopt
import sys
import os
import struct
import tkinter.filedialog as tk_fdiag
import numpy as np
import cv2 as cv

IMAGE_WIDTH = 1280
IMAGE_HEIGH = 800

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

    fns = tk_fdiag.askopenfilenames(filetypes=[("raw file", "*.raw")])
    for fpath in fns:
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
     
        print(os.path.basename(fpath), np.ndarray.mean(image_orig))