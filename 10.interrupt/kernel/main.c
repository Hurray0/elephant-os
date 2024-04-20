#include "print.h"
#include "init.h"

void main(void) {
   put_str("I am kernel\n");
   init_all();
   asm volatile("sti"); // 使能中断
   while(1);
}