#include "timer.h"
#include "interrupt.h"
#include "io.h"
#include "print.h"
#include "thread.h"
#include "debug.h"

#define IRQ0_FREQUENCY 100      // 100Hz
#define INPUT_FREQUENCY 1193180 // 计数器0的工作脉冲信号频率为1193180Hz
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY // 计数初值
#define CONTRER0_PORT 0x40    // 计数器0的端口号
#define COUNTER0_NO 0         // 计数器0
#define COUNTER_MODE 2        // 二进制方式
#define READ_WRITE_LATCH 3    // 先读写低字节，再读写高字节
#define PIT_CONTROL_PORT 0x43 // 控制字寄存器端口

uint32_t ticks; // ticks是内核自中断开启以来总共的嘀嗒数

/* 把操作的计数器counter_no、读写锁属性rwl、计数器模式counter_mode写入模式控制寄存器并赋予初始值counter_value
 */
static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl,
                          uint8_t counter_mode, uint16_t counter_value) {
  /* 往控制字寄存器端口0x43中写入控制字 */
  outb(PIT_CONTROL_PORT,
       (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
  /* 先写入counter_value的低8位 */
  outb(counter_port, (uint8_t)counter_value);
  /* 再写入counter_value的高8位 */
  outb(counter_port, (uint8_t)counter_value >> 8);
}

/* 时钟的中断处理函数 */
static void intr_timer_handler(void) {
  struct task_struct *cur_thread = running_thread();

  ASSERT(cur_thread->stack_magic == 0x19870916); // 检查栈是否溢出

  cur_thread->elapsed_ticks++; // 记录此线程占用的cpu时间嘀
  ticks++; //从内核第一次处理时间中断后开始至今的滴哒数,内核态和用户态总共的嘀哒数

  if (cur_thread->ticks == 0) { // 若进程时间片用完就开始调度新的进程上cpu
    schedule();
  } else { // 将当前进程的时间片-1
    cur_thread->ticks--;
  }
}

/* 初始化PIT8253 */
void timer_init() {
  put_str("timer_init start\n");
  /* 设置8253的定时周期,也就是发中断的周期 */
  frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE,
                COUNTER0_VALUE);
  register_handler(0x20, intr_timer_handler); // 注册时钟的中断处理函数
  put_str("timer_init done\n");
}
