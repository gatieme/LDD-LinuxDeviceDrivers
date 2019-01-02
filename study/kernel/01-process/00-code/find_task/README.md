https://blog.csdn.net/onetwothreef/article/details/49932579

```cpp
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	pcb_tmp = find_task_by_pid(pid);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)
	pcb_tmp = find_task_by_vpid(pid);
#else
	pcb_tmp = pid_task(find_vpid(pid), PIDTYPE_PID);
#endif
```
