# 内存管理系统

__PS.对应书中第8章（内存管理系统）中的8.5（内存管理系统）__

通过位图的实现，来判断内存的使用情况。

内存分配需要申请连续的虚拟内存，按1页大小申请物理内存，然后将虚拟内存和物理内存进行映射。


![8.操作系统：内存管理.svg](../doc/image/8.操作系统：内存管理.svg)