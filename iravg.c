//i586-mingw32msvc-gcc iravg.c -o iravg.exe -std=c99 -lcomdlg32

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

	uint16_t *buf_raw = malloc(fsize);
    FILE *fp_raw = fopen(fpath, "rb+");
    double sum = 0.0;
    fread(buf_raw, fsize, 1, fp_raw);
	printf("fszie is %d\n", fsize);
    for(int j=0; j < img_heigh; j++){
        for(int i=0; i < img_width; i++){
          sum += buf_raw[i + j*img_width];
        }
    }
	printf("avg %0.2f\n", sum/(img_width * img_heigh));
	fclose(fp_raw);
	return 0;
}