
root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-28 01:03:25.793008992 +0000
@@ -43,3 +43,12 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+systemReserved:
+  memory: 1Gi
+kubeReserved:
+  memory: 1Gi
+evictionSoft:
+  memory.available: 10%
+evictionSoftGracePeriod:
+  memory.available: 30s
+  


root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Mon 2024-10-28 01:04:14 UTC; 7s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 7216 (kubelet)
      Tasks: 13 (limit: 9445)
     Memory: 59.5M
     CGroup: /system.slice/kubelet.service
             └─7216 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml --container-runtime-endpoint=unix:///var/run/con>



tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.capacity.memory, .status.allocatable.memory" | xargs -n3
master1 4026044Ki 3923644Ki
node1 8150236Ki 8047836Ki
node2 8150228Ki 5950676Ki


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
No resources found in default namespace.
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   79m          3%     996Mi           26%       
node1     54m          1%     767Mi           9%        
node2     30m          0%     484Mi           8%        
No resources found in default namespace.



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
besteffort-657f6c859c-mfsh2 BestEffort
boostable-7f657bc6d7-8l2g6 Burstable
guranteed-69875c65d6-pvt8m Guaranteed


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE   IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-mfsh2   1/1     Running   0          32s   10.0.0.187   node2   <none>           <none>
boostable-7f657bc6d7-8l2g6    1/1     Running   0          31s   10.0.0.86    node2   <none>           <none>
guranteed-69875c65d6-pvt8m    1/1     Running   0          30s   10.0.0.188   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   98m          4%     996Mi           26%       
node1     36m          0%     764Mi           9%        
node2     98m          2%     1397Mi          24%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-mfsh2   8m           401Mi           
boostable-7f657bc6d7-8l2g6    8m           401Mi           
guranteed-69875c65d6-pvt8m    10m          401Mi


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-mfsh2   1/1     Running   0          6m16s   10.0.0.187   node2   <none>           <none>
boostable-7f657bc6d7-8l2g6    1/1     Running   0          6m15s   10.0.0.86    node2   <none>           <none>
guranteed-69875c65d6-pvt8m    1/1     Running   0          6m14s   10.0.0.188   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   78m          3%     998Mi           26%       
node1     36m          0%     763Mi           9%        
node2     28m          0%     3502Mi          60%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-mfsh2   0m           1002Mi          
boostable-7f657bc6d7-8l2g6    0m           1002Mi          
guranteed-69875c65d6-pvt8m    0m           1002Mi     


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":6}}'
deployment.apps/guranteed patched
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS        RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-mfsh2   1/1     Running       0          7m38s   10.0.0.187   node2   <none>           <none>
boostable-7f657bc6d7-8l2g6    1/1     Running       0          7m37s   10.0.0.86    node2   <none>           <none>
guranteed-69875c65d6-29kjs    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-2d2ms    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-2lr6j    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-2nmst    0/1     OutOfmemory   0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2pl9p    0/1     OutOfmemory   0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-42m22    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4hd9l    0/1     OutOfmemory   0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4lztx    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4qtmq    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4w9d6    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-57tp9    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-5m6w7    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-5ncxb    0/1     OutOfmemory   0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-648h8    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-6bcwd    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-6bsjg    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6h8sv    0/1     OutOfmemory   0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6nh7f    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-6phnh    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-6t2x4    0/1     OutOfmemory   0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-74htn    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-7jt6z    0/1     OutOfmemory   0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7l2cr    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-87rzl    1/1     Running       0          12s     10.0.0.113   node2   <none>           <none>
guranteed-69875c65d6-8brw6    0/1     OutOfmemory   0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8cjk4    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8hjv9    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8jr7t    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8p47b    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8ssgq    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8xslq    1/1     Running       0          12s     10.0.0.109   node2   <none>           <none>
guranteed-69875c65d6-9f7ln    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-9fgwg    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-9g9tp    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-9t5l7    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-9zvqd    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bpjdt    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bpmp4    0/1     OutOfmemory   0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bthhr    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bvjr5    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bxw9q    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-c82kp    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-cbphj    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-cm2gp    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-cmjwg    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-cmxmb    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cq49p    0/1     OutOfmemory   0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cqpdn    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-dwmt2    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-dzcw7    0/1     OutOfmemory   0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-f42j5    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-fq6nx    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-ft89g    0/1     OutOfmemory   0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-g6xjb    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g7s46    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-g9vl4    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-hdvr4    0/1     OutOfmemory   0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-j9svw    0/1     OutOfmemory   0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jjdkl    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jnfbb    0/1     OutOfmemory   0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jswq9    0/1     OutOfmemory   0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jwwv9    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-k26cd    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-k7s7x    0/1     OutOfmemory   0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kcjg7    0/1     OutOfmemory   0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kdlld    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kg9ck    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kggqw    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-khvm4    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kqzt6    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kw59f    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-lbx8r    0/1     OutOfmemory   0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lhkvg    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-lrc2w    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-ltmqt    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-m6km6    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-nngs9    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-nzns5    0/1     OutOfmemory   0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-pc9m7    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-pncj2    0/1     OutOfmemory   0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-pngkc    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-pq5zx    0/1     OutOfmemory   0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-ptqcb    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-pvt8m    1/1     Running       0          7m36s   10.0.0.188   node2   <none>           <none>
guranteed-69875c65d6-q4g9x    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-qbdq5    1/1     Running       0          12s     10.0.0.96    node2   <none>           <none>
guranteed-69875c65d6-qh6zz    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qww6h    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rbr2z    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-s2kwg    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sjx7x    0/1     OutOfmemory   0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-spfm2    0/1     OutOfmemory   0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-spqvs    0/1     OutOfmemory   0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-stp6x    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-sttfc    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-t495w    0/1     OutOfmemory   0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-t8ccx    1/1     Running       0          12s     10.0.0.143   node2   <none>           <none>
guranteed-69875c65d6-tqpgw    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vt5ks    0/1     OutOfmemory   0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vtxf4    0/1     OutOfmemory   0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-w4gfv    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-w5x7b    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-wb98l    0/1     OutOfmemory   0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wjwm2    0/1     OutOfmemory   0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-wm7ln    0/1     OutOfmemory   0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-ws5hk    0/1     OutOfmemory   0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xl7cs    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-xndkd    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-xvd97    0/1     OutOfmemory   0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-xvhxf    0/1     OutOfmemory   0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-zb977    0/1     OutOfmemory   0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-zc46l    0/1     OutOfmemory   0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-zlcrw    0/1     OutOfmemory   0          7s      <none>       node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   145m         7%     1004Mi          26%       
node1     37m          0%     763Mi           9%        
node2     127m         3%     3570Mi          61%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-mfsh2   0m           1002Mi          
boostable-7f657bc6d7-8l2g6    0m           1002Mi          
guranteed-69875c65d6-pvt8m    0m           1002Mi     




tkr@hmc:~$ kubectl get pod -o wide && kubectl top node && kubectl top pod 
NAME                          READY   STATUS                   RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-b4gnx   1/1     Running                  0          3m      10.0.2.194   node1   <none>           <none>
besteffort-657f6c859c-mfsh2   0/1     ContainerStatusUnknown   1          11m     10.0.0.187   node2   <none>           <none>
boostable-7f657bc6d7-8l2g6    1/1     Running                  0          11m     10.0.0.86    node2   <none>           <none>
guranteed-69875c65d6-225f9    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-22hqv    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-22m2g    0/1     OutOfmemory              0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-24tpq    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-25kjj    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-25xqf    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-264gs    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-26bzf    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-26m4n    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-26mvz    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-282hd    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-285wr    0/1     OutOfmemory              0          3m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-28c6q    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-28mnd    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-299qc    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-29dcj    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-29hhg    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-29kjs    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2b4x4    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2bxhj    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2bxxc    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2c7bw    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2cckk    0/1     OutOfmemory              0          2m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2cfhg    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2ch8w    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2d2ms    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2d78x    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2dp76    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2dzb2    0/1     OutOfmemory              0          36s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2f2kn    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2fjl5    0/1     OutOfmemory              0          62s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2gbwv    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2ggtv    0/1     OutOfmemory              0          3m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2gkd5    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2glrt    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2gsfj    0/1     OutOfmemory              0          2m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2h5xm    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2hfsd    0/1     OutOfmemory              0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-2hw8h    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2jgvl    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2kc5n    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2kjht    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2kmhc    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2l5sf    0/1     OutOfmemory              0          3m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2lnxf    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2lr6j    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2m6vr    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2m98q    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2mg6d    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2mgv7    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2mjbg    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2mpsb    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2nkqp    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2nlsz    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2nmst    0/1     OutOfmemory              0          4m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2nxqd    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2p86k    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2pj2j    0/1     OutOfmemory              0          3m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2pl9p    0/1     OutOfmemory              0          4m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2q925    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2qbg2    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2qgwc    0/1     OutOfmemory              0          29s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2qvwq    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2r8mv    0/1     OutOfmemory              0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-2rd7f    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2rk5l    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2rkz2    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2sdrc    0/1     OutOfmemory              0          2m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2sj9d    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2sm5c    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2sxb9    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2sxbx    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2tjww    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2tr9m    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2tt6t    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2txsn    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2vpbw    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2vz6k    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2wspt    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2x5dg    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-2xgwz    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-2xpk5    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2xtpb    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-2z9wj    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-428r8    0/1     OutOfmemory              0          3m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-42kz6    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-42m22    0/1     OutOfmemory              0          4m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-42tzc    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-444h2    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-448ld    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-44d7r    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-44hmb    0/1     OutOfmemory              0          2m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-44kht    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-44prb    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-45j7t    0/1     OutOfmemory              0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-45n55    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-45zsj    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4648t    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-46bz8    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-46n6z    0/1     OutOfmemory              0          2m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-46rl9    0/1     OutOfmemory              0          3m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-46tvg    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-47m4t    0/1     OutOfmemory              0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-47mlx    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-484bw    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-486hn    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-48qjp    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-49sz7    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-49zbn    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4b7kk    0/1     OutOfmemory              0          34s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4c5hb    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4c6jf    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4c77b    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4cjsv    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4d6wr    0/1     OutOfmemory              0          73s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4d78j    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-4dcx6    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4dmss    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4ff8h    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4g5cx    0/1     OutOfmemory              0          40s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4g5dd    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4gn57    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4hd9l    0/1     OutOfmemory              0          4m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4hkhb    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4jbfs    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4jdnd    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4jtwr    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4jwb4    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4k5bx    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4kcdv    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4kl65    0/1     OutOfmemory              0          2m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4l6wc    0/1     OutOfmemory              0          3m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4lztx    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4m5x2    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4mdcz    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4mfrm    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4mmk9    0/1     OutOfmemory              0          29s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4mpxp    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4ng4f    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4nkmz    0/1     OutOfmemory              0          98s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4nnw2    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4nrjl    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4p7wv    0/1     OutOfmemory              0          86s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4pqw6    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4q27t    0/1     OutOfmemory              0          66s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4q7xl    0/1     OutOfmemory              0          14s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4q877    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4qhgr    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4qr84    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4qtmq    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4r7xh    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4r848    0/1     OutOfmemory              0          34s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4rfc7    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4rj4v    0/1     OutOfmemory              0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-4s5jz    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4tdmb    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4tftd    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4tq5v    0/1     OutOfmemory              0          3m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4vr7l    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4vxbb    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4w8th    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4w9d6    0/1     OutOfmemory              0          4m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4wgtj    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4wmls    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4x46m    0/1     OutOfmemory              0          3m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4xgpz    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4xlg9    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-4z95d    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-4zn9b    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-4zthg    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-524x4    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-52bhm    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-52xsb    0/1     OutOfmemory              0          2m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-54ffp    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-54q8j    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-55kl5    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-55s7k    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5656n    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-56cpm    0/1     OutOfmemory              0          118s    <none>       node2   <none>           <none>
guranteed-69875c65d6-56snb    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-57787    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-577zj    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-579vp    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-57f6b    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-57h2j    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-57hn6    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-57tp9    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-57wvf    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-57xkr    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-587f4    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-58cvh    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-58tbt    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-58v4n    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-596kf    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5982d    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-59trp    0/1     OutOfmemory              0          2m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-59w9j    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5bx6z    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5c756    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5ck2q    0/1     OutOfmemory              0          74s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5clc2    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5clsk    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5ddn9    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5dhkf    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5dp45    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5f8wj    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5f9s7    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5g9vl    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5gjdg    0/1     OutOfmemory              0          118s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5gtns    0/1     OutOfmemory              0          3m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5hhnq    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5hhs2    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5hj2z    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5hjzm    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5hnjm    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5htvn    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5j6kp    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5j79b    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5j9pp    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5jplx    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5jwsn    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5k5zx    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5kfjk    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5lck5    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5lcwh    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5lq5z    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5lrbf    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5lvqj    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5m6fx    0/1     OutOfmemory              0          3m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5m6w7    0/1     OutOfmemory              0          4m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5mkh9    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5n9lx    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5ncxb    0/1     OutOfmemory              0          4m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5nx7z    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5p5n5    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5p95l    0/1     OutOfmemory              0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-5p9wh    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5pj4n    0/1     OutOfmemory              0          2m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5pn7v    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5ptjh    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5qbnl    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5ql95    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5rt9j    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5rwhw    0/1     OutOfmemory              0          34s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5s464    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5sj8m    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5tlw8    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5trr4    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5ttlc    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5vfw9    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5vh2l    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5vskn    0/1     OutOfmemory              0          52s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5w4hd    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5ww2j    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5x86v    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5xl9k    0/1     OutOfmemory              0          4m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5xn87    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5xsl9    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5xxgn    0/1     OutOfmemory              0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-5z7d6    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5z8g6    0/1     OutOfmemory              0          118s    <none>       node2   <none>           <none>
guranteed-69875c65d6-5zf6x    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5zq79    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-5zqcm    0/1     OutOfmemory              0          86s     <none>       node2   <none>           <none>
guranteed-69875c65d6-5zqxb    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-628lg    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-62wx9    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-648bl    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-648c7    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-648h8    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-64pwf    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-655x7    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-65kh4    0/1     OutOfmemory              0          3m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-66bp5    0/1     OutOfmemory              0          98s     <none>       node2   <none>           <none>
guranteed-69875c65d6-66wjh    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-67858    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-67chj    0/1     OutOfmemory              0          2m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-67ql6    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-67shd    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6876x    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-68g8f    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-68wsw    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-69bz2    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-69cr7    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6b5zg    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6bcwd    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6bqbj    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6bqch    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6bsjg    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6c5hl    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6cdkl    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6ctgt    0/1     OutOfmemory              0          2m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6d6ng    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6dcxw    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6df25    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6dfg9    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6djnl    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6dr5f    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6fjwt    0/1     OutOfmemory              0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6g52z    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6g9pp    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6ggr2    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6gjvf    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6gm5h    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6gp9d    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6h2fk    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6h8sv    0/1     OutOfmemory              0          4m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6hrn6    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6jj2k    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6jjqc    0/1     OutOfmemory              0          3m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6jvmr    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6jvsv    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6kkvv    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6kqs4    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6krbb    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6lcmx    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6lhzx    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6ljzr    0/1     OutOfmemory              0          74s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6lrdj    0/1     OutOfmemory              0          85s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6m9x6    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6mbcz    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-6mbs2    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6mm46    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6mmjr    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6mp27    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6n9t7    0/1     OutOfmemory              0          2m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6nbn9    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6ngsf    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6nh7f    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6nj9m    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6nq7q    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6p6kb    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6p6pw    0/1     OutOfmemory              0          4m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6phj5    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-6phnh    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6q27b    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6q5nx    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6qd2z    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6qvcj    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6r8ps    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6r9hz    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6rwk2    0/1     OutOfmemory              0          45s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6s2d7    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6sjvg    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6t2x4    0/1     OutOfmemory              0          4m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6t5cz    0/1     OutOfmemory              0          36s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6t9cn    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6tmlb    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6v8j5    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6wf7q    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6wn7k    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6x6hb    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6x9bk    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6xhzd    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-6z6cr    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-6z6pp    0/1     OutOfmemory              0          4m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6zc4c    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-6zj27    0/1     OutOfmemory              0          4m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7275z    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-72kfd    0/1     OutOfmemory              0          118s    <none>       node2   <none>           <none>
guranteed-69875c65d6-72kqn    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-72lvt    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-72vbb    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-74cmt    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-74htn    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-74l8z    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-74slr    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-75k77    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-75n7d    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-75r96    0/1     OutOfmemory              0          3m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7672g    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-77gk2    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-77q7c    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-77qzf    0/1     OutOfmemory              0          25s     <none>       node2   <none>           <none>
guranteed-69875c65d6-788pr    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-78h88    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-78rng    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-794f6    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-79j7d    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-79mgl    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7b72m    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7bnfg    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7c22h    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7cbr6    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7cdhb    0/1     OutOfmemory              0          3m      <none>       node2   <none>           <none>
guranteed-69875c65d6-7cf2p    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7cgpn    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7cljd    0/1     OutOfmemory              0          47s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7cllb    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7ct79    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7d89x    0/1     OutOfmemory              0          94s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7dl48    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7dvc2    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7dvzh    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7dwts    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7dz8v    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7f5rp    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7fp98    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7fqtm    0/1     OutOfmemory              0          3m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7fwfd    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7g4pz    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7g7l5    0/1     OutOfmemory              0          2m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7gdwh    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7ggp9    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7gtb9    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7h4ls    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7j7jc    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7j7mm    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7j84z    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7jcm7    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7jt6z    0/1     OutOfmemory              0          4m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7jv2c    0/1     OutOfmemory              0          47s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7k6l6    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7kch4    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7kcqt    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7kjjz    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7klkd    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7l2cr    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7l7cq    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7lbzm    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7lgq9    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7ml5p    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7mmk9    0/1     OutOfmemory              0          93s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7n492    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7n7xk    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7ncjv    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7ndh5    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7nmdd    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7nxrb    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7p4hb    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7pkh5    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7pzl7    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7q6sl    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7q8ld    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7qghx    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7qhl7    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7qqv7    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7qxx6    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7r96v    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7rs8c    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7sddz    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7srgm    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7tk52    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7tk9w    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7tzdp    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7vdpb    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7w58l    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7w87t    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7wjx9    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7xb5h    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7xdzt    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-7xftl    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-7xzvv    0/1     OutOfmemory              0          86s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7zqkx    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-7ztsc    0/1     OutOfmemory              0          98s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8257g    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8477w    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-84955    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-852nz    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-858lc    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-859c9    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-85st8    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-865p6    0/1     OutOfmemory              0          3m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-86mhv    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-86qc9    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-875jr    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-87lfq    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-87rzl    1/1     Running                  0          4m22s   10.0.0.113   node2   <none>           <none>
guranteed-69875c65d6-87vrt    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-886dj    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-88mss    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-897wc    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-89h7g    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-89v7v    0/1     OutOfmemory              0          40s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8bh7k    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8bhqv    0/1     OutOfmemory              0          94s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8bj75    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8brw6    0/1     OutOfmemory              0          4m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8btmv    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8bz7v    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8cjk4    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8ckdg    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8cnxm    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8d547    0/1     OutOfmemory              0          86s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8d6gj    0/1     OutOfmemory              0          14s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8d8kj    0/1     OutOfmemory              0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8dpbt    0/1     OutOfmemory              0          3m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8dqdb    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8f2fv    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8fpvm    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8ft4q    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8g276    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8grrs    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8hjv9    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8hjxd    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8j7lb    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8jr7t    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8jwtj    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8kgh4    0/1     OutOfmemory              0          74s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8klwh    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8kn5b    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8krjf    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8krtz    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8kwvv    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8lxch    0/1     OutOfmemory              0          3m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8m7mf    0/1     OutOfmemory              0          14s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8mbrs    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8n4dv    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8p47b    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8p77v    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8p85k    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8p8vq    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8q2hg    0/1     OutOfmemory              0          3m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8q8mj    0/1     OutOfmemory              0          3m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8qhb8    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8qpc6    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8qpph    0/1     OutOfmemory              0          73s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8qv2v    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8rdsr    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8rjnh    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8rk59    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8rlm2    0/1     OutOfmemory              0          29s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8rnql    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8s8tl    0/1     OutOfmemory              0          2m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8sc86    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8smmh    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-8sqhz    0/1     OutOfmemory              0          25s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8sqqp    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8srl5    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8ssgq    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8t598    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8t6cx    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8tgk6    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8vlt7    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8wdm6    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8wn8z    0/1     OutOfmemory              0          25s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8wsnb    0/1     OutOfmemory              0          14s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8wv64    0/1     OutOfmemory              0          47s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8x4bd    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8xrrm    0/1     OutOfmemory              0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-8xs7x    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-8xslq    1/1     Running                  0          4m22s   10.0.0.109   node2   <none>           <none>
guranteed-69875c65d6-8zbp2    0/1     OutOfmemory              0          98s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8zjhv    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-8zs5r    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-92tjn    0/1     OutOfmemory              0          99s     <none>       node2   <none>           <none>
guranteed-69875c65d6-94bvk    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-956zr    0/1     OutOfmemory              0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-95g5s    0/1     OutOfmemory              0          4m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-95vm2    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-96t7q    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-96wrq    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9788m    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-97fqd    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-97n6t    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-97p5r    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-982md    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-984z5    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-986wk    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-98j5z    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-98k2z    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-98t2g    0/1     OutOfmemory              0          2m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-98wx4    0/1     OutOfmemory              0          15s     <none>       node2   <none>           <none>
guranteed-69875c65d6-996tc    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-99bvp    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-99lmb    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-99njt    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9bbf7    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9bskr    0/1     OutOfmemory              0          2m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9bxwh    0/1     OutOfmemory              0          2m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9cdt7    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9cjf2    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9cpt5    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9cxcs    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9ds5x    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9f7ln    0/1     OutOfmemory              0          4m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9fgwg    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9g6ps    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9g9tp    0/1     OutOfmemory              0          4m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9gc6m    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9gk52    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9glsd    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9h4nl    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9h7c7    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9hbs5    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9hptb    0/1     OutOfmemory              0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-9hpxq    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9hwm5    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9jkgs    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9jqq4    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9kdgx    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9l9kd    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9m7xm    0/1     OutOfmemory              0          98s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9mp22    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9n45j    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9nbr9    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9nnbn    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9nplq    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9p8rk    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9pkmb    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9pmbl    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9pmz5    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9pr5q    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9q7hp    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9r24n    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9rkzw    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9rn8q    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9rpxl    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9sdzl    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9sq8c    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9swg5    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9t5l7    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9tt27    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9twpl    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9vg2c    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9vlvr    0/1     OutOfmemory              0          2m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9vlwj    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9vqtt    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9vtd5    0/1     OutOfmemory              0          2m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9wkp2    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9wmzs    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9wn5b    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9wszs    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9wwwl    0/1     OutOfmemory              0          2m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9wwxv    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9wxdt    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-9x2hm    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9x5qn    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9xdlt    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9xzr5    0/1     OutOfmemory              0          29s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9zgm2    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9zq4x    0/1     OutOfmemory              0          62s     <none>       node2   <none>           <none>
guranteed-69875c65d6-9zsbm    0/1     OutOfmemory              0          2m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-9zvqd    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-b296b    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-b2wbd    0/1     OutOfmemory              0          78s     <none>       node2   <none>           <none>
guranteed-69875c65d6-b2wjl    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-b482r    0/1     OutOfmemory              0          85s     <none>       node2   <none>           <none>
guranteed-69875c65d6-b4dm6    0/1     OutOfmemory              0          66s     <none>       node2   <none>           <none>
guranteed-69875c65d6-b557l    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-b5dht    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-b5fqk    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-b5mrf    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-b5sfk    0/1     OutOfmemory              0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-b5tmv    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-b6djv    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-b6ksw    0/1     OutOfmemory              0          27s     <none>       node2   <none>           <none>
guranteed-69875c65d6-b6wgg    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-b7vt7    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-b8v6n    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-b97nc    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-b9hh7    0/1     OutOfmemory              0          2m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bbdxk    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bblnl    0/1     OutOfmemory              0          4m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-bbqcz    0/1     OutOfmemory              0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bc58g    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bc62z    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bcfrj    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bcl7m    0/1     OutOfmemory              0          2m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bd8z9    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bdgdm    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-bdjnw    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bdqvz    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bdvnc    0/1     OutOfmemory              0          25s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bf756    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bf7pg    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bf9pb    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bfd8g    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bfm94    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-bfpc4    0/1     OutOfmemory              0          2m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bg8nh    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-bhb98    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-bj2zm    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bjhnc    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bjvhd    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bkk59    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-blbz2    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-blw6z    0/1     OutOfmemory              0          25s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bmjzc    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-bmwvd    0/1     OutOfmemory              0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-bn77f    0/1     OutOfmemory              0          45s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bn96k    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bn9d5    0/1     OutOfmemory              0          27s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bnqlw    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bpgcj    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bphms    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bpjdt    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bpmp4    0/1     OutOfmemory              0          4m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bq4cg    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-brc9l    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-brw7z    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bs2gb    0/1     OutOfmemory              0          2m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bs59t    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bs5bl    0/1     OutOfmemory              0          4m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-bsb6w    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bsmx5    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bsz6k    0/1     OutOfmemory              0          3m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bt7dt    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bthhr    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-btmc2    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bv7nz    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bvg4k    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bvjr5    0/1     OutOfmemory              0          4m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bw2lg    0/1     OutOfmemory              0          85s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bwcqk    0/1     OutOfmemory              0          93s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bwv9j    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bwvv5    0/1     OutOfmemory              0          2m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bx4n5    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bx585    0/1     OutOfmemory              0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-bxnhb    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bxw9q    0/1     OutOfmemory              0          4m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bxx9m    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bz4gr    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-bzsq5    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-c222r    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-c2brb    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-c46qp    0/1     OutOfmemory              0          2m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c4vld    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c4wv4    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c57sm    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c5wrb    0/1     OutOfmemory              0          2m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c69mh    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-c6hxv    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c6nbv    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-c6twh    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c7mz4    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-c82kp    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c8kjc    0/1     OutOfmemory              0          2m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c8nn5    0/1     OutOfmemory              0          2m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-c92m4    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-c972t    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-c97xj    0/1     OutOfmemory              0          4m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cb6h9    0/1     OutOfmemory              0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-cbckw    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cblhh    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cbphj    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cbr5v    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cc48z    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cc5k9    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ccftz    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ccg8m    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ccxmm    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cczhl    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cd5ds    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cdbb4    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cdh9k    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cfnqg    0/1     OutOfmemory              0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-cfxbq    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cgj5j    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ch6r2    0/1     OutOfmemory              0          2m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ch6rc    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-chclp    0/1     OutOfmemory              0          94s     <none>       node2   <none>           <none>
guranteed-69875c65d6-chfbx    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-chhjl    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-chmpj    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-chtwp    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cj29l    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cjg4r    0/1     OutOfmemory              0          3m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ck574    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ck7x9    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cklz9    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ckrwg    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ckxvt    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cl2fm    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-clp5m    0/1     OutOfmemory              0          74s     <none>       node2   <none>           <none>
guranteed-69875c65d6-clxmh    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cm2gp    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cmbtw    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-cmj6k    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cmjwg    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cmpdk    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cmxmb    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cmzp6    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cncqp    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cnxlw    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cq49p    0/1     OutOfmemory              0          4m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cqbn8    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cqjt6    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cqpdn    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cqqk9    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cr6b8    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cr7m2    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-crdc9    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-crh7x    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-csjlh    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cskrs    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cszrm    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cth48    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ctkwq    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cttzp    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cv5xz    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cv8mc    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cv9vs    0/1     OutOfmemory              0          3m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cvnqm    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cw2mb    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cw6j8    0/1     OutOfmemory              0          2m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cwcl2    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cwj25    0/1     OutOfmemory              0          27s     <none>       node2   <none>           <none>
guranteed-69875c65d6-cwklz    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-cx4qx    0/1     OutOfmemory              0          2m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cxntf    0/1     OutOfmemory              0          3m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-cz8b7    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-czpjw    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-d24r5    0/1     OutOfmemory              0          85s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d27dx    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d2g92    0/1     OutOfmemory              0          99s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d2kw7    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-d2nxr    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-d4gpd    0/1     OutOfmemory              0          3m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-d4xmh    0/1     OutOfmemory              0          34s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d569s    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d57gm    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d5cbd    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-d5s5z    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-d648w    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d6n55    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-d6qz9    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-d6sqs    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-d7bcb    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d7bg2    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-d7fwf    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d7gxb    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-d7jmb    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d8brj    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d8fps    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-d8gl7    0/1     OutOfmemory              0          88s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d9mzg    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d9q24    0/1     OutOfmemory              0          3m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-d9qnj    0/1     OutOfmemory              0          98s     <none>       node2   <none>           <none>
guranteed-69875c65d6-d9w87    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-db5rf    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-db9cq    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dbj74    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dbjkr    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dc6zc    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dd864    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ddk8g    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ddq9g    0/1     OutOfmemory              0          2m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ddqqx    0/1     OutOfmemory              0          85s     <none>       node2   <none>           <none>
guranteed-69875c65d6-df8qn    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dfhws    0/1     OutOfmemory              0          99s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dfnh2    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dgbn6    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dgjgx    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dgz7s    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dhhjg    0/1     OutOfmemory              0          14s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dhlqx    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dhpvs    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dhqf7    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-dhzcm    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dj92k    0/1     OutOfmemory              0          3m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-djf5s    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-djr7w    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-djtff    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dk8nv    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dkbxk    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dkrpp    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-dkxrm    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dlkm2    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dlkwv    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dmgdb    0/1     OutOfmemory              0          27s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dmpng    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dnctb    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dnql4    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dnzm2    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dp9ql    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dpkkp    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dq52j    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dqkbb    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dqst4    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dqvlx    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-drfn5    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-drq8z    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ds77r    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dsb97    0/1     OutOfmemory              0          86s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dsf7p    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dsjmd    0/1     OutOfmemory              0          99s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dsz9k    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dt4rw    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dt5rt    0/1     OutOfmemory              0          34s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dtjjd    0/1     OutOfmemory              0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-dtnr5    0/1     OutOfmemory              0          14s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dttmd    0/1     OutOfmemory              0          94s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dv6gv    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dvdnv    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dwmt2    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dwtmj    0/1     OutOfmemory              0          62s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dww2m    0/1     OutOfmemory              0          2m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dxj2j    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-dxlt4    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dz4jn    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dz7fd    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dz9rd    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-dzcw7    0/1     OutOfmemory              0          4m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dzfw6    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-dzlmz    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-dzm4k    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f275p    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-f42j5    0/1     OutOfmemory              0          4m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f46rt    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f4glm    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-f4lnk    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-f4qp8    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-f4xnv    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-f5znv    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-f68dl    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f6rm7    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f6s6r    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-f6x62    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f766m    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f7lxd    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-f7rz5    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f7t5s    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f96pd    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f9pnv    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-f9xvv    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fbctz    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fbjbw    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fbqjh    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fbr95    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fbs56    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fc97k    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fcc8c    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fcplr    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fcrqz    0/1     OutOfmemory              0          2m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fcs9c    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fd5jw    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fdf6v    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fdmv2    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ff6w9    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ffd8s    0/1     OutOfmemory              0          74s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fflgj    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-fg5kt    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fg775    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fgp9c    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fgpc9    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fh2st    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fhfxw    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fhhvf    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fhmg6    0/1     OutOfmemory              0          4m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fhq76    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fhskp    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fhwbt    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fj25x    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fj2t9    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fj9wl    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fjbvq    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fjh9q    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fjtqd    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fkn2c    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fl9dl    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fld6q    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-flfnz    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-flh45    0/1     OutOfmemory              0          93s     <none>       node2   <none>           <none>
guranteed-69875c65d6-flhgz    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-flnpp    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fmbv9    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fmhtc    0/1     OutOfmemory              0          52s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fmvfq    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fn4v7    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fnlxd    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fnrfm    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fpcpw    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fphx7    0/1     OutOfmemory              0          2m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fq2w9    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fq6nx    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fq7dw    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fqfx7    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fqgwv    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fqnjs    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fqscx    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fqtgl    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-frljt    0/1     OutOfmemory              0          3m      <none>       node2   <none>           <none>
guranteed-69875c65d6-fs4sw    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fs8q7    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fscsw    0/1     OutOfmemory              0          36s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fsf9k    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fsllv    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fsq28    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fsssr    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fsw4p    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ft5rf    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ft89g    0/1     OutOfmemory              0          4m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ftv54    0/1     OutOfmemory              0          3m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ftwsp    0/1     OutOfmemory              0          45s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ftxk7    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ftzjz    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ftzxv    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fv92l    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fv96l    0/1     OutOfmemory              0          3m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fvdng    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fvj42    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fvsds    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fvstl    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fvv9g    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fw625    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fw86k    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fwnrq    0/1     OutOfmemory              0          86s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fxs2n    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fzc6l    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-fzckt    0/1     OutOfmemory              0          88s     <none>       node2   <none>           <none>
guranteed-69875c65d6-fzn8m    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-fzxfn    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g29bc    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-g2gm4    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-g2pz7    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g2zsl    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g4dmn    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g4gpr    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g4l5l    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g5c2t    0/1     OutOfmemory              0          3m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g5csj    0/1     OutOfmemory              0          66s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g5l97    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g5x4h    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g69qp    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g6bvg    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g6dcb    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g6m56    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-g6svt    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-g6xjb    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g6xk4    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g78wz    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g7flw    0/1     OutOfmemory              0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-g7qhp    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g7s46    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g8hkf    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g8s4j    0/1     OutOfmemory              0          2m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-g8tfr    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g8tfw    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-g9grl    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g9mjq    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-g9rzr    0/1     OutOfmemory              0          74s     <none>       node2   <none>           <none>
guranteed-69875c65d6-g9vl4    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gbfzm    0/1     OutOfmemory              0          4m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gbm9r    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gbnpq    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gbt5h    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gcrd4    0/1     OutOfmemory              0          27s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gcw9z    0/1     OutOfmemory              0          2m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gdwpz    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-gdzgc    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gffc8    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gfh9c    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gfhhq    0/1     OutOfmemory              0          30s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gfsrw    0/1     OutOfmemory              0          3m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gggfz    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ghbqv    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ghj9r    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ghkxf    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gj27v    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gj7f4    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gj8qd    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gjldm    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gkhqr    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gkkcm    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gkrmj    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gl9h4    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-glc24    0/1     OutOfmemory              0          3m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gldnw    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gm5k8    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gmtgg    0/1     OutOfmemory              0          93s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gmxjn    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gn4fm    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gn6xs    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gnnsb    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gnttq    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gpclr    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gqm94    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gr6cw    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gr6jd    0/1     OutOfmemory              0          3m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gr8gw    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-grc7d    0/1     OutOfmemory              0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-grc7m    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gs47z    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gs79c    0/1     OutOfmemory              0          98s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gscsm    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gsfhg    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gszxg    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gthlp    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gtmw9    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gtvzf    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gvg66    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gw22q    0/1     OutOfmemory              0          3m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gwlbj    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gwsfl    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gx9hz    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gxbsd    0/1     OutOfmemory              0          47s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gxq7c    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-gxrc2    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gz2h9    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-gz4rt    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-gz84j    0/1     OutOfmemory              0          45s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gzhpg    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-gzrzb    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h29xk    0/1     OutOfmemory              0          2m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h2b6l    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-h2jtg    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h2w4b    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h44mb    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-h45cl    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h4qr2    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-h5bkr    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-h5fx9    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-h5grf    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h5s5b    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-h5xhb    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h6jlc    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-h6z5d    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-h82wj    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-h85zx    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h86sd    0/1     OutOfmemory              0          78s     <none>       node2   <none>           <none>
guranteed-69875c65d6-h8ndm    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h8pvz    0/1     OutOfmemory              0          3m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h9gzb    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-h9hcp    0/1     OutOfmemory              0          2m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hc4ss    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hc5rl    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hcb6q    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hcxrv    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hd54z    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hd99v    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hdbkc    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hdqhk    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hdvr4    0/1     OutOfmemory              0          4m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hf9tk    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hfdzj    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hfg76    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hfjd7    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hflvd    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hfqrj    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hfs6j    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hfz4h    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hggrr    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hgmhs    0/1     OutOfmemory              0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-hhmq5    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hhsrb    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hhxl8    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hjtdr    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hkpp2    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hlg7m    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hlgtv    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hlht2    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hllvc    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hm4pv    0/1     OutOfmemory              0          3m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hmcgj    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hmjmw    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hmnhm    0/1     OutOfmemory              0          2m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hnz5q    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hpprv    0/1     OutOfmemory              0          3m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hprc5    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hpwsl    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hq5cl    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hqhq7    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hqjj5    0/1     OutOfmemory              0          71s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hrf9b    0/1     OutOfmemory              0          88s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hrfnz    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hrqsx    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hs79z    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hth2n    0/1     OutOfmemory              0          3m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-htq8g    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hvjn7    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hvld5    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-hw6h9    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hw6k8    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hz7gb    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-hzbvj    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-hzbwq    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j27kx    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-j29w9    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j47qb    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-j4xxn    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-j5k8b    0/1     OutOfmemory              0          3m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j5vft    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j5xtd    0/1     OutOfmemory              0          2m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j6k7s    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j6lbj    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j6p2w    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j6t6n    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-j7bns    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j7ns8    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-j7vfz    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-j87l9    0/1     OutOfmemory              0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-j8jgc    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-j9qz9    0/1     OutOfmemory              0          27s     <none>       node2   <none>           <none>
guranteed-69875c65d6-j9svw    0/1     OutOfmemory              0          4m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jb5dv    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jbbp7    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jbjw5    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jbph6    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jbrhl    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jc5rl    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jc8sd    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jcqhb    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jd97d    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jdfh5    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jdhmv    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jdj29    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jdq4j    0/1     OutOfmemory              0          3m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jfbg6    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jfmsd    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jfvn9    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jfvvg    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jgjmr    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jgng4    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jgtgn    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jgtqm    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jhnb4    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jhrtr    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jhwz7    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jhz9z    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jjdkl    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jjfcx    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jjjd7    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jjl57    0/1     OutOfmemory              0          49s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jk6qx    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jkjcr    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jksfv    0/1     OutOfmemory              0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jkx7m    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jl2xk    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jldq8    0/1     OutOfmemory              0          29s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jlr6g    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jlzrp    0/1     OutOfmemory              0          52s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jm62p    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jm6pm    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jmlv7    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jnd5c    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jnfbb    0/1     OutOfmemory              0          4m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jnjsg    0/1     OutOfmemory              0          99s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jnq6j    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jph7m    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jprhg    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jpwr7    0/1     OutOfmemory              0          30s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jq6mh    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jqjpm    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jqk2z    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jr5f7    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jrcg2    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jrh75    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jrp57    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jrwbj    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jsc8m    0/1     OutOfmemory              0          2m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jswq9    0/1     OutOfmemory              0          4m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jt6wg    0/1     OutOfmemory              0          3m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jtjln    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jtm8p    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jv28q    0/1     OutOfmemory              0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-jv7qv    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jvwvd    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jw4tr    0/1     OutOfmemory              0          85s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jw6fp    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jwgxx    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-jwm5p    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jwwv9    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jx5f9    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-jxgqh    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-jz7zp    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-k26cd    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k29hc    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k2dvq    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k2m7q    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-k4klv    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k5n6r    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k5zl2    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k66qq    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k69zg    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k6d88    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k6hgd    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k6ncl    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k6xmz    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k7kjk    0/1     OutOfmemory              0          88s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k7rln    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k7rxd    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k7s7x    0/1     OutOfmemory              0          4m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k7z9j    0/1     OutOfmemory              0          94s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k87pk    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-k8m54    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k8pgr    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k9f4d    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-k9n2d    0/1     OutOfmemory              0          2m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-k9v9g    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-k9vtj    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-k9vvd    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kbr6s    0/1     OutOfmemory              0          40s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kcjg7    0/1     OutOfmemory              0          4m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kcrtr    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kctsl    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kcwcl    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kdchm    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kdffk    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kdjbq    0/1     OutOfmemory              0          3m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kdkbb    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kdlld    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kfnpz    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kfqrg    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kg4mg    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kg5n8    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kg9ck    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kgc7z    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kggqw    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kgktt    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kh74b    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-khd9p    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-khnq8    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-khvm4    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kjn4j    0/1     OutOfmemory              0          99s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kjq87    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kjqsv    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kjs7v    0/1     OutOfmemory              0          71s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kjz2w    0/1     OutOfmemory              0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kkcgr    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kkhs2    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kkkx4    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kkmfz    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kkq9d    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kksnn    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kl75t    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kllhf    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kmrds    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kn5jf    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kndrm    0/1     OutOfmemory              0          36s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kngp8    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kp56z    0/1     OutOfmemory              0          3m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kp8m5    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kpbn6    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kpczk    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kpq7z    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kpxbl    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kpxwr    0/1     OutOfmemory              0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kq759    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kqcs6    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kqfsj    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kqjd9    0/1     OutOfmemory              0          3m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kqw8k    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kqwnc    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kqzt6    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-krfsh    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-krptr    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-krxd8    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ks8qv    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kscj8    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kskmr    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kt4hn    0/1     OutOfmemory              0          71s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kt6gg    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ktsd2    0/1     OutOfmemory              0          2m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kv8d2    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kvlxx    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kvwdn    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kvww2    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-kw59f    0/1     OutOfmemory              0          4m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kwls9    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kww6m    0/1     OutOfmemory              0          78s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kxjt8    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kxn98    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-kxtqr    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kz48j    0/1     OutOfmemory              0          3m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kzdp9    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kzfrq    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-kzs5m    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-kzvj8    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-l2crq    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l562k    0/1     OutOfmemory              0          2m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-l579z    0/1     OutOfmemory              0          47s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l58hh    0/1     OutOfmemory              0          36s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l5w6x    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-l64v8    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-l6f6p    0/1     OutOfmemory              0          4m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-l6gcv    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-l6j8q    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l6mdc    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l6ncj    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l6nwc    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-l6qbl    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l6z6w    0/1     OutOfmemory              0          88s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l74z8    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l7rtw    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l8grw    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-l9289    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-l974s    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-l984l    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-l989m    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-l9nbt    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-l9zc2    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lb4qv    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lb5jw    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lb67v    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lb74t    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lb7n9    0/1     OutOfmemory              0          52s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lbhj5    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lbndr    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lbs8k    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lbx8r    0/1     OutOfmemory              0          4m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lc64b    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lcjlc    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lcqm9    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lcxjc    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ldqf7    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ldwxc    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lf2dn    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lf2np    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lg25s    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-lg46s    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lg9p8    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lgqm5    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lh2p8    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lh7xc    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lhkpr    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lhkvg    0/1     OutOfmemory              0          4m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lhp25    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lhz2d    0/1     OutOfmemory              0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-lj4fn    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ljfdw    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ljfrh    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ljqb7    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lkgl8    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lkz5v    0/1     OutOfmemory              0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-ll8pz    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-llcpp    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-llmck    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
guranteed-69875c65d6-llr5v    0/1     OutOfmemory              0          2m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lm92g    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lmf2s    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lmp7b    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lmpnf    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lmqxm    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ln4sv    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lnc4d    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lnfnr    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lnmdx    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lpkck    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lpzxw    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lq67h    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lq6lh    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lq6pt    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lq74b    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lq9c5    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lqn7g    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lqvxm    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lr4cj    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lr9lr    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lrc2w    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lrl55    0/1     OutOfmemory              0          3m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lrs7k    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ls8nw    0/1     OutOfmemory              0          2m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lsbpp    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lstml    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lt62q    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lt6pg    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ltbf7    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ltlj6    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ltmqt    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lv2zh    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lv5w6    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lv9hl    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lvgw5    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lvvb6    0/1     OutOfmemory              0          2m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lvvnc    0/1     OutOfmemory              0          30s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lw4jm    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-lwlrk    0/1     OutOfmemory              0          3m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lwnzm    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lx2tt    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lx5j5    0/1     OutOfmemory              0          66s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lx8p6    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lxshk    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-lz9nm    0/1     OutOfmemory              0          2m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-lzglz    0/1     OutOfmemory              0          73s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m2rlb    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m45f6    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m48mg    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-m4qxc    0/1     OutOfmemory              0          118s    <none>       node2   <none>           <none>
guranteed-69875c65d6-m4w6b    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m584g    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-m5gzb    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m5pqn    0/1     OutOfmemory              0          45s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m5pxq    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-m5rvb    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m5wcg    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m6km6    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-m74xz    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m78kt    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-m78qt    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-m7thl    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m7wm8    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-m82hb    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-m84rg    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m8gx6    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-m9hrj    0/1     OutOfmemory              0          93s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m9xg8    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-m9xn6    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mbbrd    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mbc42    0/1     OutOfmemory              0          78s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mbcz7    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mbdt7    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mbfwl    0/1     OutOfmemory              0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-mbp6t    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mbrcb    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mbs5n    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mc9c9    0/1     OutOfmemory              0          3m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mcldn    0/1     OutOfmemory              0          2m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mcp2p    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mcprt    0/1     OutOfmemory              0          52s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mcs6f    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mcxzt    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-mdntz    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mdp6c    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mdpzh    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mf9sz    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mgpt5    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mgs2f    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mgspj    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mgxqw    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mh4lm    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mh5j4    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mh6vj    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mhbqf    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mhtzm    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mhzbr    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mk2ms    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mk58k    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mk97t    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mkblc    0/1     OutOfmemory              0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-mkxnt    0/1     OutOfmemory              0          91s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ml2s4    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ml7z9    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mlktg    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mll5x    0/1     OutOfmemory              0          3m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mmqtz    0/1     OutOfmemory              0          40s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mmwdx    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mn5ts    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mngrr    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-mnn8z    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mnr7s    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mnzf4    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mpbtt    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mpph6    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mq4d6    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mqc7d    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mqj6g    0/1     OutOfmemory              0          3m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mqkss    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mqr7h    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mqvqb    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mr4dw    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mrfcg    0/1     OutOfmemory              0          78s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mrln7    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mrrz8    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mrwcl    0/1     OutOfmemory              0          71s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mrxtz    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ms97v    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-msvsz    0/1     OutOfmemory              0          86s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mt2qx    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mtslp    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mv42h    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mvdks    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mvhlh    0/1     OutOfmemory              0          62s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mvrlm    0/1     OutOfmemory              0          118s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mvrqn    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mvt79    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mwgqd    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mwq4l    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mxjxr    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-mz2bk    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mz584    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-mzpbj    0/1     OutOfmemory              0          2m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-mzqv8    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n225w    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n2d2m    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n49g6    0/1     OutOfmemory              0          30s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n4v2f    0/1     OutOfmemory              0          2m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-n59q8    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n5d5q    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-n5mjz    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n6b6f    0/1     OutOfmemory              0          66s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n6k5w    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-n7ck8    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n7zjc    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n8c4v    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-n8ksd    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-n8wfg    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n96df    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-n9bs7    0/1     OutOfmemory              0          47s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n9h5c    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-n9nf9    0/1     OutOfmemory              0          2m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-n9svg    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-n9tsb    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-n9zk4    0/1     OutOfmemory              0          2m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nb6qk    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nb6t5    0/1     OutOfmemory              0          3m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nbvhh    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nc4f4    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nc8gt    0/1     OutOfmemory              0          3m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ncckk    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ncdtr    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ndbxx    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ndhq9    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ndlg2    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ndlpf    0/1     OutOfmemory              0          3m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nf49j    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nfjt4    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ng2k2    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ng5gv    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ngsh7    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ngt5j    0/1     OutOfmemory              0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-ngx8z    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nhnbm    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nj7gh    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-njl6h    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-njlwj    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-njp54    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-njxkp    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nkbfq    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nkc2b    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nkctr    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nkm5v    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nl2wd    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nlbrd    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nlfz7    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nlnjp    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nmtcv    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nngs9    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nnw82    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-npcwf    0/1     OutOfmemory              0          4m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nr2zj    0/1     OutOfmemory              0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-nr4vg    0/1     OutOfmemory              0          35s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nrdz4    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nrrqh    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ns877    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ns8js    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nsx7j    0/1     OutOfmemory              0          94s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nt448    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ntbbs    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ntcxg    0/1     OutOfmemory              0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-nv52d    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nv6pd    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nvdw2    0/1     OutOfmemory              0          2m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nvn9k    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nvwjp    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nwp2z    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-nwwpv    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nx6p8    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nxbvt    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nz4qf    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nz6fk    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nz75g    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nzbwk    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-nzd8p    0/1     OutOfmemory              0          2m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nzjxq    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nzk9r    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-nzns5    0/1     OutOfmemory              0          4m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-nzz7w    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-p2jsb    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-p2lgv    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-p467x    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-p4kc7    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-p4knz    0/1     OutOfmemory              0          71s     <none>       node2   <none>           <none>
guranteed-69875c65d6-p52qt    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-p5gtw    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-p64ln    0/1     OutOfmemory              0          73s     <none>       node2   <none>           <none>
guranteed-69875c65d6-p6lzz    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-p6zzj    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-p74bb    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-p77dm    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-p7v67    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-p7zcn    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-p8hz9    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-p8kgk    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-p97t2    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-p9wl8    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pbq89    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pbvp8    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-pc9m7    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pcb5r    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pctpj    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pdd59    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pf4fb    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pfkh9    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pftqh    0/1     OutOfmemory              0          78s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pfz66    0/1     OutOfmemory              0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-pg7x9    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-phgdf    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-phk4q    0/1     OutOfmemory              0          4m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-phlgg    0/1     OutOfmemory              0          67s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pjjt4    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pjspl    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pjtjr    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pl2ts    0/1     OutOfmemory              0          30s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pl72h    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pm6r4    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-pmkvj    0/1     OutOfmemory              0          74s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pml2g    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-pmzbx    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pnc2z    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-pncj2    0/1     OutOfmemory              0          4m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pngkc    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pnldj    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pnr67    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pp52n    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-pp5cg    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pp895    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pp97t    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pq5zx    0/1     OutOfmemory              0          4m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pqxg8    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pqz56    0/1     OutOfmemory              0          3m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-prndn    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ps5ps    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-psc7q    0/1     OutOfmemory              0          3m      <none>       node2   <none>           <none>
guranteed-69875c65d6-psl5d    0/1     OutOfmemory              0          52s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ptdsz    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pthz5    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ptqcb    0/1     OutOfmemory              0          4m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ptxpb    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pvp85    0/1     OutOfmemory              0          3m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-pvt8m    1/1     Running                  0          11m     10.0.0.188   node2   <none>           <none>
guranteed-69875c65d6-pwbf6    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-pwc2z    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-px2xr    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pxf5s    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pxhsl    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-pxs4r    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q26w2    0/1     OutOfmemory              0          94s     <none>       node2   <none>           <none>
guranteed-69875c65d6-q288j    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-q42vf    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-q449v    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q4dmp    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q4g9x    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q4jgn    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q6mdv    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-q6tx6    0/1     OutOfmemory              0          29s     <none>       node2   <none>           <none>
guranteed-69875c65d6-q6w4z    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-q78rg    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q7t9d    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-q7xt5    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q8bfw    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q8krr    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q989m    0/1     OutOfmemory              0          3m      <none>       node2   <none>           <none>
guranteed-69875c65d6-q9bcp    0/1     OutOfmemory              0          2m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q9bkv    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-q9xf8    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qbdq5    1/1     Running                  0          4m22s   10.0.0.96    node2   <none>           <none>
guranteed-69875c65d6-qbdqr    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qbjkh    0/1     OutOfmemory              0          3m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qc65s    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qd2kc    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qd6xd    0/1     OutOfmemory              0          30s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qf28s    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qfz26    0/1     OutOfmemory              0          3m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qh5rh    0/1     OutOfmemory              0          36s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qh6zz    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qh8l4    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qhsmt    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qhx69    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qj5d2    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qjdtn    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qjll5    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qk8ks    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qkhpc    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qkjrp    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qkmdh    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qkthc    0/1     OutOfmemory              0          99s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ql9jk    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ql9v7    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qm6mn    0/1     OutOfmemory              0          3m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qmskl    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qmttt    0/1     OutOfmemory              0          73s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qngkq    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qps8b    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qq6qk    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qq88j    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qqg9t    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qqpgs    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qqqzd    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qqrlm    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qqxhh    0/1     OutOfmemory              0          2m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qqxj6    0/1     OutOfmemory              0          3m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qr4gf    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qrhjg    0/1     OutOfmemory              0          3m      <none>       node2   <none>           <none>
guranteed-69875c65d6-qrxtw    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qrxxv    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qrzcm    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qs74z    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qt5nt    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qtg8w    0/1     OutOfmemory              0          3m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qtlwp    0/1     OutOfmemory              0          2m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qtwvf    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qvg5q    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qvkpj    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qvqw7    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qvr88    0/1     OutOfmemory              0          52s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qwl9w    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qwvnw    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qww6h    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qx6pp    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qx88m    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qx9fq    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qxck8    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qxm4f    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-qxvfq    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-qzgvl    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-qzj6p    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-r2h9m    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r2jsl    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r2sw9    0/1     OutOfmemory              0          73s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r2w95    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r4f6j    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r4r96    0/1     OutOfmemory              0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r4ttl    0/1     OutOfmemory              0          2m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r4zdg    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r5jh6    0/1     OutOfmemory              0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r5nwl    0/1     OutOfmemory              0          30s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r5pfs    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r5qnd    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-r6kh4    0/1     OutOfmemory              0          3m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r6lb2    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-r6mpk    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-r74hl    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r77f7    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r7dbb    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r7gf9    0/1     OutOfmemory              0          2m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r7qc5    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r7vtd    0/1     OutOfmemory              0          2m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r86z4    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r8gn4    0/1     OutOfmemory              0          62s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r8mcn    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r8wvr    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r928g    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r9f8f    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-r9w7x    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-r9wn4    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-rbr2z    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rc4pl    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rckvx    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rcncl    0/1     OutOfmemory              0          2m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rcp6v    0/1     OutOfmemory              0          2m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rcw9p    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rf52z    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rfb7d    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rflcj    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rfw94    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-rfwcq    0/1     OutOfmemory              0          15s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rggl4    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rgkn8    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rglhf    0/1     OutOfmemory              0          29s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rgpcg    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rgs4b    0/1     OutOfmemory              0          93s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rj4tj    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rjt88    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rjvrw    0/1     OutOfmemory              0          34s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rk98d    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rkwf8    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rlcs4    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rlrmw    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rlz4l    0/1     OutOfmemory              0          2m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rm75c    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rm9cc    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rm9gv    0/1     OutOfmemory              0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-rmfj7    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rmnl2    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rmrk2    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rmwkr    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rnbgc    0/1     OutOfmemory              0          2m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rnvgd    0/1     OutOfmemory              0          2m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rp2wf    0/1     OutOfmemory              0          72s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rpjs7    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-rqbbd    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rqftj    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rqkz7    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rr52z    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rr5hq    0/1     OutOfmemory              0          3m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rrcz8    0/1     OutOfmemory              0          3m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rrs6m    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rsq2w    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rt4q7    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rtkxg    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rvlt7    0/1     OutOfmemory              0          0s      <none>       node2   <none>           <none>
guranteed-69875c65d6-rvxvx    0/1     OutOfmemory              0          30s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rwnd5    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rwq94    0/1     OutOfmemory              0          57s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rxwm6    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-rxxxh    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rzkm2    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-rzn2r    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-rzq4p    0/1     OutOfmemory              0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-rzrlc    0/1     OutOfmemory              0          46s     <none>       node2   <none>           <none>
guranteed-69875c65d6-s22m7    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-s29gf    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-s2grp    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-s2kvr    0/1     OutOfmemory              0          3m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s2kwg    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s2qt9    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s2tgz    0/1     OutOfmemory              0          2m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s47j5    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s4d5m    0/1     OutOfmemory              0          88s     <none>       node2   <none>           <none>
guranteed-69875c65d6-s4q9h    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-s56rt    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-s59h9    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s68jj    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-s6vj2    0/1     OutOfmemory              0          93s     <none>       node2   <none>           <none>
guranteed-69875c65d6-s6z5m    0/1     OutOfmemory              0          2m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s724c    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-s8h9r    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-s8kfw    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s8wkc    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s92bv    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s9hjx    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-s9mpx    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sb9lm    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sbw4f    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sbxfn    0/1     OutOfmemory              0          105s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sc4df    0/1     OutOfmemory              0          3m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sct6x    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sczlb    0/1     OutOfmemory              0          2m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sdf8n    0/1     OutOfmemory              0          3m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sdx2t    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sfnd5    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sftmk    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sgj6d    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sgjh6    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sgn5j    0/1     OutOfmemory              0          3m34s   <none>       node2   <none>           <none>
guranteed-69875c65d6-shgk8    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sjjw2    0/1     OutOfmemory              0          78s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sjpmx    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sjrnx    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sjwqc    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sjx7x    0/1     OutOfmemory              0          4m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sjz6z    0/1     OutOfmemory              0          2m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-skkd5    0/1     OutOfmemory              0          116s    <none>       node2   <none>           <none>
guranteed-69875c65d6-slfg5    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-slj8r    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sljq8    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-slk28    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sllkt    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-slxsl    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sm4gw    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-smdqj    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-smnsg    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-smps6    0/1     OutOfmemory              0          4m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-smvxp    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-snt6h    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-snzj8    0/1     OutOfmemory              0          2m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sp4rn    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-spfm2    0/1     OutOfmemory              0          4m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-splkp    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-spqvs    0/1     OutOfmemory              0          4m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sptt6    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sqhpm    0/1     OutOfmemory              0          89s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sqqxf    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sqrzq    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sr4mk    0/1     OutOfmemory              0          2m29s   <none>       node2   <none>           <none>
guranteed-69875c65d6-srqdj    0/1     OutOfmemory              0          3m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-srxpv    0/1     OutOfmemory              0          111s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ss4ms    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ss4tn    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ss7md    0/1     OutOfmemory              0          38s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ss86b    0/1     OutOfmemory              0          52s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ssbnz    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sslks    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sslrn    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sspnt    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-stjcg    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-stm6x    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-stp6x    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sttfc    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-svk8g    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-svkd6    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-svzn5    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-sw2f5    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-swfb2    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-swl4q    0/1     OutOfmemory              0          2m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-swtqb    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sxhfx    0/1     OutOfmemory              0          51s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sxn6x    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-sxw4r    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-sz5vr    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-szxk9    0/1     OutOfmemory              0          14s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t26l2    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t28hx    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t2tvv    0/1     OutOfmemory              0          45s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t2zgv    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t495w    0/1     OutOfmemory              0          4m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t4jkw    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t4lzt    0/1     OutOfmemory              0          25s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t4mxt    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t4xzb    0/1     OutOfmemory              0          62s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t69tz    0/1     OutOfmemory              0          2m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t6g5l    0/1     OutOfmemory              0          36s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t6p8b    0/1     OutOfmemory              0          3m38s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t7d5f    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t7hmf    0/1     OutOfmemory              0          3m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-t7m66    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t88d6    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t8ccx    1/1     Running                  0          4m22s   10.0.0.143   node2   <none>           <none>
guranteed-69875c65d6-t8dvd    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t8hjq    0/1     OutOfmemory              0          24s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t8mrn    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t8x2d    0/1     OutOfmemory              0          15s     <none>       node2   <none>           <none>
guranteed-69875c65d6-t8z6n    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t8zdc    0/1     OutOfmemory              0          3m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-t9wnk    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tb72z    0/1     OutOfmemory              0          115s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tbbr4    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tc9v6    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tcmjw    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tcqtn    0/1     OutOfmemory              0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-td7pn    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tdbsg    0/1     OutOfmemory              0          85s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tdpm6    0/1     OutOfmemory              0          32s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tdxvf    0/1     OutOfmemory              0          2m33s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tgnbn    0/1     OutOfmemory              0          36s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tgp67    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tgqkx    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-th5f4    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tht5n    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-thtvp    0/1     OutOfmemory              0          40s     <none>       node2   <none>           <none>
guranteed-69875c65d6-thwjf    0/1     OutOfmemory              0          25s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tjb5b    0/1     OutOfmemory              0          3m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tjnvl    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tjr2g    0/1     OutOfmemory              0          15s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tjtff    0/1     OutOfmemory              0          2m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tjwgz    0/1     OutOfmemory              0          2m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tkbdk    0/1     OutOfmemory              0          2m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tkct9    0/1     OutOfmemory              0          2m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tklk2    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tkngl    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tkrn6    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tl5g6    0/1     OutOfmemory              0          39s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tlxf2    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tm5lr    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tmfg9    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tmnbr    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tmptk    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tn285    0/1     OutOfmemory              0          2m24s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tn6xp    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tnh6d    0/1     OutOfmemory              0          118s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tnhm5    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tnprx    0/1     OutOfmemory              0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-tp9h8    0/1     OutOfmemory              0          40s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tpcs6    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tppkg    0/1     OutOfmemory              0          4m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-tqpgw    0/1     OutOfmemory              0          4m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tqsng    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-tqv2q    0/1     OutOfmemory              0          45s     <none>       node2   <none>           <none>
guranteed-69875c65d6-trm24    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-trrcg    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ts6jt    0/1     OutOfmemory              0          2m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tsj5b    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tsrkd    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ttcqc    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tv5xp    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-tvkxr    0/1     OutOfmemory              0          3m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-twh8b    0/1     OutOfmemory              0          15s     <none>       node2   <none>           <none>
guranteed-69875c65d6-twjmd    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-twrxq    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-tx8wf    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-txb9k    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-txvs5    0/1     OutOfmemory              0          12s     <none>       node2   <none>           <none>
guranteed-69875c65d6-v2bpt    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-v2x7j    0/1     OutOfmemory              0          2m54s   <none>       node2   <none>           <none>
guranteed-69875c65d6-v4289    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-v4ghw    0/1     OutOfmemory              0          3m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-v4gzz    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-v4zr2    0/1     OutOfmemory              0          3m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-v56pz    0/1     OutOfmemory              0          15s     <none>       node2   <none>           <none>
guranteed-69875c65d6-v5jm4    0/1     OutOfmemory              0          2m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-v6v6z    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-v6vpp    0/1     OutOfmemory              0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-v72df    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-v747c    0/1     OutOfmemory              0          86s     <none>       node2   <none>           <none>
guranteed-69875c65d6-v7f67    0/1     OutOfmemory              0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-v9sms    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-v9vw2    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vbjbs    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vbmmb    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vbs82    0/1     OutOfmemory              0          3m      <none>       node2   <none>           <none>
guranteed-69875c65d6-vc5gd    0/1     OutOfmemory              0          3m52s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vcwz9    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vd2pm    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vd5cz    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vdm4x    0/1     OutOfmemory              0          3m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vfvv6    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vgsws    0/1     OutOfmemory              0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vgwgx    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vkgcs    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vkgzg    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vkjz9    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vksfl    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vktlm    0/1     OutOfmemory              0          2m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vl4v9    0/1     OutOfmemory              0          1s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vl8hz    0/1     OutOfmemory              0          15s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vlw4s    0/1     OutOfmemory              0          2m14s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vm2r4    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vm72l    0/1     OutOfmemory              0          101s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vm7l4    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vnd6q    0/1     OutOfmemory              0          2m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vnwbt    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vp84t    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vq2jv    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vqms9    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vqn9g    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vqnb9    0/1     OutOfmemory              0          4m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vr2qh    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vrbjd    0/1     OutOfmemory              0          3m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vrz8x    0/1     OutOfmemory              0          3m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vss6l    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vt4b2    0/1     OutOfmemory              0          3m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vt5ks    0/1     OutOfmemory              0          4m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vt97l    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vtxf4    0/1     OutOfmemory              0          4m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vv778    0/1     OutOfmemory              0          3m27s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vwcrx    0/1     OutOfmemory              0          4m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vwhcx    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vwj8m    0/1     OutOfmemory              0          118s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vwkqh    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vws5t    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vww8r    0/1     OutOfmemory              0          3m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vx8kd    0/1     OutOfmemory              0          4m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vxcmg    0/1     OutOfmemory              0          109s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vxpqr    0/1     OutOfmemory              0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-vxql2    0/1     OutOfmemory              0          112s    <none>       node2   <none>           <none>
guranteed-69875c65d6-vxtkx    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vz2c4    0/1     OutOfmemory              0          3m      <none>       node2   <none>           <none>
guranteed-69875c65d6-vzfdq    0/1     OutOfmemory              0          61s     <none>       node2   <none>           <none>
guranteed-69875c65d6-vzjg9    0/1     OutOfmemory              0          3m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-vzqrq    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-w2w5c    0/1     OutOfmemory              0          2m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w4gfv    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w4t74    0/1     OutOfmemory              0          3m21s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w5lbn    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w5nbm    0/1     OutOfmemory              0          25s     <none>       node2   <none>           <none>
guranteed-69875c65d6-w5q67    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w5rg7    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-w5x7b    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w62vq    0/1     OutOfmemory              0          66s     <none>       node2   <none>           <none>
guranteed-69875c65d6-w66k6    0/1     OutOfmemory              0          2m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-w76nr    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w7kh8    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-w7ths    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-w7zhx    0/1     OutOfmemory              0          15s     <none>       node2   <none>           <none>
guranteed-69875c65d6-w89rj    0/1     OutOfmemory              0          11s     <none>       node2   <none>           <none>
guranteed-69875c65d6-w8fhj    0/1     OutOfmemory              0          97s     <none>       node2   <none>           <none>
guranteed-69875c65d6-w8ggm    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-w8tfs    0/1     OutOfmemory              0          117s    <none>       node2   <none>           <none>
guranteed-69875c65d6-w92xc    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w952b    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-w96pj    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-w9hx7    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wb98l    0/1     OutOfmemory              0          4m20s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wbmp7    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wc9dk    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wcjvk    0/1     OutOfmemory              0          76s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wdgv4    0/1     OutOfmemory              0          18s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wf7j9    0/1     OutOfmemory              0          47s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wfg5v    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wfknp    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wglcz    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wglk6    0/1     OutOfmemory              0          2m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wgmhb    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wh2xr    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wh7xl    0/1     OutOfmemory              0          4m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-whhhd    0/1     OutOfmemory              0          83s     <none>       node2   <none>           <none>
guranteed-69875c65d6-whk66    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-whzl9    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wj6nx    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-wjqrq    0/1     OutOfmemory              0          3m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wjwm2    0/1     OutOfmemory              0          4m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wk6vg    0/1     OutOfmemory              0          13s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wk9vk    0/1     OutOfmemory              0          2m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wkhgz    0/1     OutOfmemory              0          65s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wkqv7    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wkzq9    0/1     OutOfmemory              0          2m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wlh9p    0/1     OutOfmemory              0          42s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wlj7f    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wlst7    0/1     OutOfmemory              0          22s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wm7ln    0/1     OutOfmemory              0          4m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wmc9c    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wml4g    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wmn8q    0/1     OutOfmemory              0          94s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wn89r    0/1     OutOfmemory              0          82s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wnmh6    0/1     OutOfmemory              0          4s      <none>       node2   <none>           <none>
guranteed-69875c65d6-wnvg6    0/1     OutOfmemory              0          73s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wp57j    0/1     OutOfmemory              0          3m45s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wpbvn    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wpgzv    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wpjkd    0/1     OutOfmemory              0          119s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wpknc    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wppx4    0/1     OutOfmemory              0          95s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wq5fw    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wq5sz    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wq8rb    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wq9dr    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wqpwd    0/1     OutOfmemory              0          78s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wqz24    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wrbwm    0/1     OutOfmemory              0          20s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wrjjs    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ws457    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ws4lr    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ws4rj    0/1     OutOfmemory              0          2m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ws5hk    0/1     OutOfmemory              0          4m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ws65d    0/1     OutOfmemory              0          70s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ws6xr    0/1     OutOfmemory              0          68s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wscr8    0/1     OutOfmemory              0          3m15s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wslq8    0/1     OutOfmemory              0          16s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wsw4r    0/1     OutOfmemory              0          2m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wt2m6    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wtcth    0/1     OutOfmemory              0          2m6s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wtrbh    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wtwvn    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wv8xp    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wv94r    0/1     OutOfmemory              0          2m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wvc2g    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wvfmd    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wvj22    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-wvn7v    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wvwjr    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wwdz7    0/1     OutOfmemory              0          3m51s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wwj82    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-wwjjh    0/1     OutOfmemory              0          37s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wwxct    0/1     OutOfmemory              0          74s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wxwdp    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-wxzkd    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-wz8vm    0/1     OutOfmemory              0          19s     <none>       node2   <none>           <none>
guranteed-69875c65d6-x255r    0/1     OutOfmemory              0          3m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x25vd    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-x2g2w    0/1     OutOfmemory              0          2m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x48rh    0/1     OutOfmemory              0          48s     <none>       node2   <none>           <none>
guranteed-69875c65d6-x5blr    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x5hr6    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-x5k7d    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-x5mkw    0/1     OutOfmemory              0          2m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x5mpw    0/1     OutOfmemory              0          84s     <none>       node2   <none>           <none>
guranteed-69875c65d6-x5mqf    0/1     OutOfmemory              0          2m56s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x62td    0/1     OutOfmemory              0          2m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x65jh    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-x65k9    0/1     OutOfmemory              0          9s      <none>       node2   <none>           <none>
guranteed-69875c65d6-x65x5    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x68gk    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-x6xm6    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x7cnd    0/1     OutOfmemory              0          5s      <none>       node2   <none>           <none>
guranteed-69875c65d6-x7dvg    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x7qn9    0/1     OutOfmemory              0          2m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-x8znr    0/1     OutOfmemory              0          2m10s   <none>       node2   <none>           <none>
guranteed-69875c65d6-x94nk    0/1     OutOfmemory              0          77s     <none>       node2   <none>           <none>
guranteed-69875c65d6-x9nqf    0/1     OutOfmemory              0          17s     <none>       node2   <none>           <none>
guranteed-69875c65d6-x9zbn    0/1     OutOfmemory              0          3m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xb94z    0/1     OutOfmemory              0          2m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xbb9t    0/1     OutOfmemory              0          3m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xbfzc    0/1     OutOfmemory              0          3m59s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xc4d8    0/1     OutOfmemory              0          21s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xc4mf    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xcdqh    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xdbgs    0/1     OutOfmemory              0          3m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xdk6c    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xdz8z    0/1     OutOfmemory              0          2m23s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xfpw7    0/1     OutOfmemory              0          6s      <none>       node2   <none>           <none>
guranteed-69875c65d6-xfsqz    0/1     OutOfmemory              0          73s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xfw8g    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xg925    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xgqjh    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xgrzx    0/1     OutOfmemory              0          50s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xhk6t    0/1     OutOfmemory              0          2m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xhvfn    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xj42n    0/1     OutOfmemory              0          100s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xj6bf    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xjh9c    0/1     OutOfmemory              0          2m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xjj7j    0/1     OutOfmemory              0          66s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xk44x    0/1     OutOfmemory              0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-xkcjl    0/1     OutOfmemory              0          79s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xkdkq    0/1     OutOfmemory              0          62s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xkgsg    0/1     OutOfmemory              0          90s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xkjv9    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xl78t    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-xl7cs    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xl86m    0/1     OutOfmemory              0          3m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xlb44    0/1     OutOfmemory              0          4m1s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xlsnj    0/1     OutOfmemory              0          44s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xlzl8    0/1     OutOfmemory              0          27s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xm2s6    0/1     OutOfmemory              0          2m25s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xmplt    0/1     OutOfmemory              0          3m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xn7n9    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-xndkd    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xnrsf    0/1     OutOfmemory              0          113s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xpk8c    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xqqvz    0/1     OutOfmemory              0          33s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xqrm5    0/1     OutOfmemory              0          2m4s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xr8r6    0/1     OutOfmemory              0          108s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xrcpt    0/1     OutOfmemory              0          2m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xrnpt    0/1     OutOfmemory              0          2m      <none>       node2   <none>           <none>
guranteed-69875c65d6-xrppf    0/1     OutOfmemory              0          3m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xrq8l    0/1     OutOfmemory              0          2m43s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xrqgn    0/1     OutOfmemory              0          69s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xrsrx    0/1     OutOfmemory              0          88s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xs6zx    0/1     OutOfmemory              0          3m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xsrf7    0/1     OutOfmemory              0          2m9s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xsw6v    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xvcv4    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xvd97    0/1     OutOfmemory              0          4m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xvhxf    0/1     OutOfmemory              0          4m16s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xvwq4    0/1     OutOfmemory              0          87s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xvzgp    0/1     OutOfmemory              0          99s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xw8kl    0/1     OutOfmemory              0          60s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xw9tv    0/1     OutOfmemory              0          29s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xwfs2    0/1     OutOfmemory              0          2m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xwt4s    0/1     OutOfmemory              0          3m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xxcsr    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-xxdpw    0/1     OutOfmemory              0          88s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xxr5s    0/1     OutOfmemory              0          98s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xz2db    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-xz5zs    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-xzxmw    0/1     OutOfmemory              0          34s     <none>       node2   <none>           <none>
guranteed-69875c65d6-z2f7t    0/1     OutOfmemory              0          2m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z2lvk    0/1     OutOfmemory              0          4m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-z2msk    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
guranteed-69875c65d6-z4l7l    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z4r5w    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z4tpd    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z5nst    0/1     OutOfmemory              0          55s     <none>       node2   <none>           <none>
guranteed-69875c65d6-z5sfs    0/1     OutOfmemory              0          4m8s    <none>       node2   <none>           <none>
guranteed-69875c65d6-z5x78    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z6g7l    0/1     OutOfmemory              0          2m57s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z6gfn    0/1     OutOfmemory              0          92s     <none>       node2   <none>           <none>
guranteed-69875c65d6-z6jng    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-z78bt    0/1     OutOfmemory              0          3m5s    <none>       node2   <none>           <none>
guranteed-69875c65d6-z7cth    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-z7s2t    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z7xc4    0/1     OutOfmemory              0          110s    <none>       node2   <none>           <none>
guranteed-69875c65d6-z88g6    0/1     OutOfmemory              0          41s     <none>       node2   <none>           <none>
guranteed-69875c65d6-z8hj8    0/1     OutOfmemory              0          2s      <none>       node2   <none>           <none>
guranteed-69875c65d6-z8kg6    0/1     OutOfmemory              0          4m      <none>       node2   <none>           <none>
guranteed-69875c65d6-z8p9k    0/1     OutOfmemory              0          96s     <none>       node2   <none>           <none>
guranteed-69875c65d6-z8vtb    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z9652    0/1     OutOfmemory              0          3m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z9bd4    0/1     OutOfmemory              0          3m49s   <none>       node2   <none>           <none>
guranteed-69875c65d6-z9hjh    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-z9qpr    0/1     OutOfmemory              0          2m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zb977    0/1     OutOfmemory              0          4m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zb9mm    0/1     OutOfmemory              0          81s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zbkfj    0/1     OutOfmemory              0          4m2s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zc46l    0/1     OutOfmemory              0          4m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zc98k    0/1     OutOfmemory              0          3m50s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zcgft    0/1     OutOfmemory              0          2m42s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zcqpz    0/1     OutOfmemory              0          104s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zcsj5    0/1     OutOfmemory              0          2m26s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zcwrv    0/1     OutOfmemory              0          2m22s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zd99w    0/1     OutOfmemory              0          103s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zdwrs    0/1     OutOfmemory              0          59s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zfwhs    0/1     OutOfmemory              0          2m31s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zg6cb    0/1     OutOfmemory              0          3m18s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zg8q8    0/1     OutOfmemory              0          3m7s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zg9xx    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zggdl    0/1     OutOfmemory              0          2m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zghrb    0/1     OutOfmemory              0          7s      <none>       node2   <none>           <none>
guranteed-69875c65d6-zgjm4    0/1     OutOfmemory              0          43s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zglqk    0/1     OutOfmemory              0          2m11s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zgxrx    0/1     OutOfmemory              0          3m44s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zhhkl    0/1     OutOfmemory              0          31s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zhpf6    0/1     OutOfmemory              0          3m35s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zhqbv    0/1     OutOfmemory              0          3m12s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zhz4j    0/1     OutOfmemory              0          40s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zjvnm    0/1     OutOfmemory              0          56s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zjvpl    0/1     OutOfmemory              0          3m39s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zk9wp    0/1     OutOfmemory              0          3m37s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zkctx    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zklf9    0/1     OutOfmemory              0          114s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zknmt    0/1     OutOfmemory              0          47s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zkq6w    0/1     OutOfmemory              0          10s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zkw2x    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zl5j2    0/1     OutOfmemory              0          3m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zl7c9    0/1     OutOfmemory              0          34s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zlcrw    0/1     OutOfmemory              0          4m17s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zlf95    0/1     OutOfmemory              0          54s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zljdv    0/1     OutOfmemory              0          3m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zlp64    0/1     OutOfmemory              0          23s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zm9rh    0/1     OutOfmemory              0          58s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zmtjv    0/1     OutOfmemory              0          8s      <none>       node2   <none>           <none>
guranteed-69875c65d6-znlvb    0/1     OutOfmemory              0          2m28s   <none>       node2   <none>           <none>
guranteed-69875c65d6-znml7    0/1     OutOfmemory              0          107s    <none>       node2   <none>           <none>
guranteed-69875c65d6-znsht    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-znsjh    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zp8d2    0/1     OutOfmemory              0          27s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zph27    0/1     OutOfmemory              0          106s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zqfhj    0/1     OutOfmemory              0          2m19s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zqhdj    0/1     OutOfmemory              0          2m32s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zqm6v    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zqmrg    0/1     OutOfmemory              0          75s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zqq8h    0/1     OutOfmemory              0          3m40s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zr2pp    0/1     OutOfmemory              0          63s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zr7kf    0/1     OutOfmemory              0          3s      <none>       node2   <none>           <none>
guranteed-69875c65d6-zrf57    0/1     OutOfmemory              0          26s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zsg29    0/1     OutOfmemory              0          102s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zstll    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zsxjz    0/1     OutOfmemory              0          2m53s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zsz8v    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zt5dx    0/1     OutOfmemory              0          3m55s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ztfg8    0/1     OutOfmemory              0          53s     <none>       node2   <none>           <none>
guranteed-69875c65d6-ztgpk    0/1     OutOfmemory              0          3m58s   <none>       node2   <none>           <none>
guranteed-69875c65d6-ztr6q    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-ztzpv    0/1     OutOfmemory              0          93s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zv2vn    0/1     OutOfmemory              0          3m47s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zvcmd    0/1     OutOfmemory              0          2m46s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zw5q7    0/1     OutOfmemory              0          80s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zw6nt    0/1     OutOfmemory              0          3m48s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zw8bw    0/1     OutOfmemory              0          2m36s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zwhkl    0/1     OutOfmemory              0          3m13s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zwk8z    0/1     OutOfmemory              0          66s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zwrff    0/1     OutOfmemory              0          64s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zwvk7    0/1     OutOfmemory              0          40s     <none>       node2   <none>           <none>
guranteed-69875c65d6-zxbrr    0/1     OutOfmemory              0          3m41s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zz48p    0/1     OutOfmemory              0          3m3s    <none>       node2   <none>           <none>
guranteed-69875c65d6-zzlqg    0/1     OutOfmemory              0          3m30s   <none>       node2   <none>           <none>
guranteed-69875c65d6-zzw59    0/1     OutOfmemory              0          28s     <none>       node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   335m         16%    1140Mi          29%       
node1     36m          0%     1779Mi          22%       
node2     905m         22%    6613Mi          113%      
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-b4gnx   0m           1002Mi          
boostable-7f657bc6d7-8l2g6    0m           1002Mi          
guranteed-69875c65d6-87rzl    0m           1002Mi          
guranteed-69875c65d6-8xslq    0m           1002Mi          
guranteed-69875c65d6-pvt8m    0m           1002Mi          
guranteed-69875c65d6-qbdq5    0m           1002Mi          
guranteed-69875c65d6-t8ccx    0m           1003Mi          
tkr@hmc:~$ 




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
  RenewTime:       Mon, 28 Oct 2024 10:23:49 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Mon, 28 Oct 2024 10:23:22 +0900   Mon, 28 Oct 2024 10:23:22 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Mon, 28 Oct 2024 10:23:22 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Mon, 28 Oct 2024 10:23:22 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Mon, 28 Oct 2024 10:23:22 +0900   Mon, 28 Oct 2024 08:12:42 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
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
Non-terminated Pods:          (12 in total)
  Namespace                   Name                          CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                          ------------  ----------  ---------------  -------------  ---
  default                     boostable-7f657bc6d7-8l2g6    100m (2%)     0 (0%)      100Mi (1%)       0 (0%)         14m
  default                     guranteed-69875c65d6-87rzl    100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   7m
  default                     guranteed-69875c65d6-8xslq    100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   7m
  default                     guranteed-69875c65d6-mbklp    100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   1s
  default                     guranteed-69875c65d6-pvt8m    100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   14m
  default                     guranteed-69875c65d6-qbdq5    100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   7m
  default                     guranteed-69875c65d6-t8ccx    100m (2%)     100m (2%)   1005Mi (17%)     1005Mi (17%)   7m
  kube-system                 cilium-envoy-5w4zk            0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d17h
  kube-system                 cilium-hg4mx                  100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         2d17h
  kube-system                 coredns-76f75df574-fg7t9      100m (2%)     0 (0%)      70Mi (1%)        170Mi (2%)     2d17h
  kube-system                 coredns-76f75df574-j7w94      100m (2%)     0 (0%)      70Mi (1%)        170Mi (2%)     2d17h
  kube-system                 kube-proxy-457qh              0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d17h
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests       Limits
  --------           --------       ------
  cpu                1 (25%)        600m (15%)
  memory             6280Mi (108%)  6370Mi (109%)
  ephemeral-storage  0 (0%)         0 (0%)
  hugepages-1Gi      0 (0%)         0 (0%)
  hugepages-2Mi      0 (0%)         0 (0%)
Events:
  Type     Reason                     Age                From     Message
  ----     ------                     ----               ----     -------
  Normal   Starting                   19m                kubelet  Starting kubelet.
  Warning  InvalidDiskCapacity        19m                kubelet  invalid capacity 0 on image filesystem
  Normal   NodeAllocatableEnforced    19m                kubelet  Updated Node Allocatable limit across pods
  Normal   NodeHasNoDiskPressure      19m                kubelet  Node node2 status is now: NodeHasNoDiskPressure
  Normal   NodeHasSufficientPID       19m                kubelet  Node node2 status is now: NodeHasSufficientPID
  Normal   NodeHasInsufficientMemory  6m2s               kubelet  Node node2 status is now: NodeHasInsufficientMemory
  Warning  EvictionThresholdMet       5m39s              kubelet  Attempting to reclaim memory
  Normal   NodeHasSufficientMemory    33s (x2 over 19m)  kubelet  Node node2 status is now: NodeHasSufficientMemory




root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Mon 2024-10-28 01:31:05 UTC; 3s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 9513 (kubelet)
      Tasks: 13 (limit: 9445)
     Memory: 53.3M
     CGroup: /system.slice/kubelet.service
             └─9513 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kubelet/config.yaml --container-runtime-endpoint=unix:///var/run/con>



tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-rjdjv   1/1     Running   0          4m44s   10.0.0.175   node2   <none>           <none>
boostable-7f657bc6d7-4m65m    1/1     Running   0          4m44s   10.0.0.201   node2   <none>           <none>
guranteed-69875c65d6-cxk2v    1/1     Running   0          4m43s   10.0.0.247   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   78m          3%     1059Mi          27%       
node1     34m          0%     765Mi           9%        
node2     30m          0%     3510Mi          60%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-rjdjv   0m           1002Mi          
boostable-7f657bc6d7-4m65m    0m           1002Mi          
guranteed-69875c65d6-cxk2v    0m           1002Mi     


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-guranteed.yaml 
deployment.apps/guranteed configured
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-rjdjv   1/1     Running   0          5m42s   10.0.0.175   node2   <none>           <none>
boostable-7f657bc6d7-4m65m    1/1     Running   0          5m42s   10.0.0.201   node2   <none>           <none>
guranteed-55c8bc8b4b-nt9qv    1/1     Running   0          4s      10.0.0.110   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   79m          3%     1050Mi          27%       
node1     34m          0%     764Mi           9%        
node2     29m          0%     3510Mi          60%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-rjdjv   0m           1002Mi          
boostable-7f657bc6d7-4m65m    0m           1002Mi          
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-rjdjv   1/1     Running   0          5m47s   10.0.0.175   node2   <none>           <none>
boostable-7f657bc6d7-4m65m    1/1     Running   0          5m47s   10.0.0.201   node2   <none>           <none>
guranteed-55c8bc8b4b-nt9qv    1/1     Running   0          9s      10.0.0.110   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   91m          4%     1050Mi          27%       
node1     37m          0%     764Mi           9%        
node2     68m          1%     2627Mi          45%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-rjdjv   0m           1002Mi    
boostable-7f657bc6d7-4m65m    0m           1002Mi    


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


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .spec.taints"
master1
[
  {
    "effect": "NoSchedule",
    "key": "node-role.kubernetes.io/control-plane"
  }
]
node1
null
node2
null
tkr@hmc:~/memory-eater/k8s-yaml$ 

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":6}}'
deployment.apps/guranteed patched


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
Taints:             node.kubernetes.io/memory-pressure:NoSchedule
Unschedulable:      false
Lease:
  HolderIdentity:  node2
  AcquireTime:     <unset>
  RenewTime:       Mon, 28 Oct 2024 10:45:53 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                         Message
  ----                 ------  -----------------                 ------------------                ------                         -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                     Cilium is running on this node
  MemoryPressure       True    Mon, 28 Oct 2024 10:42:51 +0900   Mon, 28 Oct 2024 10:42:51 +0900   KubeletHasInsufficientMemory   kubelet has insufficient memory available
  DiskPressure         False   Mon, 28 Oct 2024 10:42:51 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasNoDiskPressure       kubelet has no disk pressure
  PIDPressure          False   Mon, 28 Oct 2024 10:42:51 +0900   Sun, 27 Oct 2024 18:12:09 +0900   KubeletHasSufficientPID        kubelet has sufficient PID available
  Ready                True    Mon, 28 Oct 2024 10:42:51 +0900   Mon, 28 Oct 2024 08:12:42 +0900   KubeletReady                   kubelet is posting ready status. AppArmor enabled
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
Non-terminated Pods:          (7 in total)
  Namespace                   Name                                         CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                                         ------------  ----------  ---------------  -------------  ---
  ingress-nginx               ingress-nginx-controller-849dcdc99d-2zp8p    100m (2%)     0 (0%)      90Mi (1%)        0 (0%)         3m45s
  kube-system                 cilium-envoy-5w4zk                           0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d17h
  kube-system                 cilium-hg4mx                                 100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         2d17h
  kube-system                 coredns-76f75df574-fg7t9                     100m (2%)     0 (0%)      70Mi (1%)        170Mi (2%)     2d17h
  kube-system                 coredns-76f75df574-j7w94                     100m (2%)     0 (0%)      70Mi (1%)        170Mi (2%)     2d17h
  kube-system                 kube-proxy-457qh                             0 (0%)        0 (0%)      0 (0%)           0 (0%)         2d17h
  kube-system                 metrics-server-7fb95cbc9b-86jr4              100m (2%)     0 (0%)      200Mi (3%)       0 (0%)         3m45s
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests    Limits
  --------           --------    ------
  cpu                500m (12%)  0 (0%)
  memory             440Mi (7%)  340Mi (5%)
  ephemeral-storage  0 (0%)      0 (0%)
  hugepages-1Gi      0 (0%)      0 (0%)
  hugepages-2Mi      0 (0%)      0 (0%)
Events:
  Type     Reason                     Age                    From     Message
  ----     ------                     ----                   ----     -------
  Normal   Starting                   41m                    kubelet  Starting kubelet.
  Warning  InvalidDiskCapacity        41m                    kubelet  invalid capacity 0 on image filesystem
  Normal   NodeAllocatableEnforced    41m                    kubelet  Updated Node Allocatable limit across pods
  Normal   NodeHasNoDiskPressure      41m                    kubelet  Node node2 status is now: NodeHasNoDiskPressure
  Normal   NodeHasSufficientPID       41m                    kubelet  Node node2 status is now: NodeHasSufficientPID
  Normal   NodeHasInsufficientMemory  28m                    kubelet  Node node2 status is now: NodeHasInsufficientMemory
  Warning  EvictionThresholdMet       27m                    kubelet  Attempting to reclaim memory
  Normal   NodeHasSufficientMemory    22m (x2 over 41m)      kubelet  Node node2 status is now: NodeHasSufficientMemory
  Normal   Starting                   14m                    kubelet  Starting kubelet.
  Warning  InvalidDiskCapacity        14m                    kubelet  invalid capacity 0 on image filesystem
  Normal   NodeAllocatableEnforced    14m                    kubelet  Updated Node Allocatable limit across pods
  Normal   NodeHasSufficientMemory    14m                    kubelet  Node node2 status is now: NodeHasSufficientMemory
  Normal   NodeHasNoDiskPressure      14m                    kubelet  Node node2 status is now: NodeHasNoDiskPressure
  Normal   NodeHasSufficientPID       14m                    kubelet  Node node2 status is now: NodeHasSufficientPID
  Normal   NodeHasInsufficientMemory  3m7s                   kubelet  Node node2 status is now: NodeHasInsufficientMemory
  Warning  EvictionThresholdMet       2m30s (x3 over 2m41s)  kubelet  Attempting to reclaim memory
tkr@hmc:~/memory-eater/k8s-yaml$ 

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .spec.taints"
master1
[
  {
    "effect": "NoSchedule",
    "key": "node-role.kubernetes.io/control-plane"
  }
]
node1
null
node2
[
  {
    "effect": "NoSchedule",
    "key": "node.kubernetes.io/memory-pressure",
    "timeAdded": "2024-10-28T01:42:51Z"
  }
]


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-2hndc   1/1     Running   0          3m44s   10.0.0.228   node2   <none>           <none>
boostable-7f657bc6d7-x56x6    1/1     Running   0          3m44s   10.0.0.89    node2   <none>           <none>
guranteed-69875c65d6-246f6    1/1     Running   0          3m43s   10.0.0.53    node2   <none>           <none>
guranteed-69875c65d6-lvs94    1/1     Running   0          95s     10.0.0.150   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   83m          4%     1138Mi          29%       
node1     30m          0%     611Mi           7%        
node2     37m          0%     4582Mi          78%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-2hndc   0m           1003Mi          
boostable-7f657bc6d7-x56x6    0m           1002Mi          
guranteed-69875c65d6-246f6    0m           1002Mi          
guranteed-69875c65d6-lvs94    0m           1002Mi      


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":2}}'
deployment.apps/guranteed patched
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-2hndc   1/1     Running   0          2m19s   10.0.0.228   node2   <none>           <none>
boostable-7f657bc6d7-x56x6    1/1     Running   0          2m19s   10.0.0.89    node2   <none>           <none>
guranteed-69875c65d6-246f6    1/1     Running   0          2m18s   10.0.0.53    node2   <none>           <none>
guranteed-69875c65d6-lvs94    1/1     Running   0          10s     10.0.0.150   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   79m          3%     1141Mi          29%       
node1     28m          0%     610Mi           7%        
node2     37m          0%     3576Mi          61%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-2hndc   0m           1003Mi          
boostable-7f657bc6d7-x56x6    0m           1002Mi          
guranteed-69875c65d6-246f6    0m           1002Mi   

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-2hndc   1/1     Running   0          5m22s   10.0.0.228   node2   <none>           <none>
boostable-7f657bc6d7-x56x6    1/1     Running   0          5m22s   10.0.0.89    node2   <none>           <none>
guranteed-69875c65d6-246f6    1/1     Running   0          5m21s   10.0.0.53    node2   <none>           <none>
guranteed-69875c65d6-7fl2f    1/1     Running   0          74s     10.0.0.83    node2   <none>           <none>
guranteed-69875c65d6-lvs94    1/1     Running   0          3m13s   10.0.0.150   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   76m          3%     1138Mi          29%       
node1     24m          0%     611Mi           7%        
node2     43m          1%     5591Mi          96%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-2hndc   0m           1003Mi          
boostable-7f657bc6d7-x56x6    0m           1002Mi          
guranteed-69875c65d6-246f6    0m           1002Mi          
guranteed-69875c65d6-7fl2f    0m           1002Mi          
guranteed-69875c65d6-lvs94    0m           1002Mi  

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-2hndc   1/1     Running   0          7m48s   10.0.0.228   node2   <none>           <none>
boostable-7f657bc6d7-x56x6    1/1     Running   0          7m48s   10.0.0.89    node2   <none>           <none>
guranteed-69875c65d6-246f6    1/1     Running   0          7m47s   10.0.0.53    node2   <none>           <none>
guranteed-69875c65d6-7fl2f    1/1     Running   0          3m40s   10.0.0.83    node2   <none>           <none>
guranteed-69875c65d6-l288t    1/1     Running   0          89s     10.0.0.147   node2   <none>           <none>
guranteed-69875c65d6-lvs94    1/1     Running   0          5m39s   10.0.0.150   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   81m          4%     1141Mi          29%       
node1     31m          0%     610Mi           7%        
node2     39m          0%     6596Mi          113%      
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-2hndc   0m           1003Mi          
boostable-7f657bc6d7-x56x6    0m           1002Mi          
guranteed-69875c65d6-246f6    0m           1002Mi          
guranteed-69875c65d6-7fl2f    0m           1002Mi          
guranteed-69875c65d6-l288t    0m           1002Mi          
guranteed-69875c65d6-lvs94    0m           1002Mi        

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.conditions[1].type, .status.conditions[1].status" | xargs -n3
master1 MemoryPressure False
node1 MemoryPressure False
node2 MemoryPressure False




tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS                   RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-2hndc   0/1     ContainerStatusUnknown   1          9m54s   10.0.0.228   node2   <none>           <none>
besteffort-657f6c859c-kzkc4   0/1     ContainerCreating        0          1s      <none>       node1   <none>           <none>
boostable-7f657bc6d7-x56x6    1/1     Running                  0          9m54s   10.0.0.89    node2   <none>           <none>
guranteed-69875c65d6-246f6    1/1     Running                  0          9m53s   10.0.0.53    node2   <none>           <none>
guranteed-69875c65d6-7fl2f    1/1     Running                  0          5m46s   10.0.0.83    node2   <none>           <none>
guranteed-69875c65d6-l288t    1/1     Running                  0          3m35s   10.0.0.147   node2   <none>           <none>
guranteed-69875c65d6-lvs94    1/1     Running                  0          7m45s   10.0.0.150   node2   <none>           <none>
guranteed-69875c65d6-zzx4x    1/1     Running                  0          49s     10.0.0.242   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   90m          4%     1139Mi          29%       
node1     26m          0%     610Mi           7%        
node2     55m          1%     7302Mi          125%      
NAME                         CPU(cores)   MEMORY(bytes)   
boostable-7f657bc6d7-x56x6   0m           1002Mi          
guranteed-69875c65d6-246f6   0m           1002Mi          
guranteed-69875c65d6-7fl2f   0m           1002Mi          
guranteed-69875c65d6-l288t   0m           1002Mi          
guranteed-69875c65d6-lvs94   0m           1002Mi          
guranteed-69875c65d6-zzx4x   11m          902Mi    


tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.conditions[1].type, .status.conditions[1].status" | xargs -n3
master1 MemoryPressure False
node1 MemoryPressure False
node2 MemoryPressure True

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.capacity.memory, .status.allocatable.memory" | xargs -n3
master1 4026044Ki 3923644Ki
node1 8150236Ki 8047836Ki
node2 8150228Ki 5950676Ki
