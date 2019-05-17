//i586-mingw32msvc-gcc sonix2duxin.c -o sonix2duxin.exe -std=c99 -lcomdlg32

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <windows.h>
#include <stdbool.h>

unsigned int img_width=1280;
unsigned int img_heigh=800;

bool get_file_path(char *fpath)
{
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = fpath;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.hInstance = (HMODULE)GetCurrentProcess();

    return GetOpenFileNameA(&ofn);
}

int main(int argc, char **argv)
{   
    char fpath[MAX_PATH] = {0};
    if(argc == 1) {
        if (get_file_path(fpath) == false)
            return -1;
    } else {
        strncpy(fpath, (char *)argv[1], sizeof(fpath));
    }
    unsigned int fsize = sizeof(uint16_t) * img_width * img_heigh;

	printf("fszie is %d\n", fsize);

	uint16_t *buf_sonix = malloc(fsize);
    uint16_t *buf_final = malloc(fsize);
	
    FILE *fp_sonix = fopen(fpath, "rb+");
	FILE *fp_final = fopen("ref.irh", "wb+");

    fread(buf_sonix, fsize, 1, fp_sonix);

	printf("fszie is %d\n", fsize);
    for(int j=0; j < img_heigh; j++){
        for(int i=0; i < img_width; i++){
          buf_final[i + j * img_width] = ((buf_sonix[i + j * img_width] & 0x00FF) << 2) | ((buf_sonix[i + j * img_width] & 0x0300) >> 8);
		  //printf("0x%04x\n", buf_sonix[i + j * img_width]);
        }
    }

    fwrite(buf_final, fsize, 1, fp_final);

    fclose(fp_sonix);
    fclose(fp_final);

    return 0;
}
