http://blog.chinaunix.net/uid-14528823-id-4394419.html

https://blog.csdn.net/hongzg1982/article/details/54880674

https://blog.csdn.net/u010278923/article/details/79893477

https://blog.csdn.net/moe26/article/details/10326017

https://www.cnblogs.com/yfz0/p/5829443.html

https://www.cnblogs.com/arnoldlu/p/8251333.html

https://blog.csdn.net/qianlong4526888/article/details/8840128

https://www.cnblogs.com/wzw200/p/3741340.html


一个节点一个bootmem_data结构，初始化的参数主要有:
min_low_pfn 系统中可用的最小PFN
max_low_pfn以低端内存域表示的最大PFN
highstart_pfn 高端内存区域的起始PFN
highend_pfn 高端内存区域的最后一个PFN
max_pfn 表示系统中可用的最大PFN
引导内存分配器在系统初始化start_kernel的时候被释放掉。接下来由伙伴分配器接管。
PFN全称应该是Page Frame Number
