#include <linux/kernel.h>
#include <linux/linkage.h>

asmlinkage int sys_helloworld(void) {
  printk(KERN_EMERG "By it.livekn.com");

  return 1;
}
