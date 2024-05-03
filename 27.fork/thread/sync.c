#include "sync.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"

// 初始化信号量
void sema_init(struct semaphore* psema, uint8_t value) {
    psema->value = value; // 为信号量赋初值
    list_init(&psema->waiters); // 初始化信号量的等待队列
}

// 初始化锁plock
void lock_init(struct lock* plock) {
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_init(&plock->semaphore, 1); // 信号量初值为1
}

// 信号量down操作
void sema_down(struct semaphore* psema) {
    // 关中断，保证原子操作
    enum intr_status old_status = intr_disable();
    while (psema->value == 0) { // 若value为0，表示无资源，需等待
        // 当前线程加入等待队列
        ASSERT(!elem_find(&psema->waiters, &running_thread()->general_tag));
        list_append(&psema->waiters, &running_thread()->general_tag);
        thread_block(TASK_BLOCKED); // 阻塞线程
    }
    psema->value--; // 资源数减1
    ASSERT(psema->value == 0 || list_empty(&psema->waiters));
    intr_set_status(old_status); // 恢复中断
}

// 信号量up操作
void sema_up(struct semaphore* psema) {
    // 关中断，保证原子操作
    enum intr_status old_status = intr_disable();
    if (!list_empty(&psema->waiters)) {
        struct task_struct* thread_blocked = elem2entry(struct task_struct, general_tag, list_pop(&psema->waiters));
        thread_unblock(thread_blocked); // 解除线程阻塞
    }
    psema->value++; // 资源数加1
    intr_set_status(old_status); // 恢复中断
}

// 获取锁plock
void lock_acquire(struct lock* plock) {
    // 若锁已被当前线程持有，重复申请
    if (plock->holder != running_thread()) {
        sema_down(&plock->semaphore); // 信号量P操作
        plock->holder = running_thread();
        ASSERT(plock->holder_repeat_nr == 0);
        plock->holder_repeat_nr = 1;
    } else {
        plock->holder_repeat_nr++;
    }
}

// 释放锁plock
void lock_release(struct lock* plock) {
    // 若当前线程不持有锁，无法释放
    ASSERT(plock->holder == running_thread());
    if (plock->holder_repeat_nr > 1) {
        plock->holder_repeat_nr--;
        return;
    }
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_up(&plock->semaphore); // 信号量V操作
}