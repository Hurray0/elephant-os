#include "init.h"
#include "print.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "ioqueue.h"
#include "keyboard.h"

static void k_thread_a(void*);
static void k_thread_b(void*);

int main(void) {
  put_str("I am kernel\n");
  init_all();
  thread_start("consumer_a", 31, k_thread_a, "A_");
  thread_start("consumer_b", 31, k_thread_b, "B_");
  intr_enable();	// 打开中断,使时钟中断起作用
  while (1) {
  }
  return 0;
}

static void k_thread_a(void* arg) {
  char* para = arg;
  while (1) {
    enum intr_status old_status = intr_disable();
    if (!ioq_empty(&kbd_buf)) {
      console_put_str(arg);
      char byte = ioq_getchar(&kbd_buf);
      console_put_char(byte);
    }
    intr_set_status(old_status);
  }
}

static void k_thread_b(void* arg) {
  char* para = arg;
  while (1) {
    enum intr_status old_status = intr_disable();
    if (!ioq_empty(&kbd_buf)) {
      console_put_str(arg);
      char byte = ioq_getchar(&kbd_buf);
      console_put_char(byte);
    }
    intr_set_status(old_status);
  }
}