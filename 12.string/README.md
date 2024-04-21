# 字符串功能

__PS.对应书中第8章（内存管理系统）中的8.3节（实现字符串操作函数）__

本章实现了类似`<string.h>`中的函数功能，例如`memset`/`memcpy`/`memcmp`/`strcpy`/`strlen`/`strcmp`/`strchr`/`strrchr`/`strcat`/`strchrs`等。

本章需要注意的是：
* 使用`char str1[20]`定义字符串时的注意事项（string测试用例）
在macOS下gcc编译`char str11[20]`的用法会使用movdqa、movups命令，这两个指令在bochs中会报错。因此需要在g++编译时加上`-mno-sse`参数。