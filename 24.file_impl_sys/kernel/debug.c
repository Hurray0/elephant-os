#include "debug.h"
#include "interrupt.h"
#include "print.h"

void panic_spin(char *filename, int line, const char *func,
                const char *condition) {
  // 关中断
  int r = intr_disable();
  put_str("before intr:\n");
  put_int(r);
  put_str("\n");
  int cur = intr_get_status();
  put_str("after intr:\n");
  put_int(cur);
  put_str("\n");

  put_str("\n\n\n!!!!! error !!!!!\n");
  put_str("filename: ");
  put_str(filename);
  put_str("\n");
  put_str("line: 0x");
  put_int(line);
  put_str("\n");
  put_str("function: ");
  put_str((char *)func);
  put_str("\n");
  put_str("condition: ");
  put_str((char *)condition);
  put_str("\n");
  while (1)
    ;
}
