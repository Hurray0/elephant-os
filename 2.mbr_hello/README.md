# 实现hello
__PS.对应书中第2章__

开机启动时，从BIOS入口地址(0xFFFF0)跳转BIOS。BIOS会检测硬件，然后加载引导扇区(硬盘第0个扇区)到内存`0x7c00`处，然后跳转到`0x7c00`处执行引导扇区。
本章使用汇编，实现MBR的功能，即从`0x7c00`处开始执行的程序。
这里执行了一个打印的例子，原理是调用了0x10号中断.

![1.BIOS.svg](../doc/image/1.BIOS.svg)