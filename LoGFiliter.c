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

void LoGFilter(unsigned char **img, int width, int height, double sigma) {
    int ksize = (int)(6 * sigma + 1); // カーネルサイズ（経験的に6σ程度）
    if (ksize % 2 == 0) ksize++;      // 奇数にする
    int half = ksize / 2;

    // カーネル生成
    double **kernel = (double **)malloc(ksize * sizeof(double *));
    for (int i = 0; i < ksize; i++) {
        kernel[i] = (double *)malloc(ksize * sizeof(double));
    }

    double sum = 0.0;
    double pi = 3.141592653589793;
    for (int y = -half; y <= half; y++) {
        for (int x = -half; x <= half; x++) {
            double r2 = x*x + y*y;
            double val = ((r2 - 2*sigma*sigma) / (2*pi*pow(sigma,6))) * exp(-r2 / (2*sigma*sigma));
            kernel[y+half][x+half] = val;
            sum += val;
        }
    }

    // 出力バッファ
    unsigned char **out = alloc_image(width, height);

    // 畳み込み演算
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double acc = 0.0;
            for (int ky = -half; ky <= half; ky++) {
                for (int kx = -half; kx <= half; kx++) {
                    int yy = j + ky;
                    int xx = i + kx;
                    if (yy >= 0 && yy < height && xx >= 0 && xx < width) {
                        acc += img[yy][xx] * kernel[ky+half][kx+half];
                    }
                }
            }
            int val = (int)fabs(acc); // LoGは負の値も出るので絶対値を取る
            if (val > 255) val = 255;
            out[j][i] = (unsigned char)val;
        }
    }

    // 結果をimgにコピー
    for (int y = 0; y < height; y++) {
        memcpy(img[y], out[y], width);
    }

    free_image(out, height);
    for (int i = 0; i < ksize; i++) free(kernel[i]);
    free(kernel);
}


unsigned char** read_bmp(const char *filename, int *width, int *height,
                         BITMAPFILEHEADER *bfh, BITMAPINFOHEADER *bih) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("ファイルを開けません: %s\n", filename);
        return NULL;
    }

    fread(bfh, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(bih, sizeof(BITMAPINFOHEADER), 1, fp);

    if (bfh->bfType != 0x4D42) { // "BM"
        fclose(fp);
        printf("BMP形式ではありません\n");
        return NULL;
    }

    if (bih->biBitCount != 8) {
        fclose(fp);
        printf("このプログラムは8bitグレースケールBMPのみ対応しています\n");
        return NULL;
    }

    *width = bih->biWidth;
    *height = bih->biHeight;

    // 出力用にカラーパレットのサイズを保持しておく
    int palette_size = (bfh->bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER));
    unsigned char *palette = (unsigned char *)malloc(palette_size);
    fread(palette, 1, palette_size, fp);

    // メモリ確保
    unsigned char **img = alloc_image(*width, *height);

    // BMPの1行は4バイト境界に揃えられている
    int row_padded = ((*width + 3) & ~3);
    unsigned char *row = (unsigned char *)malloc(row_padded);

    for (int y = 0; y < *height; y++) {
        fread(row, 1, row_padded, fp);
        for (int x = 0; x < *width; x++) {
            img[*height - 1 - y][x] = row[x]; // BMPは上下逆
        }
    }

    free(row);
    free(palette); // 出力時は同じパレットを再生成するので不要
    fclose(fp);
    return img;
}

// --------- BMP書き込み（8bit専用） ---------
void write_bmp(const char *filename, unsigned char **img, int width, int height,
               BITMAPFILEHEADER bfh, BITMAPINFOHEADER bih) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("出力ファイルを開けません: %s\n", filename);
        return;
    }

    int row_padded = ((width + 3) & ~3);
    bih.biSizeImage = row_padded * height;
    bfh.bfSize = bfh.bfOffBits + bih.biSizeImage;

    // 書き込み
    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);

    // グレースケールのカラーパレットを生成（0〜255）
    for (int i = 0; i < 256; i++) {
        unsigned char pal[4] = {i, i, i, 0};
        fwrite(pal, 1, 4, fp);
    }

    // ピクセルデータ
    unsigned char *row = (unsigned char *)malloc(row_padded);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            row[x] = img[height - 1 - y][x]; // BMPは上下逆
        }
        // パディング部分を0で埋める
        for (int x = width; x < row_padded; x++) row[x] = 0;
        fwrite(row, 1, row_padded, fp);
    }
    free(row);

    fclose(fp);
}


// --------- メイン関数 ---------
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("使い方: %s 入力BMP 出力BMP sigma\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];
    char *output_filename = argv[2];
    double sigma = atof(argv[3]);   // ← doubleに変換

    int width, height;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;

    unsigned char **image = read_bmp(input_filename, &width, &height, &bfh, &bih);
    if (!image) return 1;

    // ガウシアン平滑化
    LoGFilter(image, width, height, sigma);

    // BMP形式で保存
    write_bmp(output_filename, image, width, height, bfh, bih);

    free_image(image, height);
    return 0;
}

