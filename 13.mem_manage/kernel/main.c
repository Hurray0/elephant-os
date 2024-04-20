#include "debug.h"
#include "init.h"
#include "print.h"
#include "string.h"
#include "memory.h"

static void test_memory(void); // 注意不能直接实现，否则main的地址就不是0xc0001500了


int main(void) {
  put_str("I am kernel\n");
  init_all();
  // 因为目前是单线程，打开之后就没法测试ASSERT了，会一直处理中断，所以这里注释掉
  //   asm volatile("sti");  // 为演示中断处理,在此临时开中断
  test_memory();
  while (1)
    ;
  return 0;
}

static void test_memory(void) {
  put_str("test_memory start\n");
  void *addr1 = get_kernel_pages(1);
  put_str("get_kernel_page start\n");
  void *addr2 = get_kernel_pages(1);
  put_str("get_kernel_page end\n");
  void *addr3 = get_kernel_pages(3);
  put_str("get_kernel_page end\n");
  put_str("test_memory end\n");
}