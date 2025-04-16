
1.测试方法
插入内核模块后，自己创建设备文件 /dev/mmaptest

2. 输出
用户态程序 mmap_test_read 输出：
[root@linux mmap]# ./mmap_test_read 
[test 1    ---- access address: 7f51ae64d000]
[test 2    ---- access address: 7f51ae64e000]
[test 3    ---- access address: 7f51ae64f000]
[test 4    ---- access address: 7f51ae650000]
[test 5    ---- access address: 7f51ae651000]
[test 6    ---- access address: 7f51ae652000]
[test 7    ---- access address: 7f51ae653000]
[test 8    ---- access address: 7f51ae654000]
[test 9    ---- access address: 7f51ae655000]
[test 10   ---- access address: 7f51ae656000]

内核模块日志：
[936829.906747] MMAPTEST: process: mmap_test_read (3295)
[936829.906855] MMAPTEST: map_fault: map VMALLOC：0xffffc3310120a000 (PFN: 0x000000022f1ea000) to vm faulting address: 0x7f51ae64d000 , size: 0x1000, page:0 
[936829.906934] MMAPTEST: map_fault: map VMALLOC：0xffffc3310120b000 (PFN: 0x00000002359e6000) to vm faulting address: 0x7f51ae64e000 , size: 0x1000, page:1 
[936829.906989] MMAPTEST: map_fault: map VMALLOC：0xffffc3310120c000 (PFN: 0x000000022f0ca000) to vm faulting address: 0x7f51ae64f000 , size: 0x1000, page:2 
[936829.907017] MMAPTEST: map_fault: map VMALLOC：0xffffc3310120d000 (PFN: 0x000000022d872000) to vm faulting address: 0x7f51ae650000 , size: 0x1000, page:3 
[936829.907044] MMAPTEST: map_fault: map VMALLOC：0xffffc3310120e000 (PFN: 0x0000000265e93000) to vm faulting address: 0x7f51ae651000 , size: 0x1000, page:4 
[936829.907082] MMAPTEST: map_fault: map VMALLOC：0xffffc3310120f000 (PFN: 0x00000002617f7000) to vm faulting address: 0x7f51ae652000 , size: 0x1000, page:5 
[936829.907109] MMAPTEST: map_fault: map VMALLOC：0xffffc33101210000 (PFN: 0x0000000245942000) to vm faulting address: 0x7f51ae653000 , size: 0x1000, page:6 
[936829.907136] MMAPTEST: map_fault: map VMALLOC：0xffffc33101211000 (PFN: 0x000000022da61000) to vm faulting address: 0x7f51ae654000 , size: 0x1000, page:7 
[936829.907175] MMAPTEST: map_fault: map VMALLOC：0xffffc33101212000 (PFN: 0x0000000265ecf000) to vm faulting address: 0x7f51ae655000 , size: 0x1000, page:8 
[936829.907210] MMAPTEST: map_fault: map VMALLOC：0xffffc33101213000 (PFN: 0x0000000241807000) to vm faulting address: 0x7f51ae656000 , size: 0x1000, page:9 
[936836.000843] MMAPTEST: mapping vma is closed