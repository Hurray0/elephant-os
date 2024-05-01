#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "interrupt.h"
#include "debug.h"
#include "process.h"
#include "sync.h"

#define PG_SIZE 4096

struct task_struct* main_thread; // 主线程PCB
struct list thread_ready_list;   // 就绪队列
struct list thread_all_list;     // 所有任务队列
static struct list_elem* thread_tag; // 用于保存队列中的线程节点
struct lock pid_lock; // 分配pid锁

extern void switch_to(struct task_struct* cur, struct task_struct* next);

static pid_t allocate_pid(void) {
    static pid_t next_pid = -1;  // 这里和qemu不一样，qemu中初始化0并不执行
    lock_acquire(&pid_lock);
    if (next_pid == -1) next_pid = 0;
    next_pid++;
    lock_release(&pid_lock);
    return next_pid;
}

// 获取当前线程PCB指针
struct task_struct* running_thread() {
    uint32_t esp;
    asm ("mov %%esp, %0" : "=g" (esp));
    // 取esp整数部分作为PCB起始地址
    return (struct task_struct*)(esp & 0xfffff000);
}

// 由kernel_thread去执行function(func_arg)
static void kernel_thread(thread_func* function, void* func_arg) {
    // 执行function前要开中断，避免后面的时钟中断被屏蔽，而无法调度其他线程
    intr_enable();
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
    pthread->pid = allocate_pid();
    strcpy(pthread->name, name);

    if (pthread == main_thread) {
        pthread->status = TASK_RUNNING;
    } else {
        pthread->status = TASK_READY;
    }

    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE); // 线程栈位于PCB结构之后, PCB预留一个页的空间
    pthread->priority = prio;
    pthread->ticks = prio;
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;
    pthread->stack_magic = 0x19870916; // 自定义的魔数
}

// 创建一个优先级为prio的线程，线程名为name，线程所执行的函数是function(func_arg)
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg) {
    // PCB内核空间中申请一个PCB结构
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread, name, prio);
    thread_create(thread, function, func_arg);

    // 确保之前不在队列中
    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    // 加入就绪队列
    list_append(&thread_ready_list, &thread->general_tag);

    // 确保之前不在队列中
    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    // 加入所有线程队列
    list_append(&thread_all_list, &thread->all_list_tag);

    // asm volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret" : : "g" (thread->self_kstack) : "memory");   // g表示任何寄存器/内存都可以，memory表示内存有可能被修改
    // // 在上面的汇编代码中，esp指向thread->self_kstack，然后弹出ebp/ebx/edi/esi，然后ret，ret会弹出eip（在thread_create中赋值），然后开始执行kernel_thread
    // // 这里不直接调用kernel_thread是因为要切换新的线程栈，而不是直接在当前线程栈上执行kernel_thread
    return thread;
}

// 将kernel中的main函数完善为主线程
static void make_main_thread(void) {
    // 因为main线程早已运行，咱们在loader.S中进入内核时的mov esp, 0xc009f000就是为其准备的
    // 因此pcb地址为0xc009e000, 因为0xc009f000-0xc009e000=0x1000=PG_SIZE，不需要再分配内存
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);

    // main函数是当前线程，当前线程不在thread_ready_list中，所以只加入thread_all_list
    ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

/* 实现任务调度 */
void schedule() {

    ASSERT(intr_get_status() == INTR_OFF);

    struct task_struct* cur = running_thread();
    if (cur->status == TASK_RUNNING) { // 若此线程只是cpu时间片到了,将其加入到就绪队列尾
        ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        list_append(&thread_ready_list, &cur->general_tag);
        cur->ticks = cur->priority;     // 重新将当前线程的ticks再重置为其priority;
        cur->status = TASK_READY;
    } else {
        /* 若此线程需要某事件发生后才能继续上cpu运行,
        不需要将其加入队列,因为当前线程不在就绪队列中。*/
    }

    ASSERT(!list_empty(&thread_ready_list));
    thread_tag = NULL;	  // thread_tag清空
    /* 将thread_ready_list队列中的第一个就绪线程弹出,准备将其调度上cpu. */
    thread_tag = list_pop(&thread_ready_list);
    struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
    next->status = TASK_RUNNING;

    // 激活任务页表等
    process_activate(next);

    switch_to(cur, next);
}

/* 初始化线程环境 */
void thread_init(void) {
    put_str("thread_init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    lock_init(&pid_lock);
    /* 将当前main函数创建为线程 */
    make_main_thread();
    put_str("thread_init done\n");
}

/* 当前线程将自己阻塞,标志其状态为stat. */
void thread_block(enum task_status stat) {
    /* stat取值为TASK_BLOCKED,TASK_WAITING,TASK_HANDING,不会是TASK_RUNNING */
    ASSERT(((stat == TASK_BLOCKED) || (stat == TASK_WAITING) || (stat == TASK_HANGING)));
    enum intr_status old_status = intr_disable();
    struct task_struct* cur_thread = running_thread();
    cur_thread->status = stat; // 置其状态为stat
    schedule(); // 将当前线程换下处理器
    intr_set_status(old_status); // 恢复中断
}

/* 将线程pthread解除阻塞 */
void thread_unblock(struct task_struct* pthread) {
    enum intr_status old_status = intr_disable();
    ASSERT(((pthread->status == TASK_BLOCKED) || (pthread->status == TASK_WAITING) || (pthread->status == TASK_HANGING)));
    if (pthread->status != TASK_READY) {
        ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag));
        if (elem_find(&thread_ready_list, &pthread->general_tag)) {
            PANIC("thread_unblock: blocked thread in ready_list\n");
        }
        pthread->status = TASK_READY;
        list_push(&thread_ready_list, &pthread->general_tag);
    }
    intr_set_status(old_status);
}