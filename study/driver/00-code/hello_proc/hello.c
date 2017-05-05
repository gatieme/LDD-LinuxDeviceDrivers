/*
 * "Hello, world!" minimal kernel module - /proc version
 *
 * Valerie Henson <val@nmt.edu>
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

/*
 * Write "Hello, world!" to the buffer passed through the /proc file.
 */

static int
hello_read_proc(char *buffer, char **start, off_t offset, int size, int *eof,
        void *data)
{
    char *hello_str = "Hello, world!\n";
    int len = strlen(hello_str); /* Don't include the null byte. */
    /*
     * We only support reading the whole string at once.
     */
    if (size < len)
        return -EINVAL;
    /*
     * If file position is non-zero, then assume the string has
     * been read and indicate there is no more data to be read.
     */
    if (offset != 0)
        return 0;
    /*
     * We know the buffer is big enough to hold the string.
     */
    strcpy(buffer, hello_str);
    /*
     * Signal EOF.
     */
    *eof = 1;

    return len;

}

static int __init
hello_init(void)
{
    /*
     * Create an entry in /proc named "hello_world" that calls
     * hello_read_proc() when the file is read.
     */
    if (create_proc_read_entry("hello_world", 0, NULL, hello_read_proc,
                    NULL) == 0) {
        printk(KERN_ERR
               "Unable to register \"Hello, world!\" proc file\n");
        return -ENOMEM;
    }

    return 0;
}

module_init(hello_init);

static void __exit
hello_exit(void)
{
    remove_proc_entry("hello_world", NULL);
}

module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valerie Henson <val@nmt.edu>");
MODULE_DESCRIPTION("\"Hello, world!\" minimal module");
MODULE_VERSION("proc");
