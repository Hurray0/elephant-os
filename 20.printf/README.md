# 格式化打印

__PS.对应书中第12章（进一步完善内核）中的12.3节（让用户进程“说话”）__

本章的重点在于可变长参数的理解和应用。可以通过指针递增来访问后续参数。原因是可变长参数在栈中是连续存放的。

```c
typedef char *va_list;
#define va_start(ap, v) ap = (va_list) &v // 把ap指向第一个固定参数v
#define va_arg(ap, t) *((t *)(ap += 4)) // ap指向下一个参数并返回其值
#define va_end(ap) ap = NULL            // 清除ap

void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);  // args指向第一个固定参数format
    // ... 之后使用va_arg(args, type)来获取下一个参数
}
```