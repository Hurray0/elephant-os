#include "console.h"
#include "fs.h"
#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "syscall-init.h"
#include "syscall.h"
#include "thread.h"

void k_thread_a(void *);
void k_thread_b(void *);
void u_prog_a(void);
void u_prog_b(void);

int main(void) {
  put_str("I am kernel\n");
  init_all();
  process_execute(u_prog_a, "u_prog_a");
  process_execute(u_prog_b, "u_prog_b");
  thread_start("k_thread_a", 31, k_thread_a, "I am thread_a");
  thread_start("k_thread_b", 31, k_thread_b, "I am thread_b");

  // 新建文件
  uint32_t fd = sys_open("/file1", O_CREAT);
  sys_close(fd);

  // 写文件
  fd = sys_open("/file1", O_RDWR);
  printf("fd: %d\n", fd);
  char *str = "Hello, Hurray!\n";
  sys_write(fd, str, strlen(str));
  sys_close(fd);
  printf("%d closed now\n", fd);

  // 读文件
  fd = sys_open("/file1", O_RDWR);
  printf("open /file1, fd: %d\n", fd);
  char buf[64] = {0};
  int read_bytes = sys_read(fd, buf, 18);
  printf("1_ read %d bytes: %s\n", read_bytes, buf);

  memset(buf, 0, 64);
  read_bytes = sys_read(fd, buf, 6);
  printf("2_ read %d bytes: %s\n", read_bytes, buf);

  memset(buf, 0, 64);
  read_bytes = sys_read(fd, buf, 6);
  printf("3_ read %d bytes: %s\n", read_bytes, buf);

  // 关闭文件后再次打开
  printf("________  close file1 and reopen  ________\n");
  sys_close(fd);
  fd = sys_open("/file1", O_RDWR);
  memset(buf, 0, 64);
  read_bytes = sys_read(fd, buf, 24);
  printf("4_ read %d bytes:\n%s\n", read_bytes, buf);

  // 测试lseek
  printf("________  SEEK_SET 0  ________\n");
  sys_lseek(fd, 0, SEEK_SET);
  memset(buf, 0, 64);
  read_bytes = sys_read(fd, buf, 24);
  printf("5_ seek_set0 read %d bytes:\n%s\n", read_bytes, buf);

  sys_close(fd);

  // 删除文件, 需要之前的都确保sys_close
  printf("/file1 delete %s!\n", sys_unlink("/file1") == 0 ? "done" : "fail");
  while (1)
    ;
  return 0;
}

/* 在线程中运行的函数 */
void k_thread_a(void *arg) {
  void *addr1 = sys_malloc(256);
  void *addr2 = sys_malloc(255);
  void *addr3 = sys_malloc(254);
  console_put_str(" thread_a malloc addr:0x");
  console_put_int((int)addr1);
  console_put_char(',');
  console_put_int((int)addr2);
  console_put_char(',');
  console_put_int((int)addr3);
  console_put_char('\n');

  int cpu_delay = 100000;
  while (cpu_delay-- > 0)
    ;
  sys_free(addr1);
  sys_free(addr2);
  sys_free(addr3);
  while (1)
    ;
}

/* 在线程中运行的函数 */
void k_thread_b(void *arg) {
  void *addr1 = sys_malloc(256);
  void *addr2 = sys_malloc(255);
  void *addr3 = sys_malloc(254);
  console_put_str(" thread_b malloc addr:0x");
  console_put_int((int)addr1);
  console_put_char(',');
  console_put_int((int)addr2);
  console_put_char(',');
  console_put_int((int)addr3);
  console_put_char('\n');

  int cpu_delay = 100000;
  while (cpu_delay-- > 0)
    ;
  sys_free(addr1);
  sys_free(addr2);
  sys_free(addr3);
  while (1)
    ;
}

/* 测试用户进程 */
void u_prog_a(void) {
  void *addr1 = malloc(256);
  void *addr2 = malloc(255);
  void *addr3 = malloc(254);
  printf(" prog_a malloc addr:0x%x,0x%x,0x%x\n", (int)addr1, (int)addr2,
         (int)addr3);

  int cpu_delay = 100000;
  while (cpu_delay-- > 0)
    ;
  free(addr1);
  free(addr2);
  free(addr3);
  while (1)
    ;
}

/* 测试用户进程 */
void u_prog_b(void) {
  void *addr1 = malloc(256);
  void *addr2 = malloc(255);
  void *addr3 = malloc(254);
  printf(" prog_b malloc addr:0x%x,0x%x,0x%x\n", (int)addr1, (int)addr2,
         (int)addr3);

  int cpu_delay = 100000;
  while (cpu_delay-- > 0)
    ;
  free(addr1);
  free(addr2);
  free(addr3);
  while (1)
    ;
}