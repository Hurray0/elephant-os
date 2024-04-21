#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"

#define PG_SIZE 4096

// 由kernel_thread去执行function(func_arg)
static void kernel_thread(thread_func* function, void* func_arg) {
    function(func_arg);
}

// 初始化线程栈thread_stack，将待执行的函数和参数放到thread_stack中相应位置
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg) {
    // 先预留中断使用栈的空间，可见thread.h中定义的结构
    pthread->self_kstack -= sizeof(struct intr_stack);

    // 再预留线程栈空间，可见thread.h中定义的结构
    pthread->self_kstack -= sizeof(struct thread_stack);

    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;  // 线程第一次执行时，eip指向kernel_thread，eip的作用是指向函数的入口地址
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->edi = kthread_stack->esi = 0; // 其他寄存器置0，因为ABI规定，ebp/ebx/edi/esi/esp归主调函数所用，其余的寄存器归被调函数所用
}

// 初始化线程基本信息
void init_thread(struct task_struct* pthread, char* name, int prio) {
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);
    pthread->status = TASK_RUNNING;
    pthread->priority = prio;
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE); // 线程栈位于PCB结构之后, PCB预留一个页的空间
    pthread->stack_magic = 0x19870916; // 自定义的魔数
}

// 创建一个优先级为prio的线程，线程名为name，线程所执行的函数是function(func_arg)
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg) {
    // PCB内核空间中申请一个PCB结构
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread, name, prio);
    thread_create(thread, function, func_arg);

    asm volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret" : : "g" (thread->self_kstack) : "memory");   // g表示任何寄存器/内存都可以，memory表示内存有可能被修改
    // 在上面的汇编代码中，esp指向thread->self_kstack，然后弹出ebp/ebx/edi/esi，然后ret，ret会弹出eip（在thread_create中赋值），然后开始执行kernel_thread
    // 这里不直接调用kernel_thread是因为要切换新的线程栈，而不是直接在当前线程栈上执行kernel_thread
    return thread;
}