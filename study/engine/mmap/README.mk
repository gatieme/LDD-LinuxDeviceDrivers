程序组成
map_driver.c 它是以模块形式加载的虚拟字符驱动程序。
该驱动负责将一定长的内核虚拟地址(vmalloc分配的)映射到设备文件上。

其中主要的函数有——vaddress_to_kaddress（）负责对vmalloc分配的地址进行页表解析,以
找到对应的内核物理映射地址（kmalloc分配的地址）；
map_nopage()负责在进程访问一个当前并不存在的VMA页时，寻找该地址对应的物理页，并返回该页的指针。


test.c 它利用上述驱动模块对应的设备文件在用户空间读取读取内核内存。结果可以看到内核虚拟地址的内容（ok!），被显示在了屏幕上。
	 
执行步骤

编译map_driver.c为map_driver.o模块,具体参数见Makefile

加载模块 ：insmod map_driver.o
	 生成对应的设备文件

1 在/proc/devices下找到map_driver对应的设备命和设备号：grep mapdrv /proc/devices

2 建立设备文件mknod  mapfile c 254 0  （在我的系统里设备号为254）

利用maptest读取mapfile文件，将取自内核的信息打印到屏幕上。
		  

全部程序下载 mmap.tar （感谢Martin Frey，该程序的主体出自他的灵感）
return(0);
http://blog.chinaunix.net/uid-24227137-id-3723898.html
