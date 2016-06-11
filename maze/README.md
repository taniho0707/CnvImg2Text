# maze
## Build
OpenCVをインストールしてください．
```bash
make
```
をすることで，実行ファイルmainが生成されます．

## How to use
./main srcImage dstImage mazeData x y

x, yはそれぞれ迷路のマス数です．
homographyで出力されたファイル名をsrcImageとします．
途中出力画像をdstImageに出力し，テキスト形式の最終出力ファイルをmazeDataに出力します．

例：./main in.jpg out.jpg out.txt 16 8

