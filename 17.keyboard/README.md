# 从键盘获取输入

__PS.对应书中第10章（输入输出系统）中的10.3节~10.5节__

本章实现了一个简单的键盘驱动程序，可以从键盘获取输入。键盘驱动程序的主要功能是从键盘获取输入，然后将输入的字符存储在一个缓冲区中。当用户按下键盘上的某个键时，键盘控制器会产生一个中断信号，然后键盘驱动程序会从键盘控制器中读取输入的字符，并将其存储在缓冲区中。

在main中，我们创建了两个消费者线程用来消费键盘输入。