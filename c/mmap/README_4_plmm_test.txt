
在MCS上运行，
[19:36:31] newinstall:~ # /usr/sbin/plmm_test 
PLMM  version = 0x8
PLMM mq_num_instances = 0x1
show instance 1 /dev/plmm base info
number of iob page is   448610
number of pd page is    79319
number of gen page is   22513245
PLMM test : finish show , press x to exit!

查看mmap情况：
[19:37:20] newinstall:~ # cat /proc/28114/maps
00400000-00402000 r-xp 00000000 08:02 14629                              /usr/sbin/plmm_test
00601000-00602000 r--p 00001000 08:02 14629                              /usr/sbin/plmm_test
00602000-00603000 rw-p 00002000 08:02 14629                              /usr/sbin/plmm_test
01a62000-01a83000 rw-p 00000000 00:00 0                                  [heap]
// 映射mq_base, 共 22513245个Page
7f5766da6000-7f6cdf403000 r--s 00003000 00:05 21877                      /dev/plmm
// 映射mq_pd， 共79319个Page
7f6cdf403000-7f6cf29da000 r--s 00000000 00:05 21877                      /dev/plmm
// 映射mq_it, 共 448610个Page
7f6cf29da000-7f6d6023c000 r--s 00001000 00:05 21877                      /dev/plmm
7f6d6023c000-7f6d603f6000 r-xp 00000000 08:02 7147                       /usr/lib64/libc-2.28.so
7f6d603f6000-7f6d605f6000 ---p 001ba000 08:02 7147                       /usr/lib64/libc-2.28.so
7f6d605f6000-7f6d605fa000 r--p 001ba000 08:02 7147                       /usr/lib64/libc-2.28.so
7f6d605fa000-7f6d605fc000 rw-p 001be000 08:02 7147                       /usr/lib64/libc-2.28.so
7f6d605fc000-7f6d60600000 rw-p 00000000 00:00 0 
7f6d60600000-7f6d60628000 r-xp 00000000 08:02 7118                       /usr/lib64/ld-2.28.so
7f6d6081d000-7f6d6081f000 rw-p 00000000 00:00 0 
7f6d60828000-7f6d60829000 r--p 00028000 08:02 7118                       /usr/lib64/ld-2.28.so
7f6d60829000-7f6d6082a000 rw-p 00029000 08:02 7118                       /usr/lib64/ld-2.28.so
7f6d6082a000-7f6d6082b000 rw-p 00000000 00:00 0 
7ffd84095000-7ffd840b6000 rw-p 00000000 00:00 0                          [stack]
7ffd84141000-7ffd84142000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]