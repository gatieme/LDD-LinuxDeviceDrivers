

#lnux-2.6.36以上内核加载编译驱动时，出现

```c
error:unknown field 'ioctl' specified in initializer
```

原因是：在2.6.36内核上file_operations发生了重大的改变：
原先的

```c
int (*ioctl)(struct inode*, struct file*, unsigned int, unsigned long);
```

被改为了

```c
long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);

long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
```
