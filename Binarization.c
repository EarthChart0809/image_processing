#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// --------- 二値化処理 ---------
void binarize(unsigned char **img, int width, int height, int threshold) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            img[y][x] = (img[y][x] > threshold) ? 255 : 0;
        }
    }
}

// --------- BMP読み込み（8bit専用） ---------
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
        printf("使い方: %s 入力BMP 出力BMP 閾値\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];
    char *output_filename = argv[2];
    int threshold = atoi(argv[3]);

    int width, height;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;

    unsigned char **image = read_bmp(input_filename, &width, &height, &bfh, &bih);
    if (!image) return 1;

    // 二値化処理
    binarize(image, width, height, threshold);

    // BMP形式で保存
    write_bmp(output_filename, image, width, height, bfh, bih);

    free_image(image, height);
    return 0;
}
