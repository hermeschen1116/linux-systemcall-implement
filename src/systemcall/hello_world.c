#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(hello_world) {
  printk("Finish.\n");
  return 0;
}
