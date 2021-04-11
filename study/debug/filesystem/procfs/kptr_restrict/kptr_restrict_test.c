#include <linux/init.h>
#include <linux/module.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/percpu.h>
#include <linux/sched.h>
#include <linux/version.h>


#define SIZE 10
int array[SIZE];

int *p_arr = array;
static int __init exam_seq_init(void)
{
	pr_info("%%p = %p, %%pK = %pK, %%px = %px\n", p_arr, p_arr, p_arr);
	pr_info("%%p = %p, %%pK = %pK, %%px = %px\n", &p_arr, &p_arr, &p_arr);

	return 0;
}

static void __exit exam_seq_exit(void)
{

}

module_init(exam_seq_init);
module_exit(exam_seq_exit);
MODULE_LICENSE("GPL");
