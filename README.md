# memory-eater
メモリ消費量に関するテスト用コンテナ


## 実行方法

初期サイズ 100Mi から 100Miの刻みで、5秒の間隔時間をおいて、1000Mi までメモリ消費を増やします。

```
docker run -it ghcr.io/takara9/mem-eater:0.1 1 10 5
```

最終値に達した後は、状態を維持して、SIGTERMで終了します。


## ビルド方法

```
docker build -t ghcr.io/takara9/mem-eater:0.2 .
docker push
```

