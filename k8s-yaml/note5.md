
```
root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-27 23:33:18.206881570 +0000
@@ -43,3 +43,8 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+systemReserved:
+  memory: 1Gi
+kubeReserved:
+  memory: 1Gi
+
```

```
root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Sun 2024-10-27 23:33:21 UTC; 2s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 4325 (kubelet)
      Tasks: 11 (limit: 9445)
     Memory: 27.9M
     CGroup: /system.slice/kubelet.service
             └─4325 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml --container-runtime-endpoin>

```

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
Taints:             node.cilium.io/agent-not-ready:NoSchedule
Unschedulable:      false
Lease:
  HolderIdentity:  node2
  AcquireTime:     <unset>
  RenewTime:       Mon, 28 Oct 2024 08:33:31 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Mon, 28 Oct 2024 08:33:21 +0900   Sun, 27 Oct 2024 19:36:34 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Mon, 28 Oct 2024 08:33:21 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Mon, 28 Oct 2024 08:33:21 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Mon, 28 Oct 2024 08:33:21 +0900   Mon, 28 Oct 2024 08:12:42 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
Addresses:
  InternalIP:  172.16.3.32
  Hostname:    node2
Capacity:
  cpu:                4
  ephemeral-storage:  16400252Ki
  hugepages-1Gi:      0
  hugepages-2Mi:      0
  memory:             8150228Ki
  pods:               110
Allocatable:
  cpu:                4
  ephemeral-storage:  15114472219
  hugepages-1Gi:      0
  hugepages-2Mi:      0
  memory:             5950676Ki
  pods:               110
System Info:
  Machine ID:                 c1584411f7b8435ebc25c68dbb4e30e4
  System UUID:                aa87d4dd-9818-435b-97cc-14dfbfd79bd1
  Boot ID:                    0861c921-0ee8-4309-b5bf-b6faf5785452
  Kernel Version:             5.4.0-136-generic
  OS Image:                   Ubuntu 20.04.5 LTS
  Operating System:           linux
  Architecture:               amd64
  Container Runtime Version:  containerd://1.7.22
  Kubelet Version:            v1.29.9
  Kube-Proxy Version:         v1.29.9
PodCIDR:                      10.244.2.0/24
PodCIDRs:                     10.244.2.0/24
Non-terminated Pods:          (5 in total)
  Namespace                   Name                        CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                        ------------  ----------  ---------------  -------------  ---
  kube-system                 cilium-envoy-5w4zk          0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d15h
  kube-system                 cilium-hg4mx                100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         2d15h
  kube-system                 coredns-76f75df574-fg7t9    100m (2%)     0 (0%)      70Mi (1%)        170Mi (2%)     2d15h
  kube-system                 coredns-76f75df574-j7w94    100m (2%)     0 (0%)      70Mi (1%)        170Mi (2%)     2d15h
  kube-system                 kube-proxy-457qh            0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d15h
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests    Limits
  --------           --------    ------
  cpu                300m (7%)   0 (0%)
  memory             150Mi (2%)  340Mi (5%)
  ephemeral-storage  0 (0%)      0 (0%)
  hugepages-1Gi      0 (0%)      0 (0%)
  hugepages-2Mi      0 (0%)      0 (0%)
Events:
  Type     Reason                     Age                From             Message
  ----     ------                     ----               ----             -------
  Normal   Starting                   20m                kube-proxy       
  Normal   NodeHasInsufficientMemory  13h                kubelet          Node node2 status is now: NodeHasInsufficientMemory
  Warning  EvictionThresholdMet       13h (x2 over 13h)  kubelet          Attempting to reclaim memory
  Normal   NodeHasSufficientMemory    12h (x4 over 14h)  kubelet          Node node2 status is now: NodeHasSufficientMemory
  Warning  Rebooted                   20m                kubelet          Node node2 has been rebooted, boot id: 0861c921-0ee8-4309-b5bf-b6faf5785452
  Normal   NodeNotReady               20m                kubelet          Node node2 status is now: NodeNotReady
  Normal   NodeHasSufficientMemory    20m                kubelet          Node node2 status is now: NodeHasSufficientMemory
  Normal   NodeHasNoDiskPressure      20m                kubelet          Node node2 status is now: NodeHasNoDiskPressure
  Normal   NodeHasSufficientPID       20m                kubelet          Node node2 status is now: NodeHasSufficientPID
  Normal   NodeAllocatableEnforced    20m                kubelet          Updated Node Allocatable limit across pods
  Normal   Starting                   20m                kubelet          Starting kubelet.
  Warning  InvalidDiskCapacity        20m                kubelet          invalid capacity 0 on image filesystem
  Normal   NodeReady                  20m                kubelet          Node node2 status is now: NodeReady
  Normal   RegisteredNode             20m                node-controller  Node node2 event: Registered Node node2 in Controller
  Normal   Starting                   19s                kubelet          Starting kubelet.
  Warning  InvalidDiskCapacity        19s                kubelet          invalid capacity 0 on image filesystem
  Normal   NodeAllocatableEnforced    19s                kubelet          Updated Node Allocatable limit across pods
  Normal   NodeHasSufficientMemory    19s                kubelet          Node node2 status is now: NodeHasSufficientMemory
  Normal   NodeHasNoDiskPressure      19s                kubelet          Node node2 status is now: NodeHasNoDiskPressure
  Normal   NodeHasSufficientPID       19s                kubelet          Node node2 status is now: NodeHasSufficientPID
tkr@hmc:~/memory-eater/k8s-yaml$ 
```

```
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

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-pmsvb   1/1     Running   0          77s   10.0.0.65    node2   <none>           <none>
boostable-7f657bc6d7-zdw5c    1/1     Running   0          76s   10.0.0.250   node2   <none>           <none>
guranteed-69875c65d6-wzz79    1/1     Running   0          75s   10.0.0.252   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   83m          4%     897Mi           23%       
node1     35m          0%     747Mi           9%        
node2     32m          0%     3654Mi          62%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-pmsvb   0m           1002Mi          
boostable-7f657bc6d7-zdw5c    0m           1002Mi          
guranteed-69875c65d6-wzz79    0m           1002Mi      
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":6}}'
deployment.apps/guranteed patched
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS        RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-pmsvb   1/1     Running       0          2m28s   10.0.0.65    node2   <none>           <none>
boostable-7f657bc6d7-zdw5c    1/1     Running       0          2m27s   10.0.0.250   node2   <none>           <none>
guranteed-69875c65d6-22mdr    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-25889    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-2hft4    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4c25n    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4j6bv    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4k4hs    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4xkrk    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-5dg7c    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-5fqnx    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-5m44q    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-72h4h    1/1     Running       0          5s      10.0.0.66    node2   <none>           <none>
guranteed-69875c65d6-7bqrl    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8jdh6    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-9sp97    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bgr8h    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-c89bg    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-crt76    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-dkd6t    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-dl2t6    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-dv85h    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-fhvqt    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-flc6w    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-gnq7q    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-h7kcs    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-hnqjd    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jczjv    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jk5vk    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jl6jp    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-ktpgr    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-lgsp2    1/1     Running       0          5s      10.0.0.182   node2   <none>           <none>
guranteed-69875c65d6-ltgxm    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-lzv4t    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-mp772    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-nqxfg    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-nvsnl    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-pj4m5    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-qd9dn    1/1     Running       0          5s      10.0.0.147   node2   <none>           <none>
guranteed-69875c65d6-qnlpb    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-r629w    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-rmh7r    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-szcsg    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-t292l    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-t8m9p    1/1     Running       0          5s      10.0.0.106   node2   <none>           <none>
guranteed-69875c65d6-txmkx    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vl2tx    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-wzz79    1/1     Running       0          2m26s   10.0.0.252   node2   <none>           <none>
guranteed-69875c65d6-xm4pl    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-z6s5g    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   81m          4%     883Mi           23%       
node1     36m          0%     747Mi           9%        
node2     30m          0%     3656Mi          62%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-pmsvb   0m           1002Mi          
boostable-7f657bc6d7-zdw5c    0m           1002Mi          
guranteed-69875c65d6-wzz79    0m           1002Mi      
```



nodeSelectorは外して再度メモリ不足状態を作成

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.capacity.memory, .status.allocatable.memory" | xargs -n3
master1 4026044Ki 3923644Ki
node1 8150236Ki 8047836Ki
node2 8150228Ki 5950676Ki
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-5z9dv   1/1     Running   0          4m10s   10.0.0.213   node2   <none>           <none>
boostable-7f657bc6d7-tr9zl    1/1     Running   0          4m10s   10.0.0.137   node2   <none>           <none>
guranteed-55c8bc8b4b-4hvwm    1/1     Running   0          2m43s   10.0.2.8     node1   <none>           <none>
guranteed-55c8bc8b4b-6vcpm    1/1     Running   0          2m43s   10.0.0.170   node2   <none>           <none>
guranteed-55c8bc8b4b-bktxf    1/1     Running   0          4m8s    10.0.0.72    node2   <none>           <none>
guranteed-55c8bc8b4b-gq69z    1/1     Running   0          2m43s   10.0.2.22    node1   <none>           <none>
guranteed-55c8bc8b4b-k6n5f    1/1     Running   0          2m43s   10.0.0.32    node2   <none>           <none>
guranteed-55c8bc8b4b-knvcj    1/1     Running   0          59s     10.0.2.33    node1   <none>           <none>
guranteed-55c8bc8b4b-nl9k4    1/1     Running   0          59s     10.0.0.70    node2   <none>           <none>
guranteed-55c8bc8b4b-wmh9l    1/1     Running   0          2m43s   10.0.2.191   node1   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   90m          4%     996Mi           26%       
node1     48m          1%     4673Mi          59%       
node2     43m          1%     6507Mi          111%      
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-5z9dv   0m           1002Mi          
boostable-7f657bc6d7-tr9zl    0m           1002Mi          
guranteed-55c8bc8b4b-4hvwm    0m           1002Mi          
guranteed-55c8bc8b4b-6vcpm    0m           1002Mi          
guranteed-55c8bc8b4b-bktxf    0m           1002Mi          
guranteed-55c8bc8b4b-gq69z    0m           1002Mi          
guranteed-55c8bc8b4b-k6n5f    0m           1002Mi          
guranteed-55c8bc8b4b-knvcj    10m          1002Mi          
guranteed-55c8bc8b4b-nl9k4    10m          1002Mi          
guranteed-55c8bc8b4b-wmh9l    0m           1002Mi          

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.conditions[1].type, .status.conditions[1].status" | xargs -n3
master1 MemoryPressure False
node1 MemoryPressure False
node2 MemoryPressure False

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.capacity.memory, .status.allocatable.memory" | xargs -n3
master1 4026044Ki 3923644Ki
node1 8150236Ki 8047836Ki
node2 8150228Ki 5950676Ki
```

