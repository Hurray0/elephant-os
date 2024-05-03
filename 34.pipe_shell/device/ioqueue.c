#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"

/* 初始化 ioqueue */
void ioqueue_init(struct ioqueue* ioq) {
    lock_init(&ioq->lock);	// 初始化ioqueue的锁
    ioq->producer = ioq->consumer = NULL;	// 生产者和消费者置空
    ioq->head = ioq->tail = 0;	// 队列的首尾指针指向缓冲区数组第0个位置
}

/* 返回pos在缓冲区中的下一个位置值 */
static int32_t next_pos(int32_t pos) {
    return (pos + 1) % bufsize;
}

/* 判断队列是否已满 */
int ioq_full(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);
    return next_pos(ioq->head) == ioq->tail;
}

/* 判断队列是否为空 */
int ioq_empty(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);
    return ioq->head == ioq->tail;
}

/* 使当前生产者或消费者在此缓冲区上等待 */
static void ioq_wait(struct task_struct** waiter) {
    ASSERT(*waiter == NULL && waiter != NULL);
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}

/* 唤醒waiter */
static void wakeup(struct task_struct** waiter) {
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter);
    *waiter = NULL;
}

/* 消费者从ioq队列中获取一个字符 */
char ioq_getchar(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);

    /* 若缓冲区(队列)为空,把消费者ioq->consumer记为当前线程,
     * 直接ioq_wait(),把当前线程阻塞,等待生产者唤醒 */
    while (ioq_empty(ioq)) {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->consumer);
        lock_release(&ioq->lock);
    }

    char byte = ioq->buf[ioq->tail];	// 从缓冲区中取出
    ioq->tail = next_pos(ioq->tail);	// 移动尾指针
    if (ioq->producer != NULL) {	// 如果生产者不为空
        wakeup(&ioq->producer);	// 唤醒生产者
    }
    return byte;
}

/* 生产者往ioq队列中写入一个字符byte */
void ioq_putchar(struct ioqueue* ioq, char byte) {
    ASSERT(intr_get_status() == INTR_OFF);

    /* 若缓冲区(队列)已满,把生产者ioq->producer记为当前线程,
     * 直接ioq_wait(),把当前线程阻塞,等待消费者唤醒 */
    while (ioq_full(ioq)) {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->producer);
        lock_release(&ioq->lock);
    }

    ioq->buf[ioq->head] = byte;	// 写入缓冲区
    ioq->head = next_pos(ioq->head);	// 移动头指针
    if (ioq->consumer != NULL) {	// 如果消费者不为空
        wakeup(&ioq->consumer);	// 唤醒消费者
    }
}

/* 返回环形缓冲区中的数据长度 */
uint32_t ioq_length(struct ioqueue* ioq) {
    uint32_t len = 0;
    if (ioq->head >= ioq->tail) {
        len = ioq->head - ioq->tail;
    } else {
        len = bufsize - (ioq->tail - ioq->head);
    }
    return len;
}