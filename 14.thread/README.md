# 线程

__PS.对应书中第9章（线程）中的9.3节__

本章还未进行线程调度，只是实现了主线程到新线程栈空间的切换，并进入新线程执行。

需要实现PCB（进程控制块）和线程栈的保存。在线程执行时修改栈指针`esp`，使其指向新线程的栈空间。

![9.操作系统：线程.svg](../doc/image/9.操作系统：线程.svg)