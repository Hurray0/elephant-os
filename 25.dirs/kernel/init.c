#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "../device/timer.h"
#include "memory.h"
#include "thread.h"
#include "console.h"
#include "keyboard.h"
#include "tss.h"
#include "syscall-init.h"
#include "ide.h"
#include "fs.h"

/* 负责初始化所有模块 */
void init_all() {
    put_str("init_all\n");
    idt_init(); // 初始化中断
    mem_init(); // 初始化内存管理系统
    thread_init(); // 初始化线程相关结构
    timer_init(); // 初始化定时器
    console_init(); // 控制台初始化
    keyboard_init(); // 键盘初始化
    tss_init(); // TSS初始化
    syscall_init();   // 初始化系统调用
    intr_enable();    // 后面的ide_init需要打开中断
    ide_init(); // 硬盘初始化
    filesys_init();   // 初始化文件系统
}