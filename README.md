# image_processing.c 操作マニュアル

## 概要

このプログラムは8bitグレースケールBMP画像に対して、二値化・平滑化・エッジ検出・ラプラシアン・ガウシアン・LoGフィルタなどの画像処理を行います。

---

## 使い方

```sh
image_processing.exe 入力BMPファイル名 出力BMPファイル名 処理番号 [パラメータ]
```

### 処理番号とパラメータ

| 処理番号 | 処理内容         | パラメータ例         |
|:--------:|:----------------|:---------------------|
| 1        | 二値化           | 閾値（例: 128）      |
| 2        | 平滑化           | 閾値（例: 128）                 |
| 3        | エッジ検出       | 閾値（例: 50）       |
| 4        | ラプラシアン     | 閾値（例: 30）       |
| 5        | ガウシアン       | σ（例: 1.0）         |
| 6        | LoGフィルタ      | σ（例: 1.0）         |

### 実行例

```sh
.\image_processing.exe input.bmp output.bmp 1 128
```
- input.bmp を閾値128で二値化し、output.bmpに保存

---

## 関数説明

### unsigned char** alloc_image(int width, int height)
- 画像データ用の2次元配列を動的に確保
- **引数**: width（画像幅）, height（画像高さ）
- **戻り値**: 画像データのポインタ（unsigned char**）

### void free_image(unsigned char **img, int height)
- 画像データのメモリを解放
- **引数**: img（画像データ）, height（画像高さ）

### void binarize(unsigned char **img, int width, int height, int threshold)
- 画像を閾値で二値化
- **引数**: img（画像データ）, width, height, threshold（しきい値）

### void averaging(unsigned char **image, int xSize, int ySize, int threshold)
- 平均化（平滑化）フィルタ
- **引数**: image（画像データ）, xSize（幅）, ySize（高さ）, threshold（未使用）

### void edgeDetectionFilter(unsigned char **img, int width, int height, int threshold)
- エッジ検出（横・縦フィルタ）
- **引数**: img（画像データ）, width, height, threshold（しきい値）

### void laplacianFilter(unsigned char **img, int width, int height, int threshold)
- ラプラシアンフィルタ（二次微分）
- **引数**: img（画像データ）, width, height, threshold（しきい値）

### void GaussianFilter(unsigned char **img, int width, int height, double sigma)
- ガウシアンフィルタ（平滑化）
- **引数**: img（画像データ）, width, height, sigma（標準偏差）

### void LoGFilter(unsigned char **img, int width, int height, double sigma)
- LoGフィルタ（エッジ検出）
- **引数**: img（画像データ）, width, height, sigma（標準偏差）

### unsigned char** read_bmp(const char *filename, int *width, int *height, BITMAPFILEHEADER *bfh, BITMAPINFOHEADER *bih)
- BMP画像ファイルの読み込み（8bitグレースケール専用）
- **引数**: filename（ファイル名）, width, height（画像サイズを格納するポインタ）, bfh, bih（ヘッダ情報を格納するポインタ）
- **戻り値**: 画像データのポインタ（unsigned char**）

### void write_bmp(const char *filename, unsigned char **img, int width, int height, BITMAPFILEHEADER bfh, BITMAPINFOHEADER bih)
- BMP画像ファイルの書き出し（8bitグレースケール専用）
- **引数**: filename（ファイル名）, img（画像データ）, width, height, bfh, bih（ヘッダ情報）

---

## 注意事項

- 入力画像は「8bitグレースケールBMP」のみ対応しています。
- パラメータが必要な処理では、必ず指定してください。
- 出力ファイルは既存の場合上書きされます。

---
