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

// --------- 二値化処理 ---------
void binarize(unsigned char **img, int width, int height, int threshold) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            img[y][x] = (img[y][x] > threshold) ? 255 : 0;
        }
    }
}

// --------- 平均化フィルタ（平滑化） ---------
void averaging(unsigned char **image, int xSize, int ySize, int threshold) {
/*
    ！画像の画素は，int型2次元配列image[ySize][xSize]で参照できるものとする．
    */    
    int temp[xSize];

    for (int y = 0; y < ySize; ++y)
    {
        int saveresult[xSize];

        for (int x = 0; x < xSize; ++x)
        {
            int sum = image[y][x];
            int numOfNeighbors = 1;

            for (int i = 0; i < 8; ++i)
            {
                //周囲の画素を左下から反時計回りにチェック
                int neighbors[8][2] = {
                    {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
                int nextRow = y + neighbors[i][0];
                int nextCol = x + neighbors[i][1];

                //参照する値が，画素の座標として取りうる値かチェック
                if (nextRow >= 0 && nextRow < ySize && nextCol >= 0 && nextCol < xSize)
                {
                    sum += image[nextRow][nextCol];
                    numOfNeighbors++;
                }
            }
            saveresult[x] = sum / numOfNeighbors;
        }

        int lastLineTrue = 0;
        
        //1行目の計算が終わったときのみスキップされる
        if (y > 0)
        {
            //1行手前に, tempに格納しておいた計算結果を代入
            for (int x = 0; x < xSize; ++x)
            {
                image[y - 1][x] = temp[x];
            }

            //最終行は計算結果をすぐ代入
            if (y == ySize-1)
            {
                for (int x = 0; x < xSize; ++x)
                {
                    image[y][x] = saveresult[x];
                    lastLineTrue = 1;
                }
            }
        }

        //最終行はこの処理は要らないためスキップ
        if (!lastLineTrue)
        {
            for (int x = 0; x < xSize; ++x)
            {
                temp[x] = saveresult[x];
            }
        }
    }
}

// --------- エッジ検出フィルタ（横+縦） ---------
void edgeDetectionFilter(unsigned char **img, int width, int height, int threshold) {
    // 出力用バッファ確保
    unsigned char **out = alloc_image(width, height);

    // 横・縦のフィルタ
    int fx[3][3] = {
        {0,  0,  0},
        {1,  0, -1},
        {0,  0,  0}
    };
    int fy[3][3] = {
        {0,  1,  0},
        {0,  0,  0},
        {0, -1,  0}
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gx = 0, gy = 0;

            for (int yy = -1; yy <= 1; yy++) {
                for (int xx = -1; xx <= 1; xx++) {
                    int px = x + xx;
                    int py = y + yy;
                    if (px < 0) px = 0;
                    if (py < 0) py = 0;
                    if (px >= width)  px = width - 1;
                    if (py >= height) py = height - 1;

                    gx += img[py][px] * fx[yy + 1][xx + 1];
                    gy += img[py][px] * fy[yy + 1][xx + 1];
                }
            }

            int mag = (int)(sqrt(gx * gx + gy * gy));
            if (mag > 255) mag = 255;
            out[y][x] = (mag >= threshold) ? 255 : 0;
        }
    }

    // 結果をコピー
    for (int y = 0; y < height; y++) {
        memcpy(img[y], out[y], width);
    }

    free_image(out, height);
}

// --------- ラプラシアンフィルタ（二次微分） ---------
void laplacianFilter(unsigned char **img, int width, int height, int threshold) {
    // 出力用バッファ確保
    unsigned char **out = alloc_image(width, height);

    // ラプラシアンカーネル（4近傍）
    int kernel[3][3] = {
        { 0,  1,  0},
        { 1, -4,  1},
        { 0,  1,  0}
    };

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sum = 0;

            // 3x3畳み込み
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    sum += img[y + ky][x + kx] * kernel[ky + 1][kx + 1];
                }
            }

            sum = abs(sum); // 負の値を正に

            if (sum > 255) sum = 255;
            out[y][x] = (sum >= threshold) ? 255 : 0;
        }
    }

    // 結果を元の画像にコピー
    for (int y = 0; y < height; y++) {
        memcpy(img[y], out[y], width);
    }

    free_image(out, height);
}

// --------- ガウシアンフィルタ（平滑化） ---------
void GaussianFilter(unsigned char **img, int width, int height, double sigma)
{
    int size = (int)(6 * sigma + 1);   // カーネルサイズ（6σをカバー）
    if (size % 2 == 0) size++;         // 奇数にする
    int half = size / 2;

    // 出力バッファ
    unsigned char **out = alloc_image(width, height);

    double PI = 3.141592653589793;
    double gauss_const = 1.0 / (2.0 * PI * sigma * sigma);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double sum = 0.0;
            double sum_weight = 0.0;

            // カーネル適用
            for (int ky = -half; ky <= half; ky++) {
                for (int kx = -half; kx <= half; kx++) {
                    int iy = y + ky;
                    int ix = x + kx;

                    if (iy >= 0 && iy < height && ix >= 0 && ix < width) {
                        double weight = gauss_const *
                            exp(-((kx * kx + ky * ky) / (2 * sigma * sigma)));

                        sum += img[iy][ix] * weight;
                        sum_weight += weight;
                    }
                }
            }

            // 正規化
            out[y][x] = (unsigned char)(sum / sum_weight);
        }
    }

    // 結果を元の画像にコピー
    for (int y = 0; y < height; y++) {
        memcpy(img[y], out[y], width);
    }

    free_image(out, height);
}

// --------- LoGフィルタ（エッジ検出） ---------
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
    if (argc < 4) {
        printf("使い方: %s 入力BMP 出力BMP 処理番号 [パラメータ]\n", argv[0]);
        printf("処理番号: 1=二値化  2=平滑化  3=エッジ検出  4=ラプラシアン  5=ガウシアン  6=LoG\n");
        printf("例: %s input.bmp output.bmp 1 128\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];
    char *output_filename = argv[2];
    int mode = atoi(argv[3]);   // どの処理をするか

    int width, height;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;

    unsigned char **image = read_bmp(input_filename, &width, &height, &bfh, &bih);
    if (!image) return 1;

    switch (mode) {
        case 1: { // 二値化
            if (argc < 5) { printf("二値化には閾値が必要です\n"); return 1; }
            int threshold = atoi(argv[4]);
            binarize(image, width, height, threshold);
            break;
        }
        case 2: { // 平滑化
            if (argc < 5) { printf("平滑化には閾値が必要です\n"); return 1; }
            int threshold = atoi(argv[4]);
            averaging(image, width, height, threshold);
            break;
        }
        case 3: { // エッジ検出
            if (argc < 5) { printf("エッジ検出には閾値が必要です\n"); return 1; }
            int threshold = atoi(argv[4]);
            edgeDetectionFilter(image, width, height, threshold);
            break;
        }
        case 4: { // ラプラシアン
            if (argc < 5) { printf("ラプラシアンには閾値が必要です\n"); return 1; }
            int threshold = atoi(argv[4]);
            laplacianFilter(image, width, height, threshold);
            break;
        }
        case 5: { // ガウシアン
            if (argc < 5) { printf("ガウシアンにはσが必要です\n"); return 1; }
            double sigma = atof(argv[4]);
            GaussianFilter(image, width, height, sigma);
            break;
        }
        case 6: { // LoG
            if (argc < 5) { printf("LoGにはσが必要です\n"); return 1; }
            double sigma = atof(argv[4]);
            LoGFilter(image, width, height, sigma);
            break;
        }
        default:
            printf("不正な処理番号です\n");
            free_image(image, height);
            return 1;
    }

    // BMP形式で保存
    write_bmp(output_filename, image, width, height, bfh, bih);

    free_image(image, height);
    return 0;
}
