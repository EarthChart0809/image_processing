#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// --------- BMPヘッダ構造体 ---------
#pragma pack(push, 1)
typedef struct {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

// --------- ユーティリティ関数 ---------
unsigned char** alloc_image(int width, int height) {
    unsigned char **img = (unsigned char **)malloc(height * sizeof(unsigned char *));
    for (int i = 0; i < height; i++) {
        img[i] = (unsigned char *)malloc(width * sizeof(unsigned char));
    }
    return img;
}

void free_image(unsigned char **img, int height) {
    for (int i = 0; i < height; i++) {
        free(img[i]);
    }
    free(img);
}

 // --------- エッジ検出フィルタ（横+縦） ---------
void laplacianFilter(Image *img, Image *temp){
  int i, j, k, l;
  int sumr, sumg, sumb;
  int weight;

    for(i=1; i<img->height-1; i++){
        for(j=1; j<img->width-1; j++){
        sumr = sumg = sumb = 0;
            for(k=-1; k<=1; k++){
                for(l=-1; l<=1; l++){
                    if(!k && !l) weight = -4;
                    else if(!k || !l) weight = 1;
                    else weight = 0;

                    sumr += weight * img->data[(i+k)*img->width + j + l].r;
                    sumg += weight * img->data[(i+k)*img->width + j + l].g;
                    sumb += weight * img->data[(i+k)*img->width + j + l].b;
            }
            temp->data[i*temp->width + j].r = abs((double)sumr);
            temp->data[i*temp->width + j].r = abs((double)sumr);
            temp->data[i*temp->width + j].g = abs((double)sumg);
            temp->data[i*temp->width + j].b = abs((double)sumb);
        }
        }
    }
}