
## kubeletのコンフィグに設定を追加

```
root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml 
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-27 07:10:26.018265895 +0000
@@ -43,3 +43,5 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+systemReserved:
+  memory: "800Mi"
+  
```

kubeletを再起動
```
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Sun 2024-10-27 07:11:57 UTC; 4s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 14195 (kubelet)
      Tasks: 14 (limit: 9445)
     Memory: 49.7M
     CGroup: /system.slice/kubelet.service
             └─14195 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml --container-runtime-endpoint=unix:///var/run/co>

Oct 27 07:11:58 node2 kubelet[14195]: I1027 07:11:58.736333   14195 reconciler_common.go:258] "operationExecutor.VerifyControllerAttachedVolume started for volume \"etc-cni-netd\" (UniqueName: \"kubernetes.io/host-path/7436982b>
```

初期状態を確認
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
No resources found in default namespace.
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   98m          4%     1092Mi          28%       
node1     42m          1%     760Mi           9%        
node2     89m          2%     415Mi           5%        
No resources found in default namespace.
```

ノードの割り当て可能なメモリ量の確認
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl describe node node2
Name:               node2
Roles:              worker
Labels:             beta.kubernetes.io/arch=amd64
<中略>
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
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ bc
bc 1.07.1
Copyright 1991-1994, 1997, 1998, 2000, 2004, 2006, 2008, 2012-2017 Free Software Foundation, Inc.
This is free software with ABSOLUTELY NO WARRANTY.
For details type `warranty'. 
scale=3
8150236-7228636
921600
```

テイントを設定して、node2をメモリ不足状態に追い込む準備
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule
node/node1 tainted
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .spec.taints"
master1
[
  {
    "effect": "NoSchedule",
    "key": "node-role.kubernetes.io/control-plane"
  }
]
node1
[
  {
    "effect": "NoSchedule",
    "key": "workload"
  }
]
node2
null
```


ポッドをデプロイして、メモリ負荷を与える
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-besteffort.yaml
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-boostable.yaml
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-guranteed.yaml 

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o json |jq -r ".items[]| .metadata.name, .status.qosClass" | xargs -n2
besteffort-657f6c859c-g6xrg BestEffort
boostable-7f657bc6d7-wctsw Burstable
guranteed-69875c65d6-q5qjm Guaranteed
```


ノード、ポッドのメモリ使用状態を確認
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-g6xrg   1/1     Running   0          73s   10.0.0.153   node2   <none>           <none>
boostable-7f657bc6d7-wctsw    1/1     Running   0          73s   10.0.0.188   node2   <none>           <none>
guranteed-69875c65d6-q5qjm    1/1     Running   0          72s   10.0.0.106   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   83m          4%     1088Mi          28%       
node1     37m          0%     760Mi           9%        
node2     73m          1%     3504Mi          49%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-g6xrg   0m           1002Mi          
boostable-7f657bc6d7-wctsw    0m           1002Mi          
guranteed-69875c65d6-q5qjm    30m          1002Mi         
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted
```

```
kubectl patch deployment guranteed -p '{"spec":{"replicas":6}}'
```

メモリの使用量が範囲内のうちは、何事もなく、続行
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-g6xrg   1/1     Running   0          2m26s   10.0.0.153   node2   <none>           <none>
boostable-7f657bc6d7-wctsw    1/1     Running   0          2m26s   10.0.0.188   node2   <none>           <none>
guranteed-69875c65d6-576pl    1/1     Running   0          15s     10.0.0.190   node2   <none>           <none>
guranteed-69875c65d6-cr5s4    1/1     Running   0          15s     10.0.0.100   node2   <none>           <none>
guranteed-69875c65d6-gb5zj    1/1     Running   0          15s     10.0.0.73    node2   <none>           <none>
guranteed-69875c65d6-hp88k    1/1     Running   0          15s     10.0.0.177   node2   <none>           <none>
guranteed-69875c65d6-q5qjm    1/1     Running   0          2m25s   10.0.0.106   node2   <none>           <none>
guranteed-69875c65d6-x7n7z    1/1     Running   0          15s     10.0.0.19    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   93m          4%     1088Mi          28%       
node1     38m          0%     760Mi           9%        
node2     247m         6%     4461Mi          63%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-g6xrg   0m           1002Mi          
boostable-7f657bc6d7-wctsw    0m           1002Mi          
guranteed-69875c65d6-576pl    58m          188Mi           
guranteed-69875c65d6-cr5s4    53m          195Mi           
guranteed-69875c65d6-gb5zj    53m          186Mi           
guranteed-69875c65d6-hp88k    50m          201Mi           
guranteed-69875c65d6-q5qjm    0m           1002Mi          
guranteed-69875c65d6-x7n7z    58m          189Mi           
```

ノード２のメモリが不足して、OOMが発生したが、ノード２から退避されず、OOMを繰り返している状態
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS      AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-g6xrg   1/1     Running   1 (19s ago)   3m24s   10.0.0.153   node2   <none>           <none>
boostable-7f657bc6d7-wctsw    1/1     Running   1 (12s ago)   3m24s   10.0.0.188   node2   <none>           <none>
guranteed-69875c65d6-576pl    1/1     Running   0             73s     10.0.0.190   node2   <none>           <none>
guranteed-69875c65d6-cr5s4    1/1     Running   0             73s     10.0.0.100   node2   <none>           <none>
guranteed-69875c65d6-gb5zj    1/1     Running   0             73s     10.0.0.73    node2   <none>           <none>
guranteed-69875c65d6-hp88k    1/1     Running   0             73s     10.0.0.177   node2   <none>           <none>
guranteed-69875c65d6-q5qjm    1/1     Running   0             3m23s   10.0.0.106   node2   <none>           <none>
guranteed-69875c65d6-x7n7z    1/1     Running   0             73s     10.0.0.19    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   88m          4%     1088Mi          28%       
node1     39m          0%     760Mi           9%        
node2     228m         5%     7044Mi          99%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-g6xrg   10m          401Mi           
boostable-7f657bc6d7-wctsw    11m          201Mi           
guranteed-69875c65d6-576pl    5m           1002Mi          
guranteed-69875c65d6-cr5s4    5m           1002Mi          
guranteed-69875c65d6-gb5zj    5m           1003Mi          
guranteed-69875c65d6-hp88k    6m           1002Mi          
guranteed-69875c65d6-q5qjm    0m           1002Mi          
guranteed-69875c65d6-x7n7z    5m           1002Mi      
```

ノードの状態を確認する OOMが発生を確認
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl describe node node2
Name:               node2
Roles:              worker
Labels:             beta.kubernetes.io/arch=amd64
                    beta.kubernetes.io/os=linux
                    kubernetes.io/arch=amd64
                    kubernetes.io/hostname=node2
                    kubernetes.io/os=linux
                    node-role.kubernetes.io/worker=
                    role=worker-node
Annotations:        kubeadm.alpha.kubernetes.io/cri-socket: unix:///var/run/containerd/containerd.sock
                    node.alpha.kubernetes.io/ttl: 0
                    volumes.kubernetes.io/controller-managed-attach-detach: true
CreationTimestamp:  Fri, 25 Oct 2024 17:08:51 +0900
Taints:             <none>
Unschedulable:      false
Lease:
  HolderIdentity:  node2
  AcquireTime:     <unset>
  RenewTime:       Sun, 27 Oct 2024 16:27:29 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Sun, 27 Oct 2024 16:27:30 +0900   Sun, 27 Oct 2024 08:42:10 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Sun, 27 Oct 2024 16:27:30 +0900   Fri, 25 Oct 2024 18:12:53 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Sun, 27 Oct 2024 16:27:30 +0900   Fri, 25 Oct 2024 18:12:53 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Sun, 27 Oct 2024 16:27:30 +0900   Sun, 27 Oct 2024 16:12:08 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
Addresses:
  InternalIP:  172.16.3.32
  Hostname:    node2
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
System Info:
  Machine ID:                 c1584411f7b8435ebc25c68dbb4e30e4
  System UUID:                aa87d4dd-9818-435b-97cc-14dfbfd79bd1
  Boot ID:                    a564bfa5-3175-49bc-871f-3176ba9acb29
  Kernel Version:             5.4.0-136-generic
  OS Image:                   Ubuntu 20.04.5 LTS
  Operating System:           linux
  Architecture:               amd64
  Container Runtime Version:  containerd://1.7.22
  Kubelet Version:            v1.29.9
  Kube-Proxy Version:         v1.29.9
PodCIDR:                      10.244.2.0/24
PodCIDRs:                     10.244.2.0/24
Non-terminated Pods:          (13 in total)
  Namespace                   Name                           CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                           ------------  ----------  ---------------  -------------  ---
  default                     besteffort-657f6c859c-g6xrg    0 (0%)        0 (0%)      0 (0%)           0 (0%)         6m44s
  default                     boostable-7f657bc6d7-wctsw     100m (2%)     0 (0%)      100Mi (1%)       0 (0%)         6m44s
  default                     guranteed-69875c65d6-576pl     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   4m33s
  default                     guranteed-69875c65d6-cr5s4     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   4m33s
  default                     guranteed-69875c65d6-gb5zj     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   4m33s
  default                     guranteed-69875c65d6-hp88k     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   4m33s
  default                     guranteed-69875c65d6-q5qjm     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   6m43s
  default                     guranteed-69875c65d6-x7n7z     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   4m33s
  kube-system                 cilium-envoy-5w4zk             0 (0%)        0 (0%)      0 (0%)           0 (0%)         47h
  kube-system                 cilium-hg4mx                   100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         47h
  kube-system                 coredns-76f75df574-fg7t9       100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     47h
  kube-system                 coredns-76f75df574-j7w94       100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     47h
  kube-system                 kube-proxy-457qh               0 (0%)        0 (0%)      0 (0%)           0 (0%)         47h
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests      Limits
  --------           --------      ------
  cpu                1 (25%)       600m (15%)
  memory             6280Mi (88%)  6370Mi (90%)
  ephemeral-storage  0 (0%)        0 (0%)
  hugepages-1Gi      0 (0%)        0 (0%)
  hugepages-2Mi      0 (0%)        0 (0%)
Events:
  Type     Reason                   Age                   From     Message
  ----     ------                   ----                  ----     -------
  Warning  SystemOOM                32m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 11415
  Warning  SystemOOM                32m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 11481
  Warning  SystemOOM                29m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 12147
  Warning  SystemOOM                29m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 12220
  Warning  SystemOOM                29m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13320
  Warning  SystemOOM                28m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13395
  Warning  SystemOOM                27m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13469
  Warning  SystemOOM                26m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13546
  Warning  SystemOOM                26m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13623
  Warning  SystemOOM                16m (x13 over 3h14m)  kubelet  (combined from similar events): System OOM encountered, victim process: ld-musl-x86_64., pid: 14003
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 7152
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 12220
  Normal   Starting                 15m                   kubelet  Starting kubelet.
  Warning  InvalidDiskCapacity      15m                   kubelet  invalid capacity 0 on image filesystem
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 7273
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 7380
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 11415
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 11481
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 12147
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 7072
  Warning  SystemOOM                15m                   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13320
  Normal   NodeAllocatableEnforced  15m                   kubelet  Updated Node Allocatable limit across pods
  Normal   NodeHasSufficientMemory  15m                   kubelet  Node node2 status is now: NodeHasSufficientMemory
  Normal   NodeHasNoDiskPressure    15m                   kubelet  Node node2 status is now: NodeHasNoDiskPressure
  Normal   NodeHasSufficientPID     15m                   kubelet  Node node2 status is now: NodeHasSufficientPID
  Normal   NodeNotReady             15m                   kubelet  Node node2 status is now: NodeNotReady
  Warning  SystemOOM                17s (x21 over 15m)    kubelet  (combined from similar events): System OOM encountered, victim process: ld-musl-x86_64., pid: 16750
  ```

  