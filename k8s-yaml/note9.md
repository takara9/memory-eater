root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-29 02:37:14.139428814 +0000
@@ -43,3 +43,9 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+evictionSoft:
+  memory.available: 1Gi
+systemReserved:
+  memory: 1Gi
+evictionSoftGracePeriod:
+  memory.available: 30s
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Tue 2024-10-29 02:37:23 UTC; 2s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 4160 (kubelet)
      Tasks: 12 (limit: 9445)
     Memory: 312.5M
     CGroup: /system.slice/kubelet.service
             └─4160 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kube>

tkr@hmc:~/memory-eater$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.capacity.memory, .status.allocatable.memory" | xargs -n3
master1 4026052Ki 3923652Ki
node1 8150236Ki 8047836Ki
node2 8150236Ki 6999260Ki



root@node2:/var/lib/kubelet# vi config.yaml
root@node2:/var/lib/kubelet# diff -u config.yaml.org config.yaml
--- config.yaml.org	2024-10-27 07:09:47.682357933 +0000
+++ config.yaml	2024-10-29 02:39:50.558406919 +0000
@@ -43,3 +43,5 @@
 streamingConnectionIdleTimeout: 0s
 syncFrequency: 0s
 volumeStatsAggPeriod: 0s
+systemReserved:
+  memory: 1Gi
root@node2:/var/lib/kubelet# systemctl restart kubelet
root@node2:/var/lib/kubelet# systemctl status kubelet
● kubelet.service - kubelet: The Kubernetes Node Agent
     Loaded: loaded (/lib/systemd/system/kubelet.service; enabled; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/kubelet.service.d
             └─10-kubeadm.conf
     Active: active (running) since Tue 2024-10-29 02:39:57 UTC; 3s ago
       Docs: https://kubernetes.io/docs/
   Main PID: 4663 (kubelet)
      Tasks: 12 (limit: 9445)
     Memory: 59.5M
     CGroup: /system.slice/kubelet.service
             └─4663 /usr/bin/kubelet --bootstrap-kubeconfig=/etc/kubernetes/bootstrap-kubelet.conf --kubeconfig=/etc/kubernetes/kubelet.conf --config=/var/lib/kube>


tkr@hmc:~/memory-eater$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .status.capacity.memory, .status.allocatable.memory" | xargs -n3
master1 4026052Ki 3923652Ki
node1 8150236Ki 8047836Ki
node2 8150236Ki 6999260Ki

