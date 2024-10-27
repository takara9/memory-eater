

ノード１にテイントを設定して、ポッドがスケジュリングされることを抑止
kubectl taint nodes node1 workload:NoSchedule

テイントが付いたノードを確認
kubectl get node -o json |jq -r ".items[]| .metadata.name, .spec.taints"

それぞれのQoSクラスでメモリ負荷を与える

kubectl apply -f deployment-besteffort.yaml
kubectl apply -f deployment-boostable.yaml
kubectl apply -f deployment-guranteed.yaml 
kubectl get pod -o wide && kubectl top node && kubectl top pod

ポッドのQoSクラスを表示
kubectl get pod -o json |jq -r ".items[]| .metadata.name, .status.qosClass" | xargs -n2

5分後に再度
kubectl get pod -o wide && kubectl top node && kubectl top pod


ノード１のテイントを消して、スケジュールを許可
kubectl taint nodes node1 workload:NoSchedule-

デプロイメントのポッドを増やして、QoSクラスの挙動を確認するう
kubectl patch deployment guranteed -p '{"spec":{"replicas":6}}'


メモリプレッシャーの状態を確認する
kubectl describe node node2
kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.conditions[1].type, .status.conditions[1].status" | xargs -n3
kubectl get pod -o wide && kubectl top node && kubectl top pod



クリーンナップ
kubectl delete -f deployment-besteffort.yaml
kubectl delete -f deployment-guranteed.yaml 
kubectl delete -f deployment-boostable.yaml



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

  