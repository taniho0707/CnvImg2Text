# homography
## Build
OpenCVをインストールしてください．
```bash
make
```
をすることで，実行ファイルmainが生成されます．

## How to use
./main srcImage dstImage LUx LUy LDx LDy RDx RDy RUx RUy

1. 座標をGUIで入力したい場合は次のように起動してください．
./main srcImage dstImage 0 0 0 0 0 0 0 0
画面が開くので，「左上，左下，右下，右上」の順に1回ずつクリックし，最後に「Enter」を押すと完了します．
2. 座標を指定する場合は，それぞれ適切なパラメータを指定してください．
LUnは左上，LDnは左下，RDnは右下，RUnは右上です．
3. 座標の自動認識にはまだ対応していません．

例：./main in.jpg out.jpg 20 20 20 500 680 550 600 20
