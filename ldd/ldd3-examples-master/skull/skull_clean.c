#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/ioport.h>

void skull_release(unsigned int port, unsigned int range)
{
    release_region(port,range);
}

void skull_cleanup(void)
{
    /* should put real values here ... */
    /* skull_release(0,0); */
}

module_exit(skull_cleanup);




