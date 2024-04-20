[bits 32]
%define ERROR_CODE nop    ; 若在相关的异常中 CPU 已经自动压入了错误码，为保持校中格式统一，这里不做操作

%define ZERO push 0

extern put_str            ; 声明外部函数
extern put_int            ; 声明外部函数

section .data
intr_str db "interrupt occur!", 0xa, 0           ; 这里0xa是换行符, 0是字符串结束符\0
; intr_cnt db 0                                    ; 中断计数器
global intr_entry_table

intr_entry_table:

%macro VECTOR 2   ; 这里的2是参数个数
section .text
intr%1entry:    ; 定义中断入口程序
    %2
    ; ;; 自定义的显示中断计数器的功能
    ; ; 中断计数器加1
    ; mov al, [intr_cnt]
    ; inc al
    ; mov [intr_cnt], al
    ; ; 显示中断计数器
    ; push eax
    ; call put_int
    ; pop eax
    ; ;; 自定义结束
    push intr_str
    call put_str

    add esp, 4    ; 跳过参数
    ; 如果是从片上进入的中断，除了往从片上发送 EOI 外，还要往主片上发送 EOI
    mov al, 0x20  ; 中断结束命令EOI
    out 0xa0, al  ; 发送给从片
    out 0x20, al  ; 发送给主片

    add esp, 4    ; 跳过错误码
    iret          ; 中断返回, 32位下等同于iretd，会自动弹出EIP、CS、EFLAGS、ESP、SS

section .data
    dd intr%1entry       ; 存储各个中断入口程序的地址，形成intr_entry_table数组，即中断向量表
%endmacro

; 从0x00到0x1f的中断向量表，共32个中断。其中0~19是CPU内部的异常，20~31是外部中断（Intel保留）
; 0x20为我们的时钟中断
VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO
VECTOR 0x20,ZERO