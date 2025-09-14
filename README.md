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


# GUI.py 操作マニュアル

## 概要

`GUI.py` は画像処理Cプログラム（image_processing.exe）をGUIから操作できるPythonスクリプトです。
BMP画像の選択、処理モードの選択、パラメータ入力、結果画像の表示が可能です。

---

## 使い方

1. Pythonで `GUI.py` を実行します。
2. 「画像を選択」ボタンでBMP画像を選択します。
3. 処理モード（ラジオボタン）を選択します。
4. 「処理実行」ボタンを押すと、必要なパラメータ（閾値やσなど）がダイアログで表示されます。
5. 画像処理が実行され、結果画像が右側に表示されます。

### 必要な環境
- Python 3.x
- Pillow（PIL）
- tkinter
- All_image_processing.exe（Cコンパイル済み）

---

## 主な関数

### run_c_program(input_path, output_path, mode, param=None)
- Cプログラム（All_image_processing.exe）をサブプロセスで実行
- **引数**:
	- input_path: 入力画像ファイルパス
	- output_path: 出力画像ファイルパス
	- mode: 処理番号（1〜6）
	- param: 閾値やσなどの追加パラメータ（不要な場合はNone）
- **戻り値**: なし（画像処理後、output_pathに画像が保存される）

### open_file()
- ファイルダイアログで画像ファイルを選択し、GUIに表示
- **引数**: なし
- **戻り値**: なし（グローバル変数 input_path を更新）

### process_image()
- 選択画像・モード・パラメータを取得し、run_c_programで処理
- 処理後、出力画像をGUIに表示
- **引数**: なし
- **戻り値**: なし

---

## 操作手順

1. 画像を選択
2. 処理モードを選択
3. パラメータ入力（必要な場合のみ）
4. 処理実行
5. 結果画像を確認

---

## 注意事項
- 入力画像は8bitグレースケールBMPのみ対応
- All_image_processing.exeが同じフォルダに必要
- Python環境にPillow, tkinterがインストールされていること

---

# 参考資料

- 物理のかぎしっぽ「二値化してみる」https://hooktail.org/computer/index.php?%A3%B2%C3%CD%B2%BD%A4%F2%A4%B7%A4%C6%A4%DF%A4%EB

- Qiita「PGM画像の平滑化」https://qiita.com/Santonn/items/204df5fea5b305939b19

- teratail「C言語　ガウシアンフィルターのソースコードについて」https://teratail.com/questions/255786

- Qiita「画像処理 アルゴリズムから画像処理を知ろう -フィルタ処理編-」https://qiita.com/T_ksy/items/abed6aad3d5d841e0d6d

- 物理のかぎしっぽ「Laplacianフィルタでエッジ抽出する」https://hooktail.org/computer/index.php?Laplacian%A5%D5%A5%A3%A5%EB%A5%BF%A4%C7%A5%A8%A5%C3%A5%B8%C3%EA%BD%D0%A4%B9%A4%EB

- Qiita「LoGフィルタ」https://qiita.com/jajagacchi/items/55c085cb221e822bdb6e