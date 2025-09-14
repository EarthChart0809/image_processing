import subprocess
import tkinter as tk
from tkinter import filedialog, simpledialog
from PIL import Image, ImageTk

input_path = None  # グローバルで保持

def run_c_program(input_path, output_path, mode, param=None):
    cmd = ["./image_processing.exe", input_path, output_path, str(mode)]
    if param is not None:
        cmd.append(str(param))
    subprocess.run(cmd)

def open_file():
    global input_path
    input_path = filedialog.askopenfilename(filetypes=[("BMP files", "*.bmp")])
    if input_path:
        input_img = Image.open(input_path)
        tk_img = ImageTk.PhotoImage(input_img)
        input_label.config(image=tk_img)
        input_label.image = tk_img

def process_image():
    if not input_path:
        return
    output_path = "output.bmp"
    mode = mode_var.get()

    # モードごとに追加パラメータを要求
    param = None
    if mode == 1:  # 二値化 → threshold
        param = simpledialog.askinteger(
            "二値化", "閾値 (0-255)", minvalue=0, maxvalue=255)
    elif mode == 2:  # 平滑化 → threshold
        param = simpledialog.askinteger(
            "平滑化", "閾値 (0-255)", minvalue=0, maxvalue=255)
    elif mode == 3:  # エッジ検出 → threshold
        param = simpledialog.askinteger(
            "エッジ検出", "閾値 (0-255)", minvalue=0, maxvalue=255)
    elif mode == 4:  # ラプラシアン → threshold
        param = simpledialog.askinteger(
            "ラプラシアン", "閾値 (0-255)", minvalue=0, maxvalue=255)
    elif mode == 5:  # ガウシアン
        param = simpledialog.askfloat("ガウシアン", "σ の値を入力 (例: 1.0)")
    elif mode == 6:  # LoG
        param = simpledialog.askfloat("LoG", "σ の値を入力 (例: 1.0)")

    run_c_program(input_path, output_path, mode, param)

    # 出力画像を表示
    output_img = Image.open(output_path)
    tk_img = ImageTk.PhotoImage(output_img)
    output_label.config(image=tk_img)
    output_label.image = tk_img

# ---------------- GUI構築 ----------------
root = tk.Tk()
root.title("画像処理GUI")

frame = tk.Frame(root)
frame.pack()

btn_open = tk.Button(frame, text="画像を選択", command=open_file)
btn_open.grid(row=0, column=0, padx=5, pady=5)

mode_var = tk.IntVar(value=1)
tk.Radiobutton(frame, text="二値化", variable=mode_var,
                value=1).grid(row=1, column=0)
tk.Radiobutton(frame, text="平滑化", variable=mode_var,
                value=2).grid(row=1, column=1)
tk.Radiobutton(frame, text="エッジ検出", variable=mode_var,
                value=3).grid(row=1, column=2)
tk.Radiobutton(frame, text="ラプラシアン", variable=mode_var,
                value=4).grid(row=2, column=0)
tk.Radiobutton(frame, text="ガウシアン", variable=mode_var,
                value=5).grid(row=2, column=1)
tk.Radiobutton(frame, text="LoG", variable=mode_var,
                value=6).grid(row=2, column=2)

btn_process = tk.Button(frame, text="処理実行", command=process_image)
btn_process.grid(row=3, column=0, columnspan=3, pady=10)

input_label = tk.Label(root, text="入力画像")
input_label.pack(side="left", padx=10, pady=10)

output_label = tk.Label(root, text="出力画像")
output_label.pack(side="right", padx=10, pady=10)

root.mainloop()
