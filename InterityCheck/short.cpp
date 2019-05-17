#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>

#define LINE_MAX 1024

char *config_fpath = "config.ini";

struct imiFrame
{
    uint16_t *data;
    uint32_t size;

    uint32_t width;
    uint32_t height;

    uint32_t valid_pixels;
    uint32_t total_pixels;

    //Ax+By+Cz=D
    float fit_param_A;
    float fit_param_B;
    float fit_param_C;
    float fit_param_D;
};


bool get_file_path(char *fpath)
{
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Data Files(*.data)\0*.data\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = fpath;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.hInstance = (HMODULE)GetCurrentProcess();

    return GetOpenFileNameA(&ofn);
}

char *get_ini_key_string(char *filename, char *title, char *key)   
{   
    FILE *fp;   
    char line[LINE_MAX];
    static char tmpstr[LINE_MAX];

    int rtnval;
    int i = 0;
    bool find = false;

    char *tmp;

    fp = fopen(filename, "r");
    assert(fp);

    while(!feof(fp)) {   
        rtnval = fgetc(fp);   
        if(rtnval == EOF)
            break; 
        else
            line[i++] = rtnval;

        if(rtnval == '\n') {   
#ifndef WIN32  
            i--;  
#endif    
            line[--i] = '\0';
            i = 0;
            tmp = strchr(line, '=');   
  
            if(( tmp != NULL ) && (find == true) ) {   
                if(strstr(line, key) != NULL) {
                    if ('#' == line[0])
                        ;//do nothing
                    else {
                        strcpy(tmpstr, tmp+1);
                        fclose(fp);
                        return tmpstr;
                    }
                }
            } else {
                strcpy(tmpstr, "[");   
                strcat(tmpstr, title);   
                strcat(tmpstr, "]");  
                if(strncmp(tmpstr, line, strlen(tmpstr)) == 0)
                    find = true;
            }  
        }
    }  
    fclose(fp);   
    return "";   
}  
  
int get_ini_key_int(char *filename, char *title, char *key)  
{
    return atoi(get_ini_key_string(filename, title, key));
}

void fill_frame_from_fp(struct imiFrame *frame, FILE *fp, size_t offset)
{
    fseek(fp, offset, SEEK_SET);
    fread(frame->data, frame->size, 1, fp);
}

bool is_valid_point(struct imiFrame *frame, int x, int y)
{
    if (frame->data[x + y * frame->width] != 0)
        return true;
    else
        return false;
}

uint16_t get_depth_data(struct imiFrame *frame, int x, int y)
{
    return frame->data[x + y * frame->width];
}

float get_depth_fit(struct imiFrame *frame, int x, int y)
{
    //Ax+By+Cz=D
    //so z = (D-Ax-By)/C
    float A = frame->fit_param_A;
    float B = frame->fit_param_B;
    float C = frame->fit_param_C;
    float D = frame->fit_param_D;

    return ((D - A * x - B * y) / C);
}

bool is_bad_point(struct imiFrame *frame, int x, int y, float threshold)
{
    //you must check (x,y) is a valid point first
    float depth_measure = get_depth_data(frame, x, y);
    float depth_fit = get_depth_fit(frame, x, y);

    if (abs(depth_measure - depth_fit) >= threshold)
        return true;
    else
        return false;
}

static void get_valid_pixels(struct imiFrame *frame)
{
    uint32_t v = 0;
    for (uint32_t x = 0; x < frame->width; x++) {
        for (uint32_t y = 0; y < frame->height; y++) {
            if (is_valid_point(frame, x, y)) {
                v++;
            }
        }
    }
    frame->valid_pixels = v;

    return;
}

float calc_plane_integrity(struct imiFrame *frame)
{
    float percent = (float)(frame->valid_pixels) / (frame->total_pixels) * 100;

    return percent;
}
/*
float calc_black_cols_percent(struct imiFrame *frame)
{
    //total_rows = image_heigh
    //total_cols = image_width
    uint64_t total_cols = frame->width;
    uint32_t black_cols = 0;

    uint32_t col_pixels = frame->height;

    for (uint32_t x = 0; x < frame->width; x++) {
        uint32_t black_pixels = 0;
        for (uint32_t y = 0; y< frame->height; y++) {
            if (is_valid_point(frame, x, y)) {
                //do nothing
            } else {
                black_pixels++;
            }
        }

        if((float)black_pixels / col_pixels > 0.8) {
            black_cols++;
        }
    }

    return (float)black_cols / total_cols * 100;
}
*/

float calc_avg_depth(struct imiFrame *frame)
{
    uint64_t sum = 0;
    float avg = 0;

    for (uint32_t x = 0; x < frame->width; x++) {
        for (uint32_t y = 0; y< frame->height; y++) {
            if (is_valid_point(frame, x, y)) {
                sum += get_depth_data(frame, x, y);
            }
        }
    }

    avg = (float)sum / frame->valid_pixels;
    return avg;
}

static void cv_fit_plane(struct imiFrame *frame)
{
    CvMat *points = cvCreateMat(frame->valid_pixels, 3, CV_32FC1);

    int i = 0;
    for (uint32_t x = 0; x < frame->width; x++) {
        for (uint32_t y = 0; y< frame->height; y++) {
            if (is_valid_point(frame, x, y)) {
                points->data.fl[i * 3 + 0] = (float)x;
                points->data.fl[i * 3 + 1] = (float)y;
                points->data.fl[i * 3 + 2] = (float)get_depth_data(frame, x, y);

                i++;
            }
        }
    }

    // Estimate geometric centroid.  
    int nrows = points->rows;
    int ncols = points->cols;
    int type = points->type;

    CvMat* centroid = cvCreateMat(1, ncols, type);
    cvSet(centroid, cvScalar(0));

    for (int c = 0; c<ncols; c++) {
        for (int r = 0; r < nrows; r++)
        {
            centroid->data.fl[c] += points->data.fl[ncols*r + c];
        }
        centroid->data.fl[c] /= nrows;
    }

    // Subtract geometric centroid from each point.  
    CvMat* points2 = cvCreateMat(nrows, ncols, type);
    for (int r = 0; r<nrows; r++)
        for (int c = 0; c<ncols; c++)
            points2->data.fl[ncols*r + c] = points->data.fl[ncols*r + c] - centroid->data.fl[c];

    // Evaluate SVD of covariance matrix.  
    CvMat* A = cvCreateMat(ncols, ncols, type);
    CvMat* W = cvCreateMat(ncols, ncols, type);
    CvMat* V = cvCreateMat(ncols, ncols, type);
    cvGEMM(points2, points, 1, NULL, 0, A, CV_GEMM_A_T);
    cvSVD(A, W, NULL, V, CV_SVD_V_T);

    // Assign plane coefficients by singular vector corresponding to smallest singular value.  
    float plane[4] = { 0 };
    plane[ncols] = 0;
    for (int c = 0; c<ncols; c++) {
        plane[c] = V->data.fl[ncols*(ncols - 1) + c];
        plane[ncols] += plane[c] * centroid->data.fl[c];
    }

    frame->fit_param_A = plane[0];
    frame->fit_param_B = plane[1];
    frame->fit_param_C = plane[2];
    frame->fit_param_D = plane[3];

    // Release allocated resources.  
    cvReleaseMat(&points);
    cvReleaseMat(&centroid);
    cvReleaseMat(&points2);
    cvReleaseMat(&A);
    cvReleaseMat(&W);
    cvReleaseMat(&V);

    return;
}

float calc_badpoint_rate(struct imiFrame *frame, float threshold)
{

    uint32_t bad_pixels = 0;
    for (uint32_t x = 0; x < frame->width; x++) {
        for (uint32_t y = 0; y< frame->height; y++) {
            if (is_valid_point(frame, x, y)) {
                if (is_bad_point(frame, x, y, threshold)) {
                    bad_pixels++;
                }
            }
        }
    }

    return (float)(bad_pixels) / frame->valid_pixels * 100;

    return 0;
}

void clac_badpoint_class(struct imiFrame *frame, unsigned int bpclass[6])
{
    for (uint32_t x = 0; x < frame->width; x++) {
        for (uint32_t y = 0; y< frame->height; y++) {
            if (is_valid_point(frame, x, y)) {
                float depth_measure = get_depth_data(frame, x, y);
                float depth_fit = get_depth_fit(frame, x, y);
                float dvalue = 	abs(depth_measure - depth_fit);

                if ( dvalue < 1.0 )
                    bpclass[0]++;
                else if( dvalue >= 1.0 && dvalue < 3.0)
                    bpclass[1]++;
                else if( dvalue >= 3.0 && dvalue < 5.0)
                    bpclass[2]++;
                else if( dvalue >= 5.0 && dvalue < 7.0)
                    bpclass[3]++;
                else if( dvalue >= 7.0 && dvalue < 10.0)
                    bpclass[4]++;
                else if( dvalue >= 10.0)
                    bpclass[5]++;
            }
        }
    }
}

int main(int argc, void **argv)
{
    struct imiFrame frame;
	struct imiFrame cut_frame;
	
    frame.width = get_ini_key_int(config_fpath, "Settings", "ImageWidth");
    frame.height = get_ini_key_int(config_fpath, "Settings", "ImageHeight");
	float bp_threadhold = (float)get_ini_key_int(config_fpath, "Settings", "ShortThreadhold");

    frame.total_pixels = frame.width * frame.height;
    frame.size = sizeof(unsigned short) * frame.width * frame.height;
    frame.data = (unsigned short *)malloc(frame.size);

    char fpath[MAX_PATH] = { 0 };
    if(argc == 1) {
        if (get_file_path(fpath) == false)
            return -1;
    } else {
        strncpy(fpath, (char *)argv[1], sizeof(fpath));
    }

    int frame_num = 0;
    int frame_total = 0;

    FILE *fp = fopen(fpath, "rb+");
    if (fp == NULL) {
        printf("%s, can not open file", fpath);
        exit(0);
    } else {
        fseek(fp, 0L, SEEK_END);
        frame_total  = ftell(fp) / frame.size;
    }

    float avg_interity = 0;
    float avg_avgdepth = 0;
    float avg_bp_rate = 0;
    unsigned int avg_bpclass[6] = {0};   

    for (; frame_num < frame_total; frame_num++) {
        fill_frame_from_fp(&frame, fp, frame_num * frame.size);
        get_valid_pixels(&frame);
        cv_fit_plane(&frame);

        /*calc value*/
        float interity = calc_plane_integrity(&frame);
        float avgdepth = calc_avg_depth(&frame);
        float bp_rate = calc_badpoint_rate(&frame, bp_threadhold);

        avg_interity += interity;
        avg_avgdepth += avgdepth;
        avg_bp_rate += bp_rate;

        unsigned int bpclass[6] = {0};
        clac_badpoint_class(&frame, bpclass);
        for(int i=0; i<6; i++)
            avg_bpclass[i] += bpclass[i];

        printf("%03d\t%.2f\t%.2f\t%.2f\t",
               frame_num, interity, avgdepth, bp_rate);
        printf("%d\t%d\t%d\t%d\t%d\t%d\n", bpclass[0], bpclass[1], bpclass[2], bpclass[3], bpclass[4], bpclass[5]);

    }

    avg_interity /= frame_total;
    avg_avgdepth /= frame_total;
    avg_bp_rate /= frame_total;
    for(int i=0; i<6; i++)
        avg_bpclass[i] /= frame_total;

    printf("avg\t%.2f\t%.2f\t%.2f\t",
           avg_interity, avg_avgdepth, avg_bp_rate);
    printf("%d\t%d\t%d\t%d\t%d\t%d\n", avg_bpclass[0], avg_bpclass[1], avg_bpclass[2], avg_bpclass[3], avg_bpclass[4], avg_bpclass[5]);

    fclose(fp);
    return 0;
} 