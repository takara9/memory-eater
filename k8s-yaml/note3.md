## kubeletのコンフィグに設定を追加

```
root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml 
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-27 07:34:32.158581479 +0000
@@ -43,3 +43,5 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+kubeReserved:
+  memory: "800Mi"
```

kubeletを再起動
```
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Sun 2024-10-27 07:36:05 UTC; 9s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 17835 (kubelet)
      Tasks: 14 (limit: 9445)
     Memory: 65.7M
     CGroup: /system.slice/kubelet.service
             └─17835 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml --container-runtime-endpoint=unix:///var/run/co>
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
No resources found in default namespace.
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   83m          4%     1095Mi          28%       
node1     38m          0%     768Mi           9%        
node2     27m          0%     532Mi           7%        
No resources found in default namespace.
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl describe node node2
Name:               node2
Roles:              worker
Labels:             beta.kubernetes.io/arch=amd64
<中略>
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
```

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


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-besteffort.yaml
l apply -f deployment-boostable.yaml
kubectl apply -f deployment-guranteed.yaml deployment.apps/besteffort created
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-boostable.yaml
deployment.apps/boostable created
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-guranteed.yaml 
deployment.apps/guranteed created
tkr@hmc:~/memory-eater/k8s-yaml$ 
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o json |jq -r ".items[]| .metadata.name, .status.qosClass" | xargs -n2
besteffort-657f6c859c-tqnsn BestEffort
boostable-7f657bc6d7-hw6mh Burstable
guranteed-69875c65d6-7twgl Guaranteed
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP          NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-tqnsn   1/1     Running   0          67s   10.0.0.20   node2   <none>           <none>
boostable-7f657bc6d7-hw6mh    1/1     Running   0          67s   10.0.0.59   node2   <none>           <none>
guranteed-69875c65d6-7twgl    1/1     Running   0          65s   10.0.0.9    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   90m          4%     1096Mi          28%       
node1     40m          1%     768Mi           9%        
node2     89m          2%     2981Mi          42%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-tqnsn   11m          1003Mi          
boostable-7f657bc6d7-hw6mh    10m          1002Mi          
guranteed-69875c65d6-7twgl    16m          802Mi           
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP          NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-tqnsn   1/1     Running   0          73s   10.0.0.20   node2   <none>           <none>
boostable-7f657bc6d7-hw6mh    1/1     Running   0          73s   10.0.0.59   node2   <none>           <none>
guranteed-69875c65d6-7twgl    1/1     Running   0          71s   10.0.0.9    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   85m          4%     1096Mi          28%       
node1     36m          0%     769Mi           9%        
node2     65m          1%     3556Mi          50%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-tqnsn   0m           1003Mi          
boostable-7f657bc6d7-hw6mh    0m           1002Mi          
guranteed-69875c65d6-7twgl    19m          1002Mi  
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":6}}'
deployment.apps/guranteed patched
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE    IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-tqnsn   1/1     Running   0          4m2s   10.0.0.20    node2   <none>           <none>
boostable-7f657bc6d7-hw6mh    1/1     Running   0          4m2s   10.0.0.59    node2   <none>           <none>
guranteed-69875c65d6-7twgl    1/1     Running   0          4m     10.0.0.9     node2   <none>           <none>
guranteed-69875c65d6-bpb52    1/1     Running   0          34s    10.0.0.228   node2   <none>           <none>
guranteed-69875c65d6-jcpgq    1/1     Running   0          34s    10.0.0.177   node2   <none>           <none>
guranteed-69875c65d6-pdlql    1/1     Running   0          34s    10.0.0.27    node2   <none>           <none>
guranteed-69875c65d6-vkbvd    1/1     Running   0          34s    10.0.0.155   node2   <none>           <none>
guranteed-69875c65d6-zmp24    1/1     Running   0          34s    10.0.0.87    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   82m          4%     1096Mi          28%       
node1     39m          0%     768Mi           9%        
node2     162m         4%     5087Mi          72%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-tqnsn   0m           1003Mi          
boostable-7f657bc6d7-hw6mh    0m           1002Mi          
guranteed-69875c65d6-7twgl    0m           1002Mi          
guranteed-69875c65d6-bpb52    16m          458Mi           
guranteed-69875c65d6-jcpgq    13m          401Mi           
guranteed-69875c65d6-pdlql    14m          401Mi           
guranteed-69875c65d6-vkbvd    14m          401Mi           
guranteed-69875c65d6-zmp24    13m          401Mi      
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS      AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-tqnsn   1/1     Running   1 (14s ago)   4m30s   10.0.0.20    node2   <none>           <none>
boostable-7f657bc6d7-hw6mh    1/1     Running   1 (7s ago)    4m30s   10.0.0.59    node2   <none>           <none>
guranteed-69875c65d6-7twgl    1/1     Running   0             4m28s   10.0.0.9     node2   <none>           <none>
guranteed-69875c65d6-bpb52    1/1     Running   0             62s     10.0.0.228   node2   <none>           <none>
guranteed-69875c65d6-jcpgq    1/1     Running   0             62s     10.0.0.177   node2   <none>           <none>
guranteed-69875c65d6-pdlql    1/1     Running   0             62s     10.0.0.27    node2   <none>           <none>
guranteed-69875c65d6-vkbvd    1/1     Running   0             62s     10.0.0.155   node2   <none>           <none>
guranteed-69875c65d6-zmp24    1/1     Running   0             62s     10.0.0.87    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   81m          4%     1096Mi          28%       
node1     42m          1%     768Mi           9%        
node2     146m         3%     7470Mi          105%      
NAME                         CPU(cores)   MEMORY(bytes)   
boostable-7f657bc6d7-hw6mh   3m           0Mi             
guranteed-69875c65d6-7twgl   0m           1002Mi          
guranteed-69875c65d6-bpb52   15m          1002Mi          
guranteed-69875c65d6-jcpgq   16m          1002Mi          
guranteed-69875c65d6-pdlql   18m          1002Mi          
guranteed-69875c65d6-vkbvd   18m          1002Mi          
guranteed-69875c65d6-zmp24   17m          1002Mi      
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
Taints:             <none>
Unschedulable:      false
Lease:
  HolderIdentity:  node2
  AcquireTime:     <unset>
  RenewTime:       Sun, 27 Oct 2024 18:05:21 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Sun, 27 Oct 2024 18:02:49 +0900   Sun, 27 Oct 2024 08:42:10 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Sun, 27 Oct 2024 18:02:49 +0900   Fri, 25 Oct 2024 18:12:53 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Sun, 27 Oct 2024 18:02:49 +0900   Fri, 25 Oct 2024 18:12:53 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Sun, 27 Oct 2024 18:02:49 +0900   Sun, 27 Oct 2024 16:12:08 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
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
  default                     besteffort-657f6c859c-tqnsn    0 (0%)        0 (0%)      0 (0%)           0 (0%)         5m5s
  default                     boostable-7f657bc6d7-hw6mh     100m (2%)     0 (0%)      100Mi (1%)       0 (0%)         5m5s
  default                     guranteed-69875c65d6-7twgl     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   5m3s
  default                     guranteed-69875c65d6-bpb52     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   97s
  default                     guranteed-69875c65d6-jcpgq     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   97s
  default                     guranteed-69875c65d6-pdlql     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   97s
  default                     guranteed-69875c65d6-vkbvd     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   97s
  default                     guranteed-69875c65d6-zmp24     100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   97s
  kube-system                 cilium-envoy-5w4zk             0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d
  kube-system                 cilium-hg4mx                   100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         2d
  kube-system                 coredns-76f75df574-fg7t9       100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     2d
  kube-system                 coredns-76f75df574-j7w94       100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     2d
  kube-system                 kube-proxy-457qh               0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d
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
  Type     Reason     Age   From     Message
  ----     ------     ----  ----     -------
  Warning  SystemOOM  49s   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 18547
  Warning  SystemOOM  42s   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 18605
  Warning  SystemOOM  17s   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 19250
```


```
root@node2:/var/lib/kubelet# free -h
              total        used        free      shared  buff/cache   available
Mem:          7.8Gi       6.7Gi       857Mi       3.0Mi       250Mi       849Mi
Swap:            0B          0B          0B
root@node2:/var/lib/kubelet# free -h
              total        used        free      shared  buff/cache   available
Mem:          7.8Gi       6.9Gi       625Mi       3.0Mi       287Mi       654Mi
Swap:            0B          0B          0B

```

```
[22631.394739] oom-kill:constraint=CONSTRAINT_NONE,nodemask=(null),cpuset=/,mems_allowed=0,global_oom,task_memcg=/system.slice/containerd.service/kubepods-besteffort-pod4e8324ec_4991_4b87_b786_9816b5541dc8.slice:cri-containerd:bfc41374fc5d00d0645991cdb61e3fcbb4c9ac53567a236e5815b1a55da31054,task=ld-musl-x86_64.,pid=19545,uid=65534
[22631.394749] Out of memory: Killed process 19545 (ld-musl-x86_64.) total-vm:1024980kB, anon-rss:963920kB, file-rss:4kB, shmem-rss:0kB, UID:65534 pgtables:1944kB oom_score_adj:1000
```