#include "stdio.h"
#include "string.h"
#include "syscall.h"
int main(int argc, char **argv) {
  int32_t fd[2] = {-1};
  pipe(fd);
  int32_t pid = fork();

  if (pid) {      // 父进程
    close(fd[0]); // 关闭输入
    write(fd[1], "Hi, my son, I love you!", 24);
    printf("\nI`m father, my pid is %d\n", getpid());
    return 8;
  } else {
    char buf[32];
    for (int i = 0; i < 32; i++) {
      buf[i] = 0;
    }
    close(fd[1]); // 关闭输出
    read(fd[0], buf, 24);
    printf("\nI`m child, my pid is %d\n", getpid());
    printf("I`m child, my father said to me: \"%s\"\n", buf);
    //  printf("I am child, fd=%d\n", fd[0]);
    return 9;
  }
}
