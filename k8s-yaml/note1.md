
初期状態を確認
```
tkr@hmc:~/memory-eater$ kubectl get node
NAME      STATUS   ROLES           AGE   VERSION
master1   Ready    control-plane   44h   v1.29.9
node1     Ready    worker          44h   v1.29.9
node2     Ready    worker          44h   v1.29.9

tkr@hmc:~/memory-eater$ kubectl get pod -o wide && kubectl top node && kubectl top pod
No resources found in default namespace.
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   83m          4%     1025Mi          26%       
node1     44m          1%     757Mi           9%        
node2     30m          0%     495Mi           6%        
No resources found in default namespace.
```

テイントを設定して、node2をメモリ不足状態に追い込む準備
```
tkr@hmc:~/memory-eater$ kubectl taint nodes node1 workload:NoSchedule
node/node1 tainted

tkr@hmc:~/memory-eater$ kubectl get node -o json |jq -r ".items[]| .metadata.name, .spec.taints"
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
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl describe node node2
Name:               node2
Roles:              worker
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
  memory:             8047836Ki
  pods:               110
```


ポッドをデプロイして、メモリ負荷を与える
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-besteffort.yaml
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-boostable.yaml
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl apply -f deployment-guranteed.yaml 

tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o json |jq -r ".items[]| .metadata.name, .status.qosClass" | xargs -n2
besteffort-657f6c859c-bmzds BestEffort
boostable-7f657bc6d7-tplc4 Burstable
guranteed-69875c65d6-dc8q6 Guaranteed
```

```
kubectl get pod -o wide && kubectl top node && kubectl top pod
```


安定状態に達した
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS   AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-bmzds   1/1     Running   0          3m39s   10.0.0.20    node2   <none>           <none>
boostable-7f657bc6d7-tplc4    1/1     Running   0          3m39s   10.0.0.144   node2   <none>           <none>
guranteed-69875c65d6-dc8q6    1/1     Running   0          3m38s   10.0.0.107   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   79m          3%     1027Mi          26%       
node1     40m          1%     757Mi           9%        
node2     33m          0%     3520Mi          44%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-bmzds   0m           1002Mi          
boostable-7f657bc6d7-tplc4    0m           1002Mi          
guranteed-69875c65d6-dc8q6    0m           1002Mi  
```

ノード１のテイントを消す
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl taint nodes node1 workload:NoSchedule-
node/node1 untainted
```


メモリ負荷を増やし、メモリ不足状態にする

```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl patch deployment guranteed -p '{"spec":{"replicas":1}}'
deployment.apps/guranteed patched
```

besteffortとboostableのポッドは、再スタートしているが、ノード2に居座っている
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS        AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-bmzds   1/1     Running   1 (2m17s ago)   8m25s   10.0.0.20    node2   <none>           <none>
boostable-7f657bc6d7-tplc4    1/1     Running   1 (2m5s ago)    8m25s   10.0.0.144   node2   <none>           <none>
guranteed-69875c65d6-2n9t5    1/1     Running   0               30s     10.0.0.41    node2   <none>           <none>
guranteed-69875c65d6-7dx8c    1/1     Running   0               30s     10.0.0.125   node2   <none>           <none>
guranteed-69875c65d6-dc8q6    1/1     Running   0               8m24s   10.0.0.107   node2   <none>           <none>
guranteed-69875c65d6-fgbzs    1/1     Running   0               30s     10.0.0.134   node2   <none>           <none>
guranteed-69875c65d6-g58nl    1/1     Running   0               30s     10.0.0.187   node2   <none>           <none>
guranteed-69875c65d6-vbq89    1/1     Running   0               30s     10.0.0.130   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   89m          4%     1085Mi          28%       
node1     38m          0%     758Mi           9%        
node2     186m         4%     5394Mi          68%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-bmzds   0m           1003Mi          
boostable-7f657bc6d7-tplc4    0m           1002Mi          
guranteed-69875c65d6-2n9t5    14m          401Mi           
guranteed-69875c65d6-7dx8c    13m          401Mi           
guranteed-69875c65d6-dc8q6    0m           1002Mi          
guranteed-69875c65d6-fgbzs    13m          401Mi           
guranteed-69875c65d6-g58nl    13m          401Mi           
guranteed-69875c65d6-vbq89    12m          401Mi     
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS    RESTARTS      AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-bmzds   1/1     Running   3 (65s ago)   10m     10.0.0.20    node2   <none>           <none>
boostable-7f657bc6d7-tplc4    1/1     Running   3 (29s ago)   10m     10.0.0.144   node2   <none>           <none>
guranteed-69875c65d6-2n9t5    1/1     Running   0             2m48s   10.0.0.41    node2   <none>           <none>
guranteed-69875c65d6-7dx8c    1/1     Running   0             2m48s   10.0.0.125   node2   <none>           <none>
guranteed-69875c65d6-dc8q6    1/1     Running   0             10m     10.0.0.107   node2   <none>           <none>
guranteed-69875c65d6-fgbzs    1/1     Running   0             2m48s   10.0.0.134   node2   <none>           <none>
guranteed-69875c65d6-g58nl    1/1     Running   0             2m48s   10.0.0.187   node2   <none>           <none>
guranteed-69875c65d6-vbq89    1/1     Running   0             2m48s   10.0.0.130   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   88m          4%     1090Mi          28%       
node1     38m          0%     758Mi           9%        
node2     47m          1%     6883Mi          87%       
NAME                          CPU(cores)   MEMORY(bytes)   
besteffort-657f6c859c-bmzds   9m           602Mi           
guranteed-69875c65d6-2n9t5    0m           1002Mi          
guranteed-69875c65d6-7dx8c    0m           1002Mi          
guranteed-69875c65d6-dc8q6    0m           1002Mi          
guranteed-69875c65d6-fgbzs    0m           1002Mi          
guranteed-69875c65d6-g58nl    0m           1002Mi          
guranteed-69875c65d6-vbq89    0m           1002Mi     
```

ノード２の状態を確認する
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
  RenewTime:       Sun, 27 Oct 2024 16:00:18 +0900
Conditions:
  Type                 Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----                 ------  -----------------                 ------------------                ------                       -------
  NetworkUnavailable   False   Fri, 25 Oct 2024 17:09:34 +0900   Fri, 25 Oct 2024 17:09:34 +0900   CiliumIsUp                   Cilium is running on this node
  MemoryPressure       False   Sun, 27 Oct 2024 15:55:54 +0900   Sun, 27 Oct 2024 08:42:10 +0900   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure         False   Sun, 27 Oct 2024 15:55:54 +0900   Fri, 25 Oct 2024 18:12:53 +0900   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure          False   Sun, 27 Oct 2024 15:55:54 +0900   Fri, 25 Oct 2024 18:12:53 +0900   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready                True    Sun, 27 Oct 2024 15:55:54 +0900   Fri, 25 Oct 2024 18:12:53 +0900   KubeletReady                 kubelet is posting ready status. AppArmor enabled
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
  memory:             8047836Ki
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
  default                     besteffort-657f6c859c-bmzds    0 (0%)        0 (0%)      0 (0%)           0 (0%)         11m
  default                     boostable-7f657bc6d7-tplc4     100m (2%)     0 (0%)      100Mi (1%)       0 (0%)         11m
  default                     guranteed-69875c65d6-2n9t5     100m (2%)     100m (2%)   1005Mi (12%)     1005Mi (12%)   3m37s
  default                     guranteed-69875c65d6-7dx8c     100m (2%)     100m (2%)   1005Mi (12%)     1005Mi (12%)   3m37s
  default                     guranteed-69875c65d6-dc8q6     100m (2%)     100m (2%)   1005Mi (12%)     1005Mi (12%)   11m
  default                     guranteed-69875c65d6-fgbzs     100m (2%)     100m (2%)   1005Mi (12%)     1005Mi (12%)   3m37s
  default                     guranteed-69875c65d6-g58nl     100m (2%)     100m (2%)   1005Mi (12%)     1005Mi (12%)   3m37s
  default                     guranteed-69875c65d6-vbq89     100m (2%)     100m (2%)   1005Mi (12%)     1005Mi (12%)   3m37s
  kube-system                 cilium-envoy-5w4zk             0 (0%)        0 (0%)      0 (0%)           0 (0%)         46h
  kube-system                 cilium-hg4mx                   100m (2%)     0 (0%)      10Mi (0%)        0 (0%)         46h
  kube-system                 coredns-76f75df574-fg7t9       100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     46h
  kube-system                 coredns-76f75df574-j7w94       100m (2%)     0 (0%)      70Mi (0%)        170Mi (2%)     46h
  kube-system                 kube-proxy-457qh               0 (0%)        0 (0%)      0 (0%)           0 (0%)         46h
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests      Limits
  --------           --------      ------
  cpu                1 (25%)       600m (15%)
  memory             6280Mi (79%)  6370Mi (81%)
  ephemeral-storage  0 (0%)        0 (0%)
  hugepages-1Gi      0 (0%)        0 (0%)
  hugepages-2Mi      0 (0%)        0 (0%)
Events:
  Type     Reason     Age    From     Message
  ----     ------     ----   ----     -------
  Warning  SystemOOM  5m24s  kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 11415
  Warning  SystemOOM  5m12s  kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 11481
  Warning  SystemOOM  2m51s  kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 12147
  Warning  SystemOOM  2m24s  kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 12220
  Warning  SystemOOM  114s   kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13320
  Warning  SystemOOM  78s    kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13395
  Warning  SystemOOM  42s    kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13469
  ```

dmesg
```
[15088.559230] Mem-Info:
[15088.559231] active_anon:1935824 inactive_anon:334 isolated_anon:0
                active_file:34 inactive_file:50 isolated_file:0
                unevictable:4618 dirty:0 writeback:0 unstable:0
                slab_reclaimable:12827 slab_unreclaimable:29098
                mapped:2346 shmem:995 pagetables:5170 bounce:0
                free:26687 free_pcp:169 free_cma:0
[15088.559233] Node 0 active_anon:7743296kB inactive_anon:1336kB active_file:136kB inactive_file:200kB unevictable:18472kB isolated(anon):0kB isolated(file):0kB mapped:9384kB dirty:0kB writeback:0kB shmem:3980kB shmem_thp: 0kB shmem_pmdmapped: 0kB anon_thp: 0kB writeback_tmp:0kB unstable:0kB all_unreclaimable? no
[15088.559233] Node 0 DMA free:15908kB min:132kB low:164kB high:196kB active_anon:0kB inactive_anon:0kB active_file:0kB inactive_file:0kB unevictable:0kB writepending:0kB present:15992kB managed:15908kB mlocked:0kB kernel_stack:0kB pagetables:0kB bounce:0kB free_pcp:0kB local_pcp:0kB free_cma:0kB
[15088.559234] lowmem_reserve[]: 0 1886 7855 7855 7855
[15088.559235] Node 0 DMA32 free:39764kB min:16200kB low:20248kB high:24296kB active_anon:1969120kB inactive_anon:0kB active_file:0kB inactive_file:0kB unevictable:0kB writepending:0kB present:2080596kB managed:2015060kB mlocked:0kB kernel_stack:48kB pagetables:3872kB bounce:0kB free_pcp:0kB local_pcp:0kB free_cma:0kB
[15088.559236] lowmem_reserve[]: 0 0 5968 5968 5968
[15088.559237] Node 0 Normal free:51076kB min:51248kB low:64060kB high:76872kB active_anon:5774176kB inactive_anon:1336kB active_file:12kB inactive_file:12kB unevictable:18472kB writepending:0kB present:6295552kB managed:6119268kB mlocked:18472kB kernel_stack:9344kB pagetables:16808kB bounce:0kB free_pcp:676kB local_pcp:180kB free_cma:0kB
[15088.559238] lowmem_reserve[]: 0 0 0 0 0
[15088.559238] Node 0 DMA: 1*4kB (U) 0*8kB 0*16kB 1*32kB (U) 2*64kB (U) 1*128kB (U) 1*256kB (U) 0*512kB 1*1024kB (U) 1*2048kB (M) 3*4096kB (M) = 15908kB
[15088.559241] Node 0 DMA32: 23*4kB (UME) 16*8kB (UE) 25*16kB (UME) 39*32kB (UME) 42*64kB (UME) 15*128kB (UME) 2*256kB (U) 1*512kB (M) 2*1024kB (UM) 3*2048kB (UME) 6*4096kB (M) = 40268kB
[15088.559243] Node 0 Normal: 1470*4kB (UME) 317*8kB (UME) 106*16kB (UME) 98*32kB (UME) 83*64kB (UME) 31*128kB (UME) 5*256kB (E) 5*512kB (UME) 3*1024kB (ME) 3*2048kB (UME) 4*4096kB (M) = 51968kB
[15088.559246] Node 0 hugepages_total=0 hugepages_free=0 hugepages_surp=0 hugepages_size=1048576kB
[15088.559247] Node 0 hugepages_total=0 hugepages_free=0 hugepages_surp=0 hugepages_size=2048kB
[15088.559247] 3202 total pagecache pages
[15088.559248] 0 pages in swap cache
[15088.559248] Swap cache stats: add 0, delete 0, find 0/0
[15088.559248] Free swap  = 0kB
[15088.559249] Total swap = 0kB
[15088.559249] 2098035 pages RAM
[15088.559250] 0 pages HighMem/MovableOnly
[15088.559250] 60476 pages reserved
[15088.559250] 0 pages cma reserved
[15088.559250] 0 pages hwpoisoned
[15088.559250] Tasks state (memory values in pages):
[15088.559250] [  pid  ]   uid  tgid total_vm      rss pgtables_bytes swapents oom_score_adj name
[15088.559253] [    407]     0   407    26941     1060   225280        0          -250 systemd-journal
[15088.559254] [    447]     0   447     5853     1228    65536        0         -1000 systemd-udevd
[15088.559255] [    587]     0   587    70034     4487    90112        0         -1000 multipathd
[15088.559256] [    628]   114   628     1775      636    57344        0             0 rpcbind
[15088.559257] [    629]   102   629    22719      214    81920        0             0 systemd-timesyn
[15088.559258] [    680]   100   680     4797      979    77824        0             0 systemd-network
[15088.559258] [    701]     0   701    59851      286    94208        0             0 accounts-daemon
[15088.559260] [    703]   103   703     1896      677    53248        0          -900 dbus-daemon
[15088.559260] [    715]     0   715    20481      747    61440        0             0 irqbalance
[15088.559261] [    719]     0   719     7416     2839    94208        0             0 networkd-dispat
[15088.559262] [    720]     0   720    59110      564    98304        0             0 polkitd
[15088.559263] [    722]     0   722     1611      603    53248        0             0 qemu-ga
[15088.559263] [    725]     0   725   366948     3101   258048        0          -900 snapd
[15088.559264] [    732]     0   732     4325      433    69632        0             0 systemd-logind
[15088.559265] [    735]     0   735    98871     1346   126976        0             0 udisksd
[15088.559265] [    738]     0   738   910534     9240   593920        0          -999 containerd
[15088.559266] [    753]     0   753     3044      866    65536        0         -1000 sshd
[15088.559266] [    763]     0   763    26980     2680   110592        0             0 unattended-upgr
[15088.559267] [    808]     0   808    79705      911   118784        0             0 ModemManager
[15088.559268] [    809]   104   809    56123      424    81920        0             0 rsyslogd
[15088.559269] [    994]     0   994      622      128    40960        0             0 none
[15088.559269] [   1046]     0  1046     1704      571    53248        0             0 cron
[15088.559270] [   1053]     0  1053     1400      435    53248        0             0 agetty
[15088.559271] [   1055]     0  1055      949      519    40960        0             0 atd
[15088.559271] [   1057]     0  1057     1457      372    49152        0             0 agetty
[15088.559272] [   1211]     0  1211   307908      678   102400        0          -998 containerd-shim
[15088.559273] [   1216]     0  1216   307908      644   106496        0          -998 containerd-shim
[15088.559273] [   1247]     0  1247   307844      648   102400        0          -998 containerd-shim
[15088.559274] [   1289]     0  1289      241        1    28672        0          -998 pause
[15088.559275] [   1295]     0  1295      241        1    28672        0          -998 pause
[15088.559276] [   1315]     0  1315      241        1    28672        0          -998 pause
[15088.559276] [   1347]     0  1347   307844      699   106496        0          -998 containerd-shim
[15088.559277] [   1366]     0  1366   320708     2837   225280        0          -999 kube-proxy
[15088.559278] [   1381]     0  1381   307844      678   110592        0          -998 containerd-shim
[15088.559278] [   1445]     0  1445      975       28    49152        0          -997 cilium-envoy-st
[15088.559279] [   1467]     0  1467   570782     3164   253952        0          -997 cilium-envoy
[15088.559279] [   1926]     0  1926   307844      714   102400        0          -998 containerd-shim
[15088.559280] [   1946]     0  1946   352163    21844   479232        0          -997 cilium-agent
[15088.559281] [   2354]     0  2354   307399      703   102400        0          -997 cilium-health-r
[15088.559281] [   2960]     0  2960   307844      663   106496        0          -998 containerd-shim
[15088.559282] [   3009]     0  3009      241        1    28672        0          -998 pause
[15088.559282] [   3101]     0  3101   307908      713   106496        0          -998 containerd-shim
[15088.559283] [   3114]     0  3114   307844      673   106496        0          -998 containerd-shim
[15088.559283] [   3198]     0  3198      241        1    28672        0          -998 pause
[15088.559284] [   3204] 65532  3204   192207     3209   217088        0           992 coredns
[15088.559285] [   3264]     0  3264   307844      678   110592        0          -998 containerd-shim
[15088.559285] [   3293] 65532  3293   192143     3292   221184        0           992 coredns
[15088.559286] [   5360]     0  5360     3450     1014    65536        0             0 sshd
[15088.559287] [   5377]  1000  5377     4853      653    77824        0             0 systemd
[15088.559287] [   5378]  1000  5378    42443      953    98304        0             0 (sd-pam)
[15088.559288] [   5502]  1000  5502     3483      694    65536        0             0 sshd
[15088.559289] [   5503]  1000  5503     2069      906    57344        0             0 bash
[15088.559289] [   5512]     0  5512     2353      702    57344        0             0 sudo
[15088.559290] [   5513]     0  5513     1809      624    53248        0             0 bash
[15088.559291] [  10141]     0 10141   575216     9827   483328        0          -999 kubelet
[15088.559291] [  11290]     0 11290      652       22    40960        0             0 update-notifier
[15088.559292] [  11292]     0 11292     1369       48    49152        0             0 sleep
[15088.559293] [  11348]     0 11348   307908      651   102400        0          -998 containerd-shim
[15088.559293] [  11377]     0 11377      241        1    28672        0          -998 pause
[15088.559294] [  11420]     0 11420   307844      636   106496        0          -998 containerd-shim
[15088.559295] [  11448]     0 11448      241        1    28672        0          -998 pause
[15088.559295] [  11520]     0 11520   307844      641   102400        0          -998 containerd-shim
[15088.559296] [  11541]     0 11541      241        1    28672        0          -998 pause
[15088.559296] [  11552]     0 11552   307908      695   106496        0          -998 containerd-shim
[15088.559297] [  11575] 65534 11575   256245   256018  2097152        0          -997 ld-musl-x86_64.
[15088.559298] [  12881]     0 12881   307908      594   106496        0          -998 containerd-shim
[15088.559299] [  12914]     0 12914      241        1    28672        0          -998 pause
[15088.559300] [  12926]     0 12926   307844      676   102400        0          -998 containerd-shim
[15088.559300] [  12933]     0 12933   307908      624   102400        0          -998 containerd-shim
[15088.559301] [  12976]     0 12976      241        1    28672        0          -998 pause
[15088.559302] [  12987]     0 12987      241        1    28672        0          -998 pause
[15088.559302] [  13017]     0 13017   307908      641   102400        0          -998 containerd-shim
[15088.559303] [  13026]     0 13026   307908      659   102400        0          -998 containerd-shim
[15088.559303] [  13027]     0 13027   307908      664    98304        0          -998 containerd-shim
[15088.559304] [  13046]     0 13046   307844      613   106496        0          -998 containerd-shim
[15088.559305] [  13049]     0 13049   307908      664   114688        0          -998 containerd-shim
[15088.559305] [  13089] 65534 13089   256245   256008  2105344        0          -997 ld-musl-x86_64.
[15088.559306] [  13111] 65534 13111   256245   256010  2101248        0          -997 ld-musl-x86_64.
[15088.559307] [  13129]     0 13129      241        1    28672        0          -998 pause
[15088.559307] [  13147] 65534 13147   256245   256008  2105344        0          -997 ld-musl-x86_64.
[15088.559308] [  13153]     0 13153      241        1    28672        0          -998 pause
[15088.559309] [  13184]     0 13184   307908      664   102400        0          -998 containerd-shim
[15088.559309] [  13204]     0 13204   307844      688   106496        0          -998 containerd-shim
[15088.559310] [  13221] 65534 13221   256245   256006  2101248        0          -997 ld-musl-x86_64.
[15088.559311] [  13227] 65534 13227   256245   256010  2097152        0          -997 ld-musl-x86_64.
[15088.559311] [  13603]     0 13603   307844      637   106496        0          -998 containerd-shim
[15088.559312] [  13623] 65534 13623   256245   255965  2101248        0          1000 ld-musl-x86_64.
[15088.559313] [  13686]     0 13686   307700      503    98304        0          -998 containerd-shim
[15088.559313] [  13716] 65534 13716    77038    54221   491520        0           988 ld-musl-x86_64.
[15088.559314] oom-kill:constraint=CONSTRAINT_NONE,nodemask=(null),cpuset=/,mems_allowed=0,global_oom,task_memcg=/system.slice/containerd.service/kubepods-besteffort-pod569f0038_b213_4294_a1e2_4dcc5b2c8d9e.slice:cri-containerd:b5c977a23d9e75550f049867c1dcde53c5b4cd3b25d77364768bdde5ead50e98,task=ld-musl-x86_64.,pid=13623,uid=65534
[15088.559321] Out of memory: Killed process 13623 (ld-musl-x86_64.) total-vm:1024980kB, anon-rss:1023856kB, file-rss:4kB, shmem-rss:0kB, UID:65534 pgtables:2052kB oom_score_adj:1000
```


```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl describe node node2
<中略>
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests      Limits
  --------           --------      ------
  cpu                1 (25%)       600m (15%)
  memory             6280Mi (79%)  6370Mi (81%)
  ephemeral-storage  0 (0%)        0 (0%)
  hugepages-1Gi      0 (0%)        0 (0%)
  hugepages-2Mi      0 (0%)        0 (0%)
Events:
  Type     Reason     Age                  From     Message
  ----     ------     ----                 ----     -------
  Warning  SystemOOM  8m43s                kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 11415
  Warning  SystemOOM  8m31s                kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 11481
  Warning  SystemOOM  6m10s                kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 12147
  Warning  SystemOOM  5m43s                kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 12220
  Warning  SystemOOM  5m13s                kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13320
  Warning  SystemOOM  4m37s                kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13395
  Warning  SystemOOM  4m1s                 kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13469
  Warning  SystemOOM  3m4s                 kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13546
  Warning  SystemOOM  2m13s                kubelet  System OOM encountered, victim process: ld-musl-x86_64., pid: 13623
  Warning  SystemOOM  37s (x10 over 171m)  kubelet  (combined from similar events): System OOM encountered, victim process: ld-musl-x86_64., pid: 13716
```

besteffortとboostableのポッドは、node2から退避されることなく、node2上で再起動を繰り返している
```
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl describe node node2
tkr@hmc:~/memory-eater/k8s-yaml$ kubectl get pod -o wide && kubectl top node && kubectl top pod
NAME                          READY   STATUS             RESTARTS        AGE     IP           NODE    NOMINATED NODE   READINESS GATES
besteffort-657f6c859c-bmzds   0/1     CrashLoopBackOff   5 (40s ago)     16m     10.0.0.20    node2   <none>           <none>
boostable-7f657bc6d7-tplc4    1/1     Running            5 (2m14s ago)   16m     10.0.0.144   node2   <none>           <none>
guranteed-69875c65d6-2n9t5    1/1     Running            0               8m33s   10.0.0.41    node2   <none>           <none>
guranteed-69875c65d6-7dx8c    1/1     Running            0               8m33s   10.0.0.125   node2   <none>           <none>
guranteed-69875c65d6-dc8q6    1/1     Running            0               16m     10.0.0.107   node2   <none>           <none>
guranteed-69875c65d6-fgbzs    1/1     Running            0               8m33s   10.0.0.134   node2   <none>           <none>
guranteed-69875c65d6-g58nl    1/1     Running            0               8m33s   10.0.0.187   node2   <none>           <none>
guranteed-69875c65d6-vbq89    1/1     Running            0               8m33s   10.0.0.130   node2   <none>           <none>
NAME      CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
master1   89m          4%     1096Mi          28%       
node1     39m          0%     759Mi           9%        
node2     46m          1%     7080Mi          90%       
NAME                         CPU(cores)   MEMORY(bytes)   
boostable-7f657bc6d7-tplc4   9m           902Mi           
guranteed-69875c65d6-2n9t5   0m           1002Mi          
guranteed-69875c65d6-7dx8c   0m           1002Mi          
guranteed-69875c65d6-dc8q6   0m           1002Mi          
guranteed-69875c65d6-fgbzs   0m           1002Mi          
guranteed-69875c65d6-g58nl   0m           1002Mi          
guranteed-69875c65d6-vbq89   0m           1002Mi          
```