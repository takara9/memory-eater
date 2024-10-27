# memory-eater
メモリ消費量に関するテスト用コンテナ


## 実行方法

第一パラメータ: 初期メモリ使用量 x100Mi
第二パラメータ: 目標メモリ使用量 x100Mi
第三パラメータ: インターバル時間 秒

初期サイズ 100Mi から 100Miの刻みで、5秒の間隔時間をおいて、1000Mi までメモリ消費を増やします。
最終値に達した後は、状態を維持して、SIGTERMで終了します。

```
docker run -it ghcr.io/takara9/mem-eater:0.2 1 10 5
```


## ビルド方法

```
docker login
docker build -t ghcr.io/takara9/mem-eater:0.2 .
docker push ghcr.io/takara9/mem-eater:0.2
```

