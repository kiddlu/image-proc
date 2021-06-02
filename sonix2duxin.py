import mmap
import getopt
import sys
import os
import struct
import tkinter.filedialog as tk_fdiag
import numpy as np


IMAGE_WIDTH=1280
IMAGE_HEIGH=800

if __name__ == "__main__":
    try:    
        opts, args = getopt.getopt(sys.argv[1:], "w:h:", ["width=", "heigh="])    #处理命令行参数
    except getopt.GetoptError:
        print("param error")

    for o, a in opts:
        if o in ("-h", "--heigh"):    
            IMAGE_HEIGH = int(a)    
        elif o in ("-w", "--width"):    
            IMAGE_WIDTH = int(a)

    #fpath = tk_fdiag.askopenfilename(filetypes=[("data file", "*.data"), ("raw file", "*.raw")])
    # fpath = "D:\\Python\\runtime\\DepthFrame(640X480)_20180703144953.data"
    fns = tk_fdiag.askopenfilenames(filetypes=[("data file", "*.data"), ("raw file", "*.raw"), ("jpg file", "*.jpg")])
    for fpath in fns:
        if fpath == '':
            sys.exit()
    
        f = open(fpath, "rb")
        mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)#内存优化

        buf_sonix = np.zeros(IMAGE_WIDTH * IMAGE_HEIGH, dtype=np.uint16)
        buf_final = np.zeros(IMAGE_WIDTH * IMAGE_HEIGH, dtype=np.uint16)

        fsize = 2 * IMAGE_WIDTH * IMAGE_HEIGH;
        print('fszie is ',fsize)

        for i in range(0, IMAGE_WIDTH * IMAGE_HEIGH, 1) :
            data = int.from_bytes(mm.read(2), byteorder="little")#一次读两个，小端
            buf_sonix[i] = data

        for j in range (IMAGE_HEIGH):
            for i in range (IMAGE_WIDTH):
            	buf_final[i + j * IMAGE_WIDTH] = ((buf_sonix[i + j * IMAGE_WIDTH] & 0x00FF) << 2) | ((buf_sonix[i + j * IMAGE_WIDTH] & 0x0300) >> 8)

        fp_final = os.path.basename(fpath) + ".raw"
        f = open(fp_final, 'wb')
        f.write(bytes(buf_final))
        #print(os.getcwd())
    print("Finish converting")