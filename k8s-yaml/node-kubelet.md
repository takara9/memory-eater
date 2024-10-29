

- oomScoreAdj: oomScoreAdj is The oom-score-adj value for kubelet process. Values must be within the range [-1000, 1000]. Default: -999

- evictionHard: evictionHard is a map of signal names to quantities that defines hard eviction thresholds
- evictionSoft: evictionSoft is a map of signal names to quantities that defines soft eviction thresholds
- evictionSoftGracePeriod: evictionSoftGracePeriod is a map of signal names to quantities that defines grace periods for each soft eviction signal
- evictionPressureTransitionPeriod: evictionPressureTransitionPeriod is the duration for which the kubelet has to wait before transitioning out of an eviction pressure condition
- evictionMaxPodGracePeriod: evictionMaxPodGracePeriod is the maximum allowed grace period (in seconds) to use when terminating pods in response to a soft eviction threshold being met
- evictionMinimumReclaim: evictionMinimumReclaim is a map of signal names to quantities that defines minimum reclaims, which describe the minimum amount of a given resource the kubelet will reclaim when performing a pod eviction while that resource is under pressure

- systemReserved: systemReserved is a set of ResourceName=ResourceQuantity (e.g. cpu=200m,memory=150G) pairs that describe resources reserved for non-kubernetes components. Currently only cpu and memory are supported. 
- kubeReserved: kubeReserved is a set of ResourceName=ResourceQuantity (e.g. cpu=200m,memory=150G) pairs that describe resources reserved for kubernetes system components. Currently cpu, memory and local storage for root file system are supported.
- reservedSystemCPUs [Required]: The reservedSystemCPUs option specifies the CPU list reserved for the host level system threads and kubernetes related threads. This provide a "static" CPU list rather than the "dynamic" list by systemReserved and kubeReserved. This option does not support systemReservedCgroup or kubeReservedCgroup.
- systemReservedCgroup: systemReservedCgroup helps the kubelet identify absolute name of top level CGroup used to enforce systemReserved compute resource reservation for OS system daemons.
- kubeReservedCgroup: kubeReservedCgroup helps the kubelet identify absolute name of top level CGroup used to enforce KubeReserved compute resource reservation for Kubernetes node system daemons.
- enforceNodeAllocatable: This flag specifies the various Node Allocatable enforcements that Kubelet needs to perform. This flag accepts a list of options. Acceptable options are none, pods, system-reserved and kube-reserved. If none is specified, no other options may be specified. When system-reserved is in the list, systemReservedCgroup must be specified. When kube-reserved is in the list, kubeReservedCgroup must be specified. This field is supported only when cgroupsPerQOS is set to true.
- reservedMemory: reservedMemory specifies a comma-separated list of memory reservations for NUMA nodes. The parameter makes sense only in the context of the memory manager feature. The memory manager will not allocate reserved memory for container workloads. For example, if you have a NUMA0 with 10Gi of memory and the reservedMemory was specified to reserve 1Gi of memory at NUMA0, the memory manager will assume that only 9Gi is available for allocation.


memory.available := node.status.capacity[memory] - node.stats.memory.workingSet

On Linux nodes, the value for memory.available is derived from the cgroupfs instead of tools like free -m. This is important because free -m does not work in a container, and if users use the node allocatable feature, out of resource decisions are made local to the end user Pod part of the cgroup hierarchy as well as the root node.

This script or cgroupv2 script reproduces the same set of steps that the kubelet performs to calculate memory.available. 

The kubelet excludes inactive_file (the number of bytes of file-backed memory on the inactive LRU list) from its calculation, as it assumes that memory is reclaimable under pressure.


- Evictionは、available に対する閾値
- CapacityとAllocatableは、目安であり使われていない？
Hard

- systemReserved と kubeReserved は、スケジュールの際に使用する

- evictionHard と evictionSoft　のどちらを使うべきか？
