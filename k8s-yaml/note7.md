root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Mon 2024-10-28 02:36:34 UTC; 2s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 13945 (kubelet)
      Tasks: 14 (limit: 9445)
     Memory: 55.5M
     CGroup: /system.slice/kubelet.service
             └─13945 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml --container-runtime-endpoint=unix:///var/run/co>


root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-28 02:36:29.066007984 +0000
@@ -43,3 +43,10 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+systemReserved:
+  memory: 1Gi
+kubeReserved:
+  memory: 1Gi
+evictionHard:
+  memory.available: 1Gi
+  


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
besteffort-657f6c859c-dbv2r BestEffort
boostable-7f657bc6d7-5n5tk Burstable
guranteed-69875c65d6-mm9lx Guaranteed

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP          NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-dbv2r   1/1     Running   0          36s   10.0.0.23   node2   <none>           <none>
boostable-7f657bc6d7-5n5tk    1/1     Running   0          35s   10.0.0.45   node2   <none>           <none>
guranteed-69875c65d6-mm9lx    1/1     Running   0          34s   10.0.0.57   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   80m          4%     1206Mi          31%       
node1     26m          0%     614Mi           7%        
node2     64m          1%     1845Mi          37%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-dbv2r   8m           602Mi           
boostable-7f657bc6d7-5n5tk    8m           601Mi           
guranteed-69875c65d6-mm9lx    7m           501Mi           
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP          NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-dbv2r   1/1     Running   0          2m19s   10.0.0.23   node2   <none>           <none>
boostable-7f657bc6d7-5n5tk    1/1     Running   0          2m18s   10.0.0.45   node2   <none>           <none>
guranteed-69875c65d6-mm9lx    1/1     Running   0          2m17s   10.0.0.57   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   85m          4%     1203Mi          31%       
node1     26m          0%     614Mi           7%        
node2     38m          0%     3651Mi          74%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-dbv2r   0m           1002Mi          
boostable-7f657bc6d7-5n5tk    0m           1002Mi          
guranteed-69875c65d6-mm9lx    0m           1002Mi       

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":3}}'

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-dbv2r   1/1     Running   0          4m15s   10.0.0.23    node2   <none>           <none>
boostable-7f657bc6d7-5n5tk    1/1     Running   0          4m14s   10.0.0.45    node2   <none>           <none>
guranteed-69875c65d6-cvhp2    1/1     Running   0          73s     10.0.0.185   node2   <none>           <none>
guranteed-69875c65d6-mm9lx    1/1     Running   0          4m13s   10.0.0.57    node2   <none>           <none>
guranteed-69875c65d6-slmmm    1/1     Running   0          73s     10.0.0.176   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   84m          4%     1204Mi          31%       
node1     26m          0%     614Mi           7%        
node2     45m          1%     5665Mi          115%      
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-dbv2r   0m           1002Mi          
boostable-7f657bc6d7-5n5tk    0m           1002Mi          
guranteed-69875c65d6-cvhp2    0m           1002Mi          
guranteed-69875c65d6-mm9lx    0m           1002Mi          
guranteed-69875c65d6-slmmm    0m           1002Mi       

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":4}}'
deployment.apps/guranteed patched

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-dbv2r   1/1     Running   0          6m20s   10.0.0.23    node2   <none>           <none>
boostable-7f657bc6d7-5n5tk    1/1     Running   0          6m19s   10.0.0.45    node2   <none>           <none>
guranteed-69875c65d6-cvhp2    1/1     Running   0          3m18s   10.0.0.185   node2   <none>           <none>
guranteed-69875c65d6-mm9lx    1/1     Running   0          6m18s   10.0.0.57    node2   <none>           <none>
guranteed-69875c65d6-slmmm    1/1     Running   0          3m18s   10.0.0.176   node2   <none>           <none>
guranteed-69875c65d6-sndvn    1/1     Running   0          76s     10.0.0.216   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   85m          4%     1204Mi          31%       
node1     25m          0%     614Mi           7%        
node2     44m          1%     6673Mi          136%      
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-dbv2r   0m           1002Mi          
boostable-7f657bc6d7-5n5tk    0m           1002Mi          
guranteed-69875c65d6-cvhp2    0m           1002Mi          
guranteed-69875c65d6-mm9lx    0m           1002Mi          
guranteed-69875c65d6-slmmm    0m           1002Mi          
guranteed-69875c65d6-sndvn    0m           1002Mi  



tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS        RESTARTS   AGE    IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-dbv2r   1/1     Running       0          7m7s   10.0.0.23    node2   <none>           <none>
boostable-7f657bc6d7-5n5tk    1/1     Running       0          7m6s   10.0.0.45    node2   <none>           <none>
guranteed-69875c65d6-24cln    0/1     OutOfmemory   0          2s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2jpxx    0/1     OutOfmemory   0          11s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2mrw4    0/1     OutOfmemory   0          4s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2q47p    0/1     OutOfmemory   0          21s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2vpmb    0/1     OutOfmemory   0          11s    <none>       node2   <none>           <none>
guranteed-69875c65d6-47fgr    0/1     OutOfmemory   0          16s    <none>       node2   <none>           <none>
guranteed-69875c65d6-49ctq    0/1     OutOfmemory   0          16s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4b7ds    0/1     OutOfmemory   0          8s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4fbql    0/1     OutOfmemory   0          9s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4jwvb    0/1     OutOfmemory   0          14s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4m4nh    0/1     OutOfmemory   0          22s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4q2c8    0/1     OutOfmemory   0          12s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4trm6    0/1     OutOfmemory   0          13s    <none>       node2   <none>           <none>

...

NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   257m         12%    1208Mi          31%       
node1     25m          0%     613Mi           7%        
node2     76m          1%     6676Mi          136%      
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-dbv2r   0m           1002Mi          
boostable-7f657bc6d7-5n5tk    0m           1002Mi          
guranteed-69875c65d6-cvhp2    0m           1002Mi          
guranteed-69875c65d6-mm9lx    0m           1002Mi          
guranteed-69875c65d6-slmmm    0m           1002Mi          
guranteed-69875c65d6-sndvn    0m           1002Mi       


これは良くない？
+systemReserved: +kubeReserved:を除去して、以下のシンプルな設定でトライ

root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-28 02:47:21.201055517 +0000
@@ -43,3 +43,6 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+evictionHard:
+  memory.available: 1Gi
+  

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.capacity.memory, .status.allocatable.memory" | xargs -n3
master1 4026044Ki 3923644Ki
node1 8150236Ki 8047836Ki
node2 8150228Ki 7101652Ki

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
[
  {
    "effect": "NoSchedule",
    "key": "node.kubernetes.io/memory-pressure",
    "timeAdded": "2024-10-28T02:48:33Z"
  }
]
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl delete -f deployment-boostable.yaml^C
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
besteffort-657f6c859c-ngzc9 BestEffort
boostable-7f657bc6d7-slxf7 Burstable
guranteed-69875c65d6-k25b7 Guaranteed

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-ngzc9   1/1     Running   0          34s   10.0.0.179   node2   <none>           <none>
boostable-7f657bc6d7-slxf7    1/1     Running   0          33s   10.0.0.114   node2   <none>           <none>
guranteed-69875c65d6-k25b7    1/1     Running   0          33s   10.0.0.211   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   93m          4%     1312Mi          34%       
node1     25m          0%     613Mi           7%        
node2     107m         2%     1603Mi          23%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-ngzc9   8m           401Mi           
boostable-7f657bc6d7-slxf7    8m           401Mi           
guranteed-69875c65d6-k25b7    10m          401Mi           
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted



tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS                   RESTARTS   AGE    IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-l2hbp   1/1     Running                  0          78s    10.0.2.72    node1   <none>           <none>
besteffort-657f6c859c-ngzc9   0/1     Error                    0          3m9s   10.0.0.179   node2   <none>           <none>
boostable-7f657bc6d7-f4z6m    1/1     Running                  0          63s    10.0.2.68    node1   <none>           <none>
boostable-7f657bc6d7-slxf7    0/1     ContainerStatusUnknown   1          3m8s   10.0.0.114   node2   <none>           <none>
guranteed-69875c65d6-6qp95    1/1     Running                  0          2m4s   10.0.0.163   node2   <none>           <none>
guranteed-69875c65d6-794pv    1/1     Running                  0          2m4s   10.0.0.168   node2   <none>           <none>
guranteed-69875c65d6-dscrv    1/1     Running                  0          2m4s   10.0.0.230   node2   <none>           <none>
guranteed-69875c65d6-k25b7    1/1     Running                  0          3m8s   10.0.0.211   node2   <none>           <none>
guranteed-69875c65d6-ml6xv    1/1     Running                  0          2m4s   10.0.0.73    node2   <none>           <none>
guranteed-69875c65d6-xx5px    1/1     Running                  0          2m4s   10.0.0.112   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   87m          4%     1300Mi          33%       
node1     34m          0%     2708Mi          34%       
node2     52m          1%     6547Mi          94%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-l2hbp   0m           1002Mi          
boostable-7f657bc6d7-f4z6m    3m           1002Mi          
guranteed-69875c65d6-6qp95    0m           1002Mi          
guranteed-69875c65d6-794pv    0m           1002Mi          
guranteed-69875c65d6-dscrv    0m           1002Mi          
guranteed-69875c65d6-k25b7    0m           1002Mi          
guranteed-69875c65d6-ml6xv    0m           1002Mi          
guranteed-69875c65d6-xx5px    0m           1002Mi         