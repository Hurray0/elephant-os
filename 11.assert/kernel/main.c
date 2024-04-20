#include "debug.h"
#include "init.h"
#include "print.h"

int main(void) {
  put_str("I am kernel\n");
  init_all();
  // 因为目前是单线程，打开之后就没法测试assert了，会一直处理中断，所以这里注释掉
  //   asm volatile("sti");  // 为演示中断处理,在此临时开中断
  ASSERT(1 == 2);
  while (1)
    ;
  return 0;
}