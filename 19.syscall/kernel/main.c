#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "print.h"
#include "process.h"
#include "thread.h"

static void k_thread_a(void *);
static void k_thread_b(void *);
void user_prog_a(void);
void user_prog_b(void);
int test_var_a = 0, test_var_b = 0;

int main(void) {
  put_str("I am kernel\n");
  init_all();
  thread_start("consumer_a", 31, k_thread_a, "argA ");
  thread_start("consumer_b", 31, k_thread_b, "argB ");
  process_execute(user_prog_a, "user_prog_a");
  process_execute(user_prog_b, "user_prog_b");
  intr_enable(); // 打开中断,使时钟中断起作用
  while (1) {
  }
  return 0;
}

static void k_thread_a(void *arg) {
  char *para = arg;
  while (1) {
    console_put_str(" v_a:0x");
    console_put_int(test_var_a);
  }
}

static void k_thread_b(void *arg) {
  char *para = arg;
  while (1) {
    console_put_str(" v_b:0x");
    console_put_int(test_var_b);
  }
}

void user_prog_a(void) {
  while (1) {
    test_var_a++;
  }
}

void user_prog_b(void) {
  while (1) {
    test_var_b++;
  }
}