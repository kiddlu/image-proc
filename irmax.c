//i586-mingw32msvc-gcc irmax.c -o irmax.exe -std=c99 -lcomdlg32

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

unsigned int max_ratio = 500;// 1/500, 2048 points

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

static int comp(const void *a, const void *b)
{
  return (*(uint16_t *)b - *(uint16_t *)a);//up
  //return (*(int *)a - *(int *)b);//down
}

int quick_sort(uint16_t array[], int len)
{
  int start = 0;
  int end = len - 1;

  qsort(&array[start], end + 1, sizeof(uint16_t), comp);
  return 0;
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
    uint32_t max_avg = 0;
    fread(buf_raw, fsize, 1, fp_raw);

	quick_sort(buf_raw, img_width*img_heigh);
	unsigned int i = 0;
	for(i = 0; i<(img_width*img_heigh/500);i++){
		max_avg += buf_raw[i];
	}
	max_avg /= i;
	
	printf("max-avg %d\n", max_avg, i);
	fclose(fp_raw);
	system("pause");
	return 0;
}