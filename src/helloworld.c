#include <linux/kernel.h>

asmlinkage int sys_helloworld(void) {
  printk("kaibro ininder\n");
  return 1;
}
