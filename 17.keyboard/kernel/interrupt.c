#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "print.h"
#include "stdint.h"

#define IDT_DESC_CNT 0x30 // 目前只定义了 0x30 个中断描述符
#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

#define EFLAGS_IF 0x00000200 // eflags寄存器中的if位为1

/* 中断门描述符结构体 */
struct gate_desc {
  uint16_t func_offset_low_word;
  uint16_t selector;
  uint8_t dcount; // 此项为双字计数字段，是门描述符中的第 4 字节, 为固定值
  uint8_t attribute; // P(1) DPL(2) DT(1) TYPE(4)
  uint16_t func_offset_high_word;
};

// 静态函数声明，非必须
static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr,
                          intr_handler function);
static struct gate_desc
    idt[IDT_DESC_CNT]; // idt 是中断描述符表，本质上就是个中断门描述符数组
char *intr_name[IDT_DESC_CNT]; // 用于保存异常的名字
intr_handler idt_table
    [IDT_DESC_CNT]; // 定义中断处理程序数组.在kernel.S中定义的intrXXentry只是中断处理程序的入口,最终调用的是ide_table中的处理程序
extern intr_handler intr_entry_table
    [IDT_DESC_CNT]; // 声明引用定义在kernel.S中的中断处理函数入口数组

static void pic_init(void) {
  /* 初始化主片 */
  outb(PIC_M_CTRL, 0x11); // ICW1: 边沿触发，级联8259，需要 ICW4
  outb(PIC_M_DATA, 0x20); // ICW2: 起始中断向量号为 0x20
  outb(PIC_M_DATA, 0x04); // ICW3: IR2 接从片
  outb(PIC_M_DATA, 0x01); // ICW4: 8086 模式，正常 EOI

  /* 初始化从片 */
  outb(PIC_S_CTRL, 0x11); // ICW1: 边沿触发，级联8259，需要 ICW4
  outb(PIC_S_DATA, 0x28); // ICW2: 起始中断向量号为 0x28
  outb(PIC_S_DATA, 0x02); // ICW3: 设置从片连接到主片的 IR2 引脚
  outb(PIC_S_DATA, 0x01); // ICW4: 8086 模式，正常 EOI

  /* 打开主片上 IR0, 即目前中断打开：时钟、键盘 */
  outb(PIC_M_DATA, 0xfc);
  outb(PIC_S_DATA, 0xff);

  put_str("   pic_init done\n");
}

/* 创建中断门描述符 */
static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr,
                          intr_handler function) {
  p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF;
  p_gdesc->selector = SELECTOR_K_CODE;
  p_gdesc->dcount = 0;
  p_gdesc->attribute = attr;
  p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}

/* 初始化中断描述符表 */
static void idt_desc_init(void) {
  int i;
  for (i = 0; i < IDT_DESC_CNT; i++) {
    make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
  }
  put_str("   idt_desc_init done\n");
}

/* 通用的中断处理函数,一般用在异常出现时的处理 */
static void general_intr_handler(uint8_t vec_nr) {
  if (vec_nr == 0x27 ||
      vec_nr == 0x2f) { // 0x2f是从片8259A上的最后一个irq引脚，保留
    return; // IRQ7和IRQ15会产生伪中断(spurious interrupt),无须处理。
  }
  /* 将光标置为0,从屏幕左上角清出一片打印异常信息的区域,方便阅读 */
  set_cursor(0);
  int cursor_pos = 0;
  while (cursor_pos < 320) {  // 320 = 80*4
    put_char(' ');
    cursor_pos++;
  }

  set_cursor(0); // 重置光标为屏幕左上角
  put_str("!!!!!!!      excetion message begin  !!!!!!!!\n");
  set_cursor(88); // 从第2行第8个字符开始打印
  put_str(intr_name[vec_nr]);
  if (vec_nr == 14) { // 若为Pagefault,将缺失的地址打印出来并悬停
    int page_fault_vaddr = 0;
    asm("movl %%cr2, %0"
        : "=r"(page_fault_vaddr)); // cr2是存放造成page_fault的地址
    put_str("\npage fault addr is ");
    put_int(page_fault_vaddr);
  }
  put_str("\n!!!!!!!      excetion message end    !!!!!!!!\n");
  // 能进入中断处理程序就表示已经处在关中断情况下,
  // 不会出现调度进程的情况。故下面的死循环不会再被中断。
  while (1)
    ;
}

/* 完成一般中断处理函数注册及异常名称注册 */
static void exception_init(void) { // 完成一般中断处理函数注册及异常名称注册
  int i;
  for (i = 0; i < IDT_DESC_CNT; i++) {

    /* idt_table数组中的函数是在进入中断后根据中断向量号调用的,
     * 见kernel/kernel.S的call [idt_table + %1*4] */
    idt_table[i] =
        general_intr_handler; // 默认为general_intr_handler。
                              // 以后会由register_handler来注册具体处理函数。
    intr_name[i] = "unknown?"; // 先统一赋值为unknown
  }
  intr_name[0] = "#DE Divide Error";
  intr_name[1] = "#DB Debug Exception";
  intr_name[2] = "NMI Interrupt";
  intr_name[3] = "#BP Breakpoint Exception";
  intr_name[4] = "#OF Overflow Exception";
  intr_name[5] = "#BR BOUND Range Exceeded Exception";
  intr_name[6] = "#UD Invalid Opcode Exception";
  intr_name[7] = "#NM Device Not Available Exception";
  intr_name[8] = "#DF Double Fault Exception";
  intr_name[9] = "Coprocessor Segment Overrun";
  intr_name[10] = "#TS Invalid TSS Exception";
  intr_name[11] = "#NP Segment Not Present";
  intr_name[12] = "#SS Stack Fault Exception";
  intr_name[13] = "#GP General Protection Exception";
  intr_name[14] = "#PF Page-Fault Exception";
  // intr_name[15] 第15项是intel保留项，未使用
  intr_name[16] = "#MF x87 FPU Floating-Point Error";
  intr_name[17] = "#AC Alignment Check Exception";
  intr_name[18] = "#MC Machine-Check Exception";
  intr_name[19] = "#XF SIMD Floating-Point Exception";
}

/* 完成有关中断的所有初始化工作 */
void idt_init() {
  put_str("idt_init start\n");
  idt_desc_init();  // 初始化中断描述符表
  exception_init(); // 异常名初始化并注册通用异常处理函数
  pic_init();       // 初始化 8259A

  // 加载 idt
  uint64_t idt_operand =
      ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt
                            << 16)); // idt_operand 为 64 位, 低 48 位为 idt
                                     // 的大小减 1, 高 16 位为 idt 的起始地址
  asm volatile("lidt %0" : : "m"(idt_operand));
  put_str("idt_init done\n");
}

/* 开中断并返回之前的中断状态 */
enum intr_status intr_enable() {
  enum intr_status old_status;
  if (INTR_ON == intr_get_status()) {
    old_status = INTR_ON;
    return old_status;
  } else {
    old_status = INTR_OFF;
    asm volatile("sti"); // 开中断,sti 指令将IF位置1
    return old_status;
  }
}

/* 关中断并返回之前的中断状态 */
enum intr_status intr_disable() {
  enum intr_status old_status;
  if (INTR_ON == intr_get_status()) {
    old_status = INTR_ON;
    asm volatile("cli" : : : "memory"); // 关中断,cli 指令将IF位置0
    return old_status;
  } else {
    old_status = INTR_OFF;
    return old_status;
  }
}

/* 将中断状态设置为 status */
enum intr_status intr_set_status(enum intr_status status) {
  return status & INTR_ON ? intr_enable() : intr_disable();
}

/* 获取当前中断状态 */
enum intr_status intr_get_status() {
  uint32_t eflags = 0;
  GET_EFLAGS(eflags);
  return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

/* 在中断处理程序数组第vector_no个元素中注册安装中断处理程序function */
void register_handler(uint8_t vector_no, intr_handler function) {
/* idt_table数组中的函数是在进入中断后根据中断向量号调用的,
 * 见kernel/kernel.S的call [idt_table + %1*4] */
  idt_table[vector_no] = function;
}