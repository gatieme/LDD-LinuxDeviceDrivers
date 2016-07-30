#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/rtc.h>
#include <linux/ioctl.h>

int main(void)
{
	int rtc_fd;
	unsigned long data;
	int ret, i;
	struct rtc_time rtc_tm;
	char *rtc_dev = "/dev/rtc0";
	time_t t1, t2;
	rtc_fd = open(rtc_dev, O_RDONLY);
	if (rtc_fd == -1) {
		printf("failed to open '%s': %s\n", rtc_dev, strerror(errno));
		exit(1);
	} else
		printf("opened '%s': fd = %d\n", rtc_dev, rtc_fd);

	printf("Get RTC Time\n");
	ret = ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm);
	if (ret == -1) {
		perror("rtc ioctl RTC_RD_TIME error");
	}

	printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d\n",
		rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year,
		rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	rtc_tm.tm_mday = 31;
	rtc_tm.tm_mon = 4;
	rtc_tm.tm_year = 104;
	rtc_tm.tm_hour = 2;
	rtc_tm.tm_min = 30;
	rtc_tm.tm_sec = 0;

	printf("Set RTC Time\n");
	ret = ioctl(rtc_fd, RTC_SET_TIME, &rtc_tm);
	if (ret == -1) {
		perror("rtc ioctl RTC_SET_TIME error");
	}

	printf("Set Current RTC date/time to %d-%d-%d, %02d:%02d:%02d\n",
		rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year,
		rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	printf("Get RTC time\n");
	ret = ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm);
	if (ret == -1) {
		perror("rtc ioctl RTC_RD_TIME error");
	}

	printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d\n",
		rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year,
		rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	printf("RTC Tests done !!\n");
	close(rtc_fd);
	return 0;
}
