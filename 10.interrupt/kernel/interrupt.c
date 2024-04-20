#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "print.h"
#include "stdint.h"

#define IDT_DESC_CNT 0x21 // 目前只定义了 0x21 个中断描述符
#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

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

extern intr_handler intr_entry_table[IDT_DESC_CNT]; // 声明引用定义在 kernel.asm
                                                    // 中的中断处理函数入口数组

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

  /* 打开主片上 IR0, 即目前只接受时钟产生的中断 */
  outb(PIC_M_DATA, 0xfe);
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

/* 完成有关中断的所有初始化工作 */
void idt_init() {
  put_str("idt_init start\n");
  idt_desc_init(); // 初始化中断描述符表
  // exception_init(); // 异常名初始化并注册通用异常处理函数
  pic_init(); // 初始化 8259A

  // 加载 idt
  uint64_t idt_operand =
      ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt
                            << 16)); // idt_operand 为 64 位, 低 48 位为 idt
                                     // 的大小减 1, 高 16 位为 idt 的起始地址
  asm volatile("lidt %0" : : "m"(idt_operand));
  put_str("idt_init done\n");
}