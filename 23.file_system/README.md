# 编写硬盘驱动程序

__PS.对应书中第13章（编写硬盘驱动程序）__

本章编程没有太多难点。难点在于分区程序的编写。
在Mac中，`fdisk`使用与linux不同，需要使用`hdiutil`和`diskutil`命令来操作。
此外`bochs`/`qemu`/`virtualbox`虚拟机的从盘操作也不同，需要注意。

PPS. 本章在mac/linux上分区的硬盘文件(hd80M.img)分区大小、类型等不太一致，请忽略相关细节。

![13.硬盘驱动.svg](../doc/image/13.硬盘驱动.svg)