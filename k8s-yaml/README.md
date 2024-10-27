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


### テスト結果






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

  