# K8sノードのメモリ枯渇時の振る舞い調査

ノードのメモリが枯渇した時に、ポッドの振る舞いについて調べた。
実験環境は、RAM 8Gi, CPU x4 のワーカーノードを準備して実施した。

```
$ kubectl get node
NAME      STATUS   ROLES           AGE   VERSION
master1   Ready    control-plane   44h   v1.29.9
node1     Ready    worker          44h   v1.29.9
node2     Ready    worker          44h   v1.29.9
```

## ノード２でメモリ不足の状態を作る

ノード１に、ポッドの配置を抑止するためテイントを設定
```
kubectl taint nodes node1 workload:NoSchedule
```

master1とnode1には、テイントがあり、ポッドが配置されないことを確認
```
kubectl get node -o json |jq -r ".items[]| .metadata.name, .spec.taints"
```

QoSクラスで、besteffort、boostable、guranteedの３種のポッドをデプロイする。
メモリの使用量は、この３種のポッドが十分治る量に調整しておく。 

```
kubectl apply -f deployment-besteffort.yaml
kubectl apply -f deployment-boostable.yaml
kubectl apply -f deployment-guranteed.yaml 
```

ポッドのQoSクラスを表示
```
kubectl get pod -o json |jq -r ".items[]| .metadata.name, .status.qosClass" | xargs -n2
```

ノード、ポッドのメモリ使用状態を確認、３分後に再度確認する
```
kubectl get pod -o wide && kubectl top node && kubectl top pod
```


ノード１のテイントを消して、ポッドがノード１へ配置されるようにする
```
kubectl taint nodes node1 workload:NoSchedule-
```

## Pod eviction を発動させる

デプロイメントのポッドを増やして、QoSクラスの挙動を確認する
```
kubectl patch deployment guranteed -p '{"spec":{"replicas":6}}'
```


メモリプレッシャーの状態を確認する
```
kubectl describe node node2
kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.conditions[1].type, .status.conditions[1].status" | xargs -n3
kubectl get pod -o wide && kubectl top node && kubectl top pod
```


クリーンナップ
```
kubectl delete -f deployment-besteffort.yaml
kubectl delete -f deployment-guranteed.yaml 
kubectl delete -f deployment-boostable.yaml
```

##　テストケース

- ケース１（デフォルト設定） 
設定を何もせず、デフォルト値で実行
  - systemReserved: N/A
  - kubeReserved: N/A
  - evictionHard: N/A

- ケース２
systemReservedに、メモリを設定
  - systemReserved: {memory: "800Mi"}
  - kubeReserved: N/A
  - evictionHard: N/A

- ケース３
kubeReservedに、メモリを設定
  - systemReserved: N/A
  - kubeReserved: {memory: "800Mi"}
  - evictionHard: N/A

- ケース4
evictionHardに、メモリを設定
  - systemReserved: N/A
  - kubeReserved: N/A
  - evictionHard: {memory: "800Mi"}

- ケース5
  systemReserved と kubeReserved　にそれぞれ　１Gづつ設定、合計　２Gが使用できない




### テスト結果



systemReserved と kubeReserved の設定が無い時
  QoSクラスの優先度で(BestEffort < Boostable < Guranteed) のポッドのコンテナから OOM-Killされて、ポッドが何度も再スタートする。 
  Podが他のノードへ退避されることは無い。

systemReserved と kubeReserved 設定した場合
  Podが他のノードへ退避されることは無い。
  ノードのAllocatableのメモリが減少して、スケジュールが抑止される。
  しかし、ノードセレクターなどが付いている場合、Allocatableのメモリを超えて、ポッドが配置される。
  さらに、超えると「OutOfmemory」状態のポッドが大量に発生して、そのポッドのコントローラーを消さないと削除できなくなる。（K8sのバグっぽい）

systemReserved と kubeReserved の設定が無く、evictionHard や evictionSoft を設定した場合
  OOM-Killedされず、他のノードへ退避される。ただし、退避されたポッドが ErrorやUnknown Statusで残ることがある。（K8sのバグっぽい）
  このケースはメモリ不足となっているノードから、他のノードへポッドが移動するので、ノードのメモリ不足対策として、もっとも有効だと思われる。

evictionHard と evictionSoft のどちらを設定するべきか？
  evictionSoftは、
    設定値がAllocatableのメモリの減算に使われない
    猶予時間を設定できるため、アプリにとって優しい。
    コンテナがOOMでKillされることを回避できる

  evictionHard
    設定値がAllocatableのメモリの減算に使われる
    猶予時間を設定できない。
    コンテナがOOMでKillされることを回避できる
 
 対策の提案
 メモリ不足でノードが応答不能になる対策として、SystemReserved と kubeReservedは、現状どおりとして、evictionSoftを設定するのが良いと考えれる。


言葉の整理
  - systemReserved: systemReserved is a set of ResourceName=ResourceQuantity (e.g. cpu=200m,memory=150G) pairs that describe resources reserved for non-kubernetes components. Currently only cpu and memory are supported. 
  - kubeReserved: kubeReserved is a set of ResourceName=ResourceQuantity (e.g. cpu=200m,memory=150G) pairs that describe resources reserved for kubernetes system components. Currently cpu, memory and local storage for root file system are supported.
  - OOM-Killは、Linuxカーネルが自身の保護のため、プロセスを強制終了する。 優先度を設定でき、kubeletはポッドのコンテナをQoSクラスに合わせて、優先度を設定している。










systemReserved
  cpu: "1"
  memory: "800Mi"
evictionHard:
  memory.available: "800Mi"
volumeStatsAggPeriod: 0s
kubeReserved:
  memory: "800Mi"

OOMが発生しているにも関わらず、メモリプレッシャーは、Falseとなるのは正しくない。どうしてか？

kubeletに以下を設定することで Evictionが実施された
config.yaml
evictionHard:
  memory.available: "800Mi"

Capacity:
  cpu:                4
  ephemeral-storage:  16400252Ki
  hugepages-1Gi:      0
  hugepages-2Mi:      0
  memory:             8150236Ki
  pods:               110
Allocatable:
  cpu:                4
  ephemeral-storage:  15114472219
  hugepages-1Gi:      0
  hugepages-2Mi:      0
  memory:             7228636Ki
  pods:               110


以下だけでは、Evictionが働かない
systemReserved:
  memory: "800Mi"

  