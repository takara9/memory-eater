```
root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml 
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-27 09:10:00.532869534 +0000
@@ -43,3 +43,5 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+evictionHard:
+        memory.available: "800Mi"`
```

```
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Sun 2024-10-27 09:12:09 UTC; 2s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 20914 (kubelet)
      Tasks: 13 (limit: 9445)
     Memory: 34.6M
     CGroup: /system.slice/kubelet.service
             └─20914 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml --container-runtime-endpoint=unix:///var/run/co>
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
  RenewTime:       Sun, 27 Oct 2024 19:24:35 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Sun, 27 Oct 2024 19:23:36 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Sun, 27 Oct 2024 19:23:36 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Sun, 27 Oct 2024 19:23:36 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Sun, 27 Oct 2024 19:23:36 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
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
  ephemeral-storage:  16400252Ki
  hugepages-1Gi:      0
  hugepages-2Mi:      0
  memory:             7331036Ki
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
Non-terminated Pods:          (5 in total)
  Namespace                   Name                        CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                        ------------  ----------  ---------------  -------------  ---
  kube-system                 cilium-envoy-5w4zk          0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d2h
  kube-system                 cilium-hg4mx                100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         2d2h
  kube-system                 coredns-76f75df574-fg7t9    100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     2d2h
  kube-system                 coredns-76f75df574-j7w94    100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     2d2h
  kube-system                 kube-proxy-457qh            0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d2h
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests    Limits
  --------           --------    ------
  cpu                300m (7%)   0 (0%)
  memory             150Mi (2%)  340Mi (4%)
  ephemeral-storage  0 (0%)      0 (0%)
  hugepages-1Gi      0 (0%)      0 (0%)
  hugepages-2Mi      0 (0%)      0 (0%)
Events:              <none>
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node
NAME      STATUS   ROLES           AGE    VERSION
master1   Ready    control-plane   2d2h   v1.29.9
node1     Ready    worker          2d2h   v1.29.9
node2     Ready    worker          2d2h   v1.29.9
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
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-hdfbw   1/1     Running   0          28s   10.0.0.38    node2   <none>           <none>
boostable-7f657bc6d7-wbzz7    1/1     Running   0          27s   10.0.0.111   node2   <none>           <none>
guranteed-69875c65d6-zw4kw    1/1     Running   0          27s   10.0.0.65    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   82m          4%     1107Mi          28%       
node1     39m          0%     836Mi           10%       
node2     99m          2%     1927Mi          26%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-hdfbw   21m          501Mi           
boostable-7f657bc6d7-wbzz7    20m          501Mi           
guranteed-69875c65d6-zw4kw    21m          401Mi     
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-hdfbw   1/1     Running   0          45s   10.0.0.38    node2   <none>           <none>
boostable-7f657bc6d7-wbzz7    1/1     Running   0          44s   10.0.0.111   node2   <none>           <none>
guranteed-69875c65d6-zw4kw    1/1     Running   0          44s   10.0.0.65    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   84m          4%     1108Mi          28%       
node1     39m          0%     837Mi           10%       
node2     88m          2%     2430Mi          33%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-hdfbw   21m          802Mi           
boostable-7f657bc6d7-wbzz7    21m          802Mi           
guranteed-69875c65d6-zw4kw    23m          602Mi           
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-hdfbw   1/1     Running   0          2m28s   10.0.0.38    node2   <none>           <none>
boostable-7f657bc6d7-wbzz7    1/1     Running   0          2m27s   10.0.0.111   node2   <none>           <none>
guranteed-69875c65d6-zw4kw    1/1     Running   0          2m27s   10.0.0.65    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   82m          4%     1109Mi          28%       
node1     40m          1%     837Mi           10%       
node2     34m          0%     3533Mi          49%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-hdfbw   0m           1002Mi          
boostable-7f657bc6d7-wbzz7    0m           1002Mi          
guranteed-69875c65d6-zw4kw    0m           1002Mi     
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted
```

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-hdfbw   1/1     Running   0          3m34s   10.0.0.38    node2   <none>           <none>
boostable-7f657bc6d7-wbzz7    1/1     Running   0          3m33s   10.0.0.111   node2   <none>           <none>
guranteed-69875c65d6-b5v42    1/1     Running   0          10s     10.0.0.143   node2   <none>           <none>
guranteed-69875c65d6-dpw2f    1/1     Running   0          10s     10.0.0.91    node2   <none>           <none>
guranteed-69875c65d6-jbtkf    1/1     Running   0          10s     10.0.0.124   node2   <none>           <none>
guranteed-69875c65d6-s579f    1/1     Running   0          10s     10.0.0.174   node2   <none>           <none>
guranteed-69875c65d6-v9tw8    1/1     Running   0          10s     10.0.0.120   node2   <none>           <none>
guranteed-69875c65d6-zw4kw    1/1     Running   0          3m33s   10.0.0.65    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   84m          4%     1109Mi          28%       
node1     38m          0%     837Mi           10%       
node2     33m          0%     3533Mi          49%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-hdfbw   0m           1002Mi          
boostable-7f657bc6d7-wbzz7    0m           1002Mi          
guranteed-69875c65d6-zw4kw    0m           1002Mi          
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-hdfbw   1/1     Running   0          3m54s   10.0.0.38    node2   <none>           <none>
boostable-7f657bc6d7-wbzz7    1/1     Running   0          3m53s   10.0.0.111   node2   <none>           <none>
guranteed-69875c65d6-b5v42    1/1     Running   0          30s     10.0.0.143   node2   <none>           <none>
guranteed-69875c65d6-dpw2f    1/1     Running   0          30s     10.0.0.91    node2   <none>           <none>
guranteed-69875c65d6-jbtkf    1/1     Running   0          30s     10.0.0.124   node2   <none>           <none>
guranteed-69875c65d6-s579f    1/1     Running   0          30s     10.0.0.174   node2   <none>           <none>
guranteed-69875c65d6-v9tw8    1/1     Running   0          30s     10.0.0.120   node2   <none>           <none>
guranteed-69875c65d6-zw4kw    1/1     Running   0          3m53s   10.0.0.65    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   94m          4%     1110Mi          28%       
node1     44m          1%     837Mi           10%       
node2     343m         8%     4559Mi          63%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-hdfbw   0m           1002Mi          
boostable-7f657bc6d7-wbzz7    0m           1002Mi          
guranteed-69875c65d6-b5v42    27m          301Mi           
guranteed-69875c65d6-dpw2f    27m          301Mi           
guranteed-69875c65d6-jbtkf    26m          301Mi           
guranteed-69875c65d6-s579f    27m          301Mi           
guranteed-69875c65d6-v9tw8    22m          301Mi           
guranteed-69875c65d6-zw4kw    0m           1002Mi          
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS      RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-77str   1/1     Running     0          10s     10.0.2.156   node1   <none>           <none>
besteffort-657f6c859c-hdfbw   0/1     Completed   0          4m24s   10.0.0.38    node2   <none>           <none>
boostable-7f657bc6d7-wbzz7    1/1     Running     0          4m23s   10.0.0.111   node2   <none>           <none>
guranteed-69875c65d6-b5v42    1/1     Running     0          60s     10.0.0.143   node2   <none>           <none>
guranteed-69875c65d6-dpw2f    1/1     Running     0          60s     10.0.0.91    node2   <none>           <none>
guranteed-69875c65d6-jbtkf    1/1     Running     0          60s     10.0.0.124   node2   <none>           <none>
guranteed-69875c65d6-s579f    1/1     Running     0          60s     10.0.0.174   node2   <none>           <none>
guranteed-69875c65d6-v9tw8    1/1     Running     0          60s     10.0.0.120   node2   <none>           <none>
guranteed-69875c65d6-zw4kw    1/1     Running     0          4m23s   10.0.0.65    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   91m          4%     1109Mi          28%       
node1     39m          0%     837Mi           10%       
node2     123m         3%     6568Mi          91%       
NAME                         CPU(cores)   MEMORY(bytes)   
boostable-7f657bc6d7-wbzz7   0m           1002Mi          
guranteed-69875c65d6-b5v42   25m          736Mi           
guranteed-69875c65d6-dpw2f   26m          731Mi           
guranteed-69875c65d6-jbtkf   27m          746Mi           
guranteed-69875c65d6-s579f   25m          734Mi           
guranteed-69875c65d6-v9tw8   27m          742Mi           
guranteed-69875c65d6-zw4kw   0m           1002Mi          
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS      RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-77str   1/1     Running     0          2m23s   10.0.2.156   node1   <none>           <none>
besteffort-657f6c859c-hdfbw   0/1     Completed   0          6m37s   10.0.0.38    node2   <none>           <none>
boostable-7f657bc6d7-wbzz7    0/1     Completed   0          6m36s   10.0.0.111   node2   <none>           <none>
boostable-7f657bc6d7-zdbtw    1/1     Running     0          2m11s   10.0.2.176   node1   <none>           <none>
guranteed-69875c65d6-b5v42    1/1     Running     0          3m13s   10.0.0.143   node2   <none>           <none>
guranteed-69875c65d6-dpw2f    1/1     Running     0          3m13s   10.0.0.91    node2   <none>           <none>
guranteed-69875c65d6-jbtkf    1/1     Running     0          3m13s   10.0.0.124   node2   <none>           <none>
guranteed-69875c65d6-s579f    1/1     Running     0          3m13s   10.0.0.174   node2   <none>           <none>
guranteed-69875c65d6-v9tw8    1/1     Running     0          3m13s   10.0.0.120   node2   <none>           <none>
guranteed-69875c65d6-zw4kw    1/1     Running     0          6m36s   10.0.0.65    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   80m          4%     1112Mi          29%       
node1     40m          1%     2855Mi          36%       
node2     40m          1%     6506Mi          90%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-77str   0m           1003Mi          
boostable-7f657bc6d7-zdbtw    0m           1002Mi          
guranteed-69875c65d6-b5v42    0m           1003Mi          
guranteed-69875c65d6-dpw2f    0m           1002Mi          
guranteed-69875c65d6-jbtkf    0m           1002Mi          
guranteed-69875c65d6-s579f    0m           1002Mi          
guranteed-69875c65d6-v9tw8    0m           1002Mi          
guranteed-69875c65d6-zw4kw    0m           1002Mi          
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS      RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-77str   1/1     Running     0          2m54s   10.0.2.156   node1   <none>           <none>
besteffort-657f6c859c-hdfbw   0/1     Completed   0          7m8s    10.0.0.38    node2   <none>           <none>
boostable-7f657bc6d7-wbzz7    0/1     Completed   0          7m7s    10.0.0.111   node2   <none>           <none>
boostable-7f657bc6d7-zdbtw    1/1     Running     0          2m42s   10.0.2.176   node1   <none>           <none>
guranteed-69875c65d6-b5v42    1/1     Running     0          3m44s   10.0.0.143   node2   <none>           <none>
guranteed-69875c65d6-dpw2f    1/1     Running     0          3m44s   10.0.0.91    node2   <none>           <none>
guranteed-69875c65d6-jbtkf    1/1     Running     0          3m44s   10.0.0.124   node2   <none>           <none>
guranteed-69875c65d6-s579f    1/1     Running     0          3m44s   10.0.0.174   node2   <none>           <none>
guranteed-69875c65d6-v9tw8    1/1     Running     0          3m44s   10.0.0.120   node2   <none>           <none>
guranteed-69875c65d6-zw4kw    1/1     Running     0          7m7s    10.0.0.65    node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   82m          4%     1112Mi          29%       
node1     40m          1%     2855Mi          36%       
node2     39m          0%     6502Mi          90%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-77str   0m           1003Mi          
boostable-7f657bc6d7-zdbtw    0m           1002Mi          
guranteed-69875c65d6-b5v42    0m           1003Mi          
guranteed-69875c65d6-dpw2f    0m           1002Mi          
guranteed-69875c65d6-jbtkf    0m           1002Mi          
guranteed-69875c65d6-s579f    0m           1002Mi          
guranteed-69875c65d6-v9tw8    0m           1002Mi          
guranteed-69875c65d6-zw4kw    0m           1002Mi          
tkr@hmc:~/memory-eater/k8s-yaml$ 
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
  RenewTime:       Sun, 27 Oct 2024 19:38:52 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Sun, 27 Oct 2024 19:36:34 +0900   Sun, 27 Oct 2024 19:36:34 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Sun, 27 Oct 2024 19:36:34 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Sun, 27 Oct 2024 19:36:34 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Sun, 27 Oct 2024 19:36:34 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
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
  ephemeral-storage:  16400252Ki
  hugepages-1Gi:      0
  hugepages-2Mi:      0
  memory:             7331036Ki
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
Non-terminated Pods:          (11 in total)
  Namespace                   Name                          CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                          ------------  ----------  ---------------  -------------  ---
  default                     guranteed-69875c65d6-b5v42    100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   8m30s
  default                     guranteed-69875c65d6-dpw2f    100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   8m30s
  default                     guranteed-69875c65d6-jbtkf    100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   8m30s
  default                     guranteed-69875c65d6-s579f    100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   8m30s
  default                     guranteed-69875c65d6-v9tw8    100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   8m30s
  default                     guranteed-69875c65d6-zw4kw    100m (2%)     100m (2%)   1005Mi (14%)     1005Mi (14%)   11m
  kube-system                 cilium-envoy-5w4zk            0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d2h
  kube-system                 cilium-hg4mx                  100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         2d2h
  kube-system                 coredns-76f75df574-fg7t9      100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     2d2h
  kube-system                 coredns-76f75df574-j7w94      100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     2d2h
  kube-system                 kube-proxy-457qh              0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d2h
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests      Limits
  --------           --------      ------
  cpu                900m (22%)    600m (15%)
  memory             6180Mi (86%)  6370Mi (88%)
  ephemeral-storage  0 (0%)        0 (0%)
  hugepages-1Gi      0 (0%)        0 (0%)
  hugepages-2Mi      0 (0%)        0 (0%)
Events:
  Type     Reason                     Age                    From     Message
  ----     ------                     ----                   ----     -------
  Normal   NodeHasInsufficientMemory  7m40s                  kubelet  Node node2 status is now: NodeHasInsufficientMemory
  Warning  EvictionThresholdMet       7m29s (x2 over 7m41s)  kubelet  Attempting to reclaim memory
  Normal   NodeHasSufficientMemory    2m21s (x4 over 86m)    kubelet  Node node2 status is now: NodeHasSufficientMemory
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl describe node node1
Name:               node1
Roles:              worker
Labels:             beta.kubernetes.io/arch=amd64
                    beta.kubernetes.io/os=linux
                    kubernetes.io/arch=amd64
                    kubernetes.io/hostname=node1
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
  HolderIdentity:  node1
  AcquireTime:     <unset>
  RenewTime:       Sun, 27 Oct 2024 19:39:24 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:44 +0900   Fri, 25 Oct 2024 17:09:44 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Sun, 27 Oct 2024 19:35:28 +0900   Fri, 25 Oct 2024 17:08:51 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Sun, 27 Oct 2024 19:35:28 +0900   Fri, 25 Oct 2024 17:08:51 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Sun, 27 Oct 2024 19:35:28 +0900   Fri, 25 Oct 2024 17:08:51 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Sun, 27 Oct 2024 19:35:28 +0900   Fri, 25 Oct 2024 17:09:42 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
Addresses:
  InternalIP:  172.16.3.31
  Hostname:    node1
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
  memory:             8047836Ki
  pods:               110
System Info:
  Machine ID:                 c1584411f7b8435ebc25c68dbb4e30e4
  System UUID:                43a486a0-2b58-4ec0-b5ee-af4f0d087306
  Boot ID:                    38aeadc9-6b2d-4e4e-a7ec-06981480db85
  Kernel Version:             5.4.0-136-generic
  OS Image:                   Ubuntu 20.04.5 LTS
  Operating System:           linux
  Architecture:               amd64
  Container Runtime Version:  containerd://1.7.22
  Kubelet Version:            v1.29.9
  Kube-Proxy Version:         v1.29.9
PodCIDR:                      10.244.1.0/24
PodCIDRs:                     10.244.1.0/24
Non-terminated Pods:          (10 in total)
  Namespace                   Name                                         CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                                         ------------  ----------  ---------------  -------------  ---
  default                     besteffort-657f6c859c-77str                  0 (0%)        0 (0%)      0 (0%)           0 (0%)         8m18s
  default                     boostable-7f657bc6d7-zdbtw                   100m (2%)     0 (0%)      100Mi (1%)       0 (0%)         8m6s
  ingress-nginx               ingress-nginx-controller-849dcdc99d-kdg2c    100m (2%)     0 (0%)      90Mi (1%)        0 (0%)         11h
  kube-system                 cilium-99gp6                                 100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         2d2h
  kube-system                 cilium-envoy-8vfrw                           0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d2h
  kube-system                 cilium-operator-696b7f8c9-vc4zv              0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d2h
  kube-system                 kube-proxy-w2vsq                             0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d2h
  kube-system                 metrics-server-7fb95cbc9b-f88kg              100m (2%)     0 (0%)      200Mi (2%)       0 (0%)         2d2h
  kubernetes-dashboard        dashboard-metrics-scraper-db57c5c46-gg8nb    0 (0%)        0 (0%)      0 (0%)           0 (0%)         11h
  kubernetes-dashboard        kubernetes-dashboard-5b47fd8cb5-tmgcc        0 (0%)        0 (0%)      0 (0%)           0 (0%)         11h
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests    Limits
  --------           --------    ------
  cpu                400m (10%)  0 (0%)
  memory             400Mi (5%)  0 (0%)
  ephemeral-storage  0 (0%)      0 (0%)
  hugepages-1Gi      0 (0%)      0 (0%)
  hugepages-2Mi      0 (0%)      0 (0%)
Events:              <none>
```