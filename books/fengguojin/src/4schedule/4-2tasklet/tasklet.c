#include <linux/interrupt.h> 

int myint_for_something=1;
void tasklet_function(unsigned long);
char tasklet_data[64];

DECLARE_TASKLET(test_tasklet,tasklet_function, (unsigned long) &tasklet_data);

void tasklet_function(unsigned long data)
{
	struct timeval now;
	do_gettimeofday(&now);
	printk("%s at %ld,%ld\n",
		(char *) data,
		now.tv_sec,
		now.tv_usec);
}

int init_module_task(void) 
{
	sprintf(tasklet_data,"%s\n",
		"Linux tasklet called in init_module");
	tasklet_schedule(&test_tasklet);
}

void cleanup_moduletask(void)
{
	return ;
}

module_init(init_module_task);
module_exit(cleanup_moduletask);