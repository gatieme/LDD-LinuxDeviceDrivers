 /*                                                     
 * ftrace_demo.c 
 */                                                    
 #include <linux/init.h> 
 #include <linux/module.h> 
 #include <linux/kernel.h> 

 MODULE_LICENSE("GPL"); 

 static int ftrace_demo_init(void) 
 { 
     trace_printk("Can not see this in trace unless loaded for the second time\n"); 
     return 0; 
 } 

 static void ftrace_demo_exit(void) 
 { 
     trace_printk("Module unloading\n"); 
 } 

 module_init(ftrace_demo_init); 
 module_exit(ftrace_demo_exit);