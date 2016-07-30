int version;
ioctl(fd, EVIOCGVERSION, &version)；//获取版本
struct input_devinfo device_info;
ioctl(fd, EVIOCGID, &device_info)；//获取设备信息
char name[256]= "Unknown";
ioctl(fd, EVIOCGNAME(sizeof(name)), name)//获取名称
uint8_t rel_bitmask[REL_MAX/8 + 1];
ioctl(fd, EVIOCGBIT(EV_REL, sizeof(rel_bitmask))//获取支持的鼠标特性
