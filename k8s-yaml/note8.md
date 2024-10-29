root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Mon 2024-10-28 07:13:07 UTC; 2s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 4418 (kubelet)
      Tasks: 12 (limit: 9445)
     Memory: 29.7M
     CGroup: /system.slice/kubelet.service
             └─4418 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml >

root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-28 07:13:04.428936353 +0000
@@ -43,3 +43,7 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+evictionHard:
+  memory.available: 1Gi
+systemReserved:
+  memory: 1Gi


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.capacity.memory, .status.allocatable.memory" | xargs -n3
master1 4026044Ki 3923644Ki
node1 8150236Ki 8047836Ki
node2 8150236Ki 6053084Ki



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




tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-besteffort.yaml
l apply -f deployment-boostable.yaml
kubectl apply -f deployment-guranteed.yaml deployment.apps/besteffort created
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-boostable.yaml
deployment.apps/boostable created
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-guranteed.yaml 
deployment.apps/guranteed created
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o json |jq -r ".items[]| .metadata.name, .status.qosClass" | xargs -n2
besteffort-657f6c859c-qbkhk BestEffort
boostable-7f657bc6d7-9z7bq Burstable
guranteed-69875c65d6-j5ptx Guaranteed



tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-qbkhk   1/1     Running   0          3m48s   10.0.0.223   node2   <none>           <none>
boostable-7f657bc6d7-9z7bq    1/1     Running   0          3m48s   10.0.0.138   node2   <none>           <none>
guranteed-69875c65d6-j5ptx    1/1     Running   0          3m47s   10.0.0.8     node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   88m          4%     906Mi           23%       
node1     28m          0%     652Mi           8%        
node2     34m          0%     3698Mi          62%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-qbkhk   0m           1002Mi          
boostable-7f657bc6d7-9z7bq    0m           1002Mi          
guranteed-69875c65d6-j5ptx    0m           1002Mi     



tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":6}}'
deployment.apps/guranteed patched
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS        RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-qbkhk   1/1     Running       0          4m54s   10.0.0.223   node2   <none>           <none>
boostable-7f657bc6d7-9z7bq    1/1     Running       0          4m54s   10.0.0.138   node2   <none>           <none>
guranteed-69875c65d6-44jmt    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-45dnn    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4lbks    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-52snt    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-82j8b    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8b5kv    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8fvj8    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-97qh8    1/1     Running       0          3s      10.0.0.65    node2   <none>           <none>
guranteed-69875c65d6-bdcw2    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-djqqn    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-f44k9    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-j5ptx    1/1     Running       0          4m53s   10.0.0.8     node2   <none>           <none>
guranteed-69875c65d6-jfq78    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jvm57    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kr4xw    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-l8rfv    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-lcl2x    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-lxkcp    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-mdq8b    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-n7scr    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-n9f5q    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-njqc4    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-nw84x    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-qhskj    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-qssqj    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-sfnwj    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-ss4dd    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-t4lmx    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vbjgr    1/1     Running       0          3s      10.0.0.144   node2   <none>           <none>
guranteed-69875c65d6-vnbpf    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vzk8t    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-wm28c    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-zgq8p    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-zmdc4    1/1     Running       0          3s      10.0.0.112   node2   <none>           <none>
guranteed-69875c65d6-zvvm8    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-zxqc4    1/1     Running       0          3s      10.0.0.118   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   75m          3%     906Mi           23%       
node1     28m          0%     652Mi           8%        
node2     34m          0%     3699Mi          62%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-qbkhk   0m           1002Mi          
boostable-7f657bc6d7-9z7bq    0m           1002Mi          
guranteed-69875c65d6-j5ptx    0m           1002Mi    



Evictionが起きる前に、ノードのメモリ不足で、スケジュールできない。


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod guranteed-69875c65d6-vzk8t -o yaml
apiVersion: v1
kind: Pod
metadata:
  creationTimestamp: "2024-10-28T07:20:16Z"
  generateName: guranteed-69875c65d6-
  labels:
    app.kubernetes.io/name: mem-eater-gura
    pod-template-hash: 69875c65d6
  name: guranteed-69875c65d6-vzk8t
  namespace: default
  ownerReferences:
  - apiVersion: apps/v1
    blockOwnerDeletion: true
    controller: true
    kind: ReplicaSet
    name: guranteed-69875c65d6
    uid: d950f827-c910-407f-9f4c-bb08f0482363
  resourceVersion: "269869"
  uid: 30e4713b-7934-448b-bf6b-1a5d79d63009
spec:
  containers:
  - args:
    - "1"
    - "10"
    - "5"
    - guranteed
    image: ghcr.io/takara9/mem-eater:0.2
    imagePullPolicy: IfNotPresent
    name: mem-eater
    resources:
      limits:
        cpu: 100m
        memory: 1005Mi
      requests:
        cpu: 100m
        memory: 1005Mi
    terminationMessagePath: /dev/termination-log
    terminationMessagePolicy: File
    volumeMounts:
    - mountPath: /var/run/secrets/kubernetes.io/serviceaccount
      name: kube-api-access-swwft
      readOnly: true
  dnsPolicy: ClusterFirst
  enableServiceLinks: true
  nodeName: node2
  preemptionPolicy: PreemptLowerPriority
  priority: 0
  restartPolicy: Always
  schedulerName: default-scheduler
  securityContext: {}
  serviceAccount: default
  serviceAccountName: default
  terminationGracePeriodSeconds: 30
  tolerations:
  - effect: NoExecute
    key: node.kubernetes.io/not-ready
    operator: Exists
    tolerationSeconds: 300
  - effect: NoExecute
    key: node.kubernetes.io/unreachable
    operator: Exists
    tolerationSeconds: 300
  volumes:
  - name: kube-api-access-swwft
    projected:
      defaultMode: 420
      sources:
      - serviceAccountToken:
          expirationSeconds: 3607
          path: token
      - configMap:
          items:
          - key: ca.crt
            path: ca.crt
          name: kube-root-ca.crt
      - downwardAPI:
          items:
          - fieldRef:
              apiVersion: v1
              fieldPath: metadata.namespace
            path: namespace
status:
  message: 'Pod was rejected: Node didn''t have enough resource: memory, requested:
    1053818880, used: 5740953600, capacity: 6198358016'
  phase: Failed
  reason: OutOfmemory
  startTime: "2024-10-28T07:20:16Z"




tkr@hmc:~/memory-eater/k8s-yaml$ kubectl describe pod guranteed-69875c65d6-vzk8t
Name:             guranteed-69875c65d6-vzk8t
Namespace:        default
Priority:         0
Service Account:  default
Node:             node2/
Start Time:       Mon, 28 Oct 2024 16:20:16 +0900
Labels:           app.kubernetes.io/name=mem-eater-gura
                  pod-template-hash=69875c65d6
Annotations:      <none>
Status:           Failed
Reason:           OutOfmemory
Message:          Pod was rejected: Node didn't have enough resource: memory, requested: 1053818880, used: 5740953600, capacity: 6198358016
IP:               
IPs:              <none>
Controlled By:    ReplicaSet/guranteed-69875c65d6
Containers:
  mem-eater:
    Image:      ghcr.io/takara9/mem-eater:0.2
    Port:       <none>
    Host Port:  <none>
    Args:
      1
      10
      5
      guranteed
    Limits:
      cpu:     100m
      memory:  1005Mi
    Requests:
      cpu:        100m
      memory:     1005Mi
    Environment:  <none>
    Mounts:
      /var/run/secrets/kubernetes.io/serviceaccount from kube-api-access-swwft (ro)
Volumes:
  kube-api-access-swwft:
    Type:                    Projected (a volume that contains injected data from multiple sources)
    TokenExpirationSeconds:  3607
    ConfigMapName:           kube-root-ca.crt
    ConfigMapOptional:       <nil>
    DownwardAPI:             true
QoS Class:                   Guaranteed
Node-Selectors:              <none>
Tolerations:                 node.kubernetes.io/not-ready:NoExecute op=Exists for 300s
                             node.kubernetes.io/unreachable:NoExecute op=Exists for 300s
Events:
  Type     Reason       Age   From     Message
  ----     ------       ----  ----     -------
  Warning  OutOfmemory  10m   kubelet  Node didn't have enough resource: memory, requested: 1053818880, used: 5740953600, capacity: 6198358016


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get deploy guranteed
NAME        READY   UP-TO-DATE   AVAILABLE   AGE
guranteed   5/6     6            5           19m


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod |grep OutOfmemory |wc -l
6659

スケジュールできない場合、OutOfMemoryが発生するのは、仕様通り。
しかし、大量にリストに載ってくるのは、不明
https://kubernetes.io/docs/concepts/scheduling-eviction/assign-pod-node/#nodename


大量に出力される件は、バグとして扱われていない。
https://github.com/kubernetes/kubernetes/issues/114637
https://github.com/kubernetes/kubernetes/issues/89007#issuecomment-650727834



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
  RenewTime:       Mon, 28 Oct 2024 17:10:56 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Mon, 28 Oct 2024 17:08:37 +0900   Mon, 28 Oct 2024 16:32:51 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Mon, 28 Oct 2024 17:08:37 +0900   Mon, 28 Oct 2024 16:13:05 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Mon, 28 Oct 2024 17:08:37 +0900   Mon, 28 Oct 2024 16:13:05 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Mon, 28 Oct 2024 17:08:37 +0900   Mon, 28 Oct 2024 16:13:05 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
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
  memory:             6053084Ki
  pods:               110
System Info:
  Machine ID:                 c1584411f7b8435ebc25c68dbb4e30e4
  System UUID:                aa87d4dd-9818-435b-97cc-14dfbfd79bd1
  Boot ID:                    9628c828-e502-4da6-ad73-368ed25f23c7
  Kernel Version:             5.4.0-136-generic
  OS Image:                   Ubuntu 20.04.5 LTS
  Operating System:           linux
  Architecture:               amd64
  Container Runtime Version:  containerd://1.7.22
  Kubelet Version:            v1.29.9
  Kube-Proxy Version:         v1.29.9
PodCIDR:                      10.244.2.0/24
PodCIDRs:                     10.244.2.0/24
Non-terminated Pods:          (12 in total)
  Namespace                   Name                               CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                               ------------  ----------  ---------------  -------------  ---
  default                     guranteed-69875c65d6-97qh8         100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   50m
  default                     guranteed-69875c65d6-j4k8l         100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   1s
  default                     guranteed-69875c65d6-j5ptx         100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   55m
  default                     guranteed-69875c65d6-vbjgr         100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   50m
  default                     guranteed-69875c65d6-zmdc4         100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   50m
  default                     guranteed-69875c65d6-zxqc4         100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   50m
  kube-system                 cilium-envoy-5w4zk                 0 (0%)        0 (0%)      0 (0%)           0 (0%)         3d
  kube-system                 cilium-hg4mx                       100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         3d
  kube-system                 coredns-76f75df574-fg7t9           100m (2%)     0 (0%)      70Mi (1%)        170Mi (2%)     3d
  kube-system                 coredns-76f75df574-j7w94           100m (2%)     0 (0%)      70Mi (1%)        170Mi (2%)     3d
  kube-system                 kube-proxy-457qh                   0 (0%)        0 (0%)      0 (0%)           0 (0%)         3d
  kube-system                 metrics-server-7fb95cbc9b-86jr4    100m (2%)     0 (0%)      200Mi (3%)       0 (0%)         6h28m
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests       Limits
  --------           --------       ------
  cpu                1 (25%)        600m (15%)
  memory             6380Mi (107%)  6370Mi (107%)
  ephemeral-storage  0 (0%)         0 (0%)
  hugepages-1Gi      0 (0%)         0 (0%)
  hugepages-2Mi      0 (0%)         0 (0%)
Events:
  Type     Reason                     Age                From             Message
  ----     ------                     ----               ----             -------
  Normal   NodeNotReady               58m                node-controller  Node node2 status is now: NodeNotReady
  Normal   Starting                   58m                kubelet          Starting kubelet.
  Warning  InvalidDiskCapacity        58m                kubelet          invalid capacity 0 on image filesystem
  Normal   NodeAllocatableEnforced    58m                kubelet          Updated Node Allocatable limit across pods
  Normal   NodeHasSufficientMemory    58m (x3 over 58m)  kubelet          Node node2 status is now: NodeHasSufficientMemory
  Normal   NodeHasNoDiskPressure      58m (x3 over 58m)  kubelet          Node node2 status is now: NodeHasNoDiskPressure
  Normal   NodeHasSufficientPID       58m (x3 over 58m)  kubelet          Node node2 status is now: NodeHasSufficientPID
  Normal   NodeReady                  58m (x2 over 58m)  kubelet          Node node2 status is now: NodeReady
  Normal   Starting                   57m                kubelet          Starting kubelet.
  Warning  InvalidDiskCapacity        57m                kubelet          invalid capacity 0 on image filesystem
  Normal   NodeAllocatableEnforced    57m                kubelet          Updated Node Allocatable limit across pods
  Normal   NodeHasNoDiskPressure      57m                kubelet          Node node2 status is now: NodeHasNoDiskPressure
  Normal   NodeHasSufficientPID       57m                kubelet          Node node2 status is now: NodeHasSufficientPID
  Warning  EvictionThresholdMet       43m (x2 over 50m)  kubelet          Attempting to reclaim memory
  Normal   NodeHasInsufficientMemory  43m (x2 over 49m)  kubelet          Node node2 status is now: NodeHasInsufficientMemory
  Normal   NodeHasSufficientMemory    38m (x3 over 57m)  kubelet          Node node2 status is now: NodeHasSufficientMemory



tkr@hmc:~/memory-eater/k8s-yaml$ kubectl top node
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   294m         14%    1640Mi          42%       
node1     22m          0%     2679Mi          34%       
node2     1421m        35%    6380Mi          107%      


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
  RenewTime:       Mon, 28 Oct 2024 17:17:05 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:44 +0900   Fri, 25 Oct 2024 17:09:44 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Mon, 28 Oct 2024 17:15:16 +0900   Fri, 25 Oct 2024 17:08:51 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Mon, 28 Oct 2024 17:15:16 +0900   Fri, 25 Oct 2024 17:08:51 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Mon, 28 Oct 2024 17:15:16 +0900   Fri, 25 Oct 2024 17:08:51 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Mon, 28 Oct 2024 17:15:16 +0900   Fri, 25 Oct 2024 17:09:42 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
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
  Boot ID:                    5b66d783-c88d-4b75-b7cc-81298014ba08
  Kernel Version:             5.4.0-136-generic
  OS Image:                   Ubuntu 20.04.5 LTS
  Operating System:           linux
  Architecture:               amd64
  Container Runtime Version:  containerd://1.7.22
  Kubelet Version:            v1.29.9
  Kube-Proxy Version:         v1.29.9
PodCIDR:                      10.244.1.0/24
PodCIDRs:                     10.244.1.0/24
Non-terminated Pods:          (8 in total)
  Namespace                   Name                                         CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                                         ------------  ----------  ---------------  -------------  ---
  default                     besteffort-657f6c859c-hg77n                  0 (0%)        0 (0%)      0 (0%)           0 (0%)         56m
  default                     boostable-7f657bc6d7-grxf5                   100m (2%)     0 (0%)      100Mi (1%)       0 (0%)         49m
  ingress-nginx               ingress-nginx-controller-849dcdc99d-brswp    100m (2%)     0 (0%)      90Mi (1%)        0 (0%)         5h20m
  kube-system                 cilium-99gp6                                 100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         3d
  kube-system                 cilium-envoy-8vfrw                           0 (0%)        0 (0%)      0 (0%)           0 (0%)         3d
  kube-system                 kube-proxy-w2vsq                             0 (0%)        0 (0%)      0 (0%)           0 (0%)         3d
  kubernetes-dashboard        dashboard-metrics-scraper-db57c5c46-94cm5    0 (0%)        0 (0%)      0 (0%)           0 (0%)         6h33m
  kubernetes-dashboard        kubernetes-dashboard-5b47fd8cb5-gqmfk        0 (0%)        0 (0%)      0 (0%)           0 (0%)         6h33m
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests    Limits
  --------           --------    ------
  cpu                300m (7%)   0 (0%)
  memory             200Mi (2%)  0 (0%)
  ephemeral-storage  0 (0%)      0 (0%)
  hugepages-1Gi      0 (0%)      0 (0%)
  hugepages-2Mi      0 (0%)      0 (0%)
Events:              <none>

