# 管道程序

__PS.对应书中第15章（系统交互）中的15.7节（管道）__

实现管道用户程序。

本节需要注意的是，`prog_pipe.c`中的数组初始化会导致运行报错。应该为手动设值。
```c
char buf[32] = {0};
```
改为
```c
char buf[32];
for (int i = 0; i < 32; i++) {
    buf[i] = 0;
}
```