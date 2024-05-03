#include "assert.h"
#include "console.h"
#include "dir.h"
#include "fs.h"
#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "shell.h"
#include "stdio.h"
#include "syscall-init.h"
#include "syscall.h"
#include "thread.h"

#include "ide.h"
#include "stdio-kernel.h"

void init(void);

int main(void) {
  put_str("I am kernel\n");
  init_all();

  /*************    写入应用程序    *************/
  uint32_t file_size = 4777;
  uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
  struct disk *sda = &channels[0].devices[0];
  void *prog_buf = sys_malloc(file_size);
  // ide_read(sda, 300, prog_buf, sec_cnt);
  for (uint32_t i = 0; i < sec_cnt; i++) {
    ide_read(sda, 300 + i, prog_buf + i * 512, 1);
  }
  int32_t fd = sys_open("/prog_no_arg", O_CREAT | O_RDWR);
  if (fd != -1) {
    if (sys_write(fd, prog_buf, file_size) == -1) {
      printk("file write error!\n");
      while (1)
        ;
    }
  }

  file_size = 5307;
  sec_cnt = DIV_ROUND_UP(file_size, 512);
  sda = &channels[0].devices[0];
  prog_buf = sys_malloc(file_size);
  // ide_read(sda, 300, prog_buf, sec_cnt);
  for (uint32_t i = 0; i < sec_cnt; i++) {
    ide_read(sda, 400 + i, prog_buf + i * 512, 1);
  }
  int32_t fd2 = sys_open("/prog_arg", O_CREAT | O_RDWR);
  if (fd2 != -1) {
    if (sys_write(fd2, prog_buf, file_size) == -1) {
      printk("file write error!\n");
      while (1)
        ;
    }
  }
  /*************    写入应用程序结束   *************/
  cls_screen();
  console_put_str("[rabbit@localhost /]$ ");
  while (1)
    ;
  return 0;
}

/* init进程 */
void init(void) {
  uint32_t ret_pid = fork();
  if (ret_pid) { // 父进程
    while (1)
      ;
  } else { // 子进程
    my_shell();
  }
  panic("init: should not be here");
}
