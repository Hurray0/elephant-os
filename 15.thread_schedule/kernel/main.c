#include "debug.h"
#include "init.h"
#include "print.h"
#include "string.h"
#include "memory.h"
#include "thread.h"

static void k_thread_a(void*);


int main(void) {
  put_str("I am kernel\n");
  init_all();
  // 因为目前是单线程，打开之后就没法测试ASSERT了，会一直处理中断，所以这里注释掉
  //   asm volatile("sti");  // 为演示中断处理,在此临时开中断
  thread_start("k_thread_a", 31, k_thread_a, "argA ");
  while (1)
    ;
  return 0;
}

static void k_thread_a(void* arg) {
  char* para = arg;
  while (1) {
    put_str(para);
  }
}