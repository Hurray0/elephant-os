%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start
; 下面使用nop对齐，是为了让代码在内存中的地址是16字节对齐的，便于查看二进制文件
nop
nop
nop
nop
nop

; 构建 GDT(全局描述符表) 及其内部的描述符
GDT_BASE: dd 0x00000000 ; dd 为 4 字节, 是double word的缩写
           dd 0x00000000
CODE_DESC: dd 0x0000FFFF ; 大小限制的低4字节为0xFFFF, 代码段的段基址为0x0000
           dd DESC_CODE_HIGH4
DATA_STACK_DESC: dd 0x0000FFFF
                  dd DESC_DATA_HIGH4
VIDEO_DESC: dd 0x80000007 ; limit=(0xbffff-0xb8000)/4k=0x7; 基址为0xb8000，所以这里取0x8000
            dd DESC_VIDEO_HIGH4 ; 此时 dpl 为 0

GDT_SIZE       equ $ - GDT_BASE
GDT_LIMIT      equ GDT_SIZE - 1
times 60       dq  0                             ; 保留60个描述符的空间. dq 为 8 字节, 是double quadword的缩写

SELECTOR_CODE  equ (0x0001 << 3) + TI_GDT + RPL0 ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
SELECTOR_DATA  equ (0x0002 << 3) + TI_GDT + RPL0 ; 相当于(DATA_STACK_DESC - GDT_BASE)/8 + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0 ; 相当于(VIDEO_DESC - GDT_BASE)/8 + TI_GDT + RPL0

; 以下是 gdt 的指针，前两个字节是 gdt 的大小，后四个字节是 gdt 的基地址
gdt_ptr        dw  GDT_LIMIT
        dd GDT_BASE
loadermsg db '2 loader in real.'

loader_start:
; ------------------------------------------------------------
; INT 0x10  功能号 0x13, 功能：显示字符串
; ------------------------------------------------------------
    mov sp, LOADER_BASE_ADDR
    mov bp, loadermsg        ; es:bp = 字符串地址. bp是基址寄存器
    mov cx, 17               ; cx = 字符串长度，用cx是循环计数器
    mov ax, 0x1301           ; 功能号 0x13, 功能：显示字符串. ah = 0x13, al = 0x01
    mov bx, 0x001f           ; 页号为0, 属性为蓝底粉红字(BL=1fh)
    mov dx, 0x1800           ; 行号为24, 列号为0
    int 0x10                 ; 10h 号中断，功能号 0x13

; ------------------------ 准备进入保护模式 ----------------------------
; 1. 打开 A20
; 2. 加载 GDT
; 3. 将 cr0 的 PE 位置 1

; step1. 打开 A20
    in  al,   0x92
    or  al,   0000_0010b
    out 0x92, al

; step2. 加载 GDT
    lgdt [gdt_ptr]  ; 加载 GDT, 告诉 CPU GDT 的位置和大小。这个指令是写入 GDTR 寄存器

; step3. 将 cr0 的 PE 位置 1
    mov eax, cr0
    or  eax, 0x00000001
    mov cr0, eax

    jmp dword SELECTOR_CODE:p_mode_start ; 刷新流水线，进入保护模式

[bits 32]
p_mode_start:
    mov ax,  SELECTOR_DATA    ; 数据段选择子, 0x0010, 即0x0002 << 3
    mov ds,  ax
    mov es,  ax
    mov ss,  ax
    mov esp, LOADER_STACK_TOP ; 设置栈顶指针, 0x00000900
    mov ax,  SELECTOR_VIDEO   ; 显存段选择子, 0x0018, 即0x0003 << 3
    mov gs,  ax

    WIDTH equ 0xA0

    ; 打印 "Protect Mode Loader"
    mov byte [gs:0x00 + WIDTH], 'P'
    mov byte [gs:0x01 + WIDTH], 0xa4
    mov byte [gs:0x02 + WIDTH], 'r'
    mov byte [gs:0x03 + WIDTH], 0xa4
    mov byte [gs:0x04 + WIDTH], 'o'
    mov byte [gs:0x05 + WIDTH], 0xa4
    mov byte [gs:0x06 + WIDTH], 't'
    mov byte [gs:0x07 + WIDTH], 0xa4
    mov byte [gs:0x08 + WIDTH], 'e'
    mov byte [gs:0x09 + WIDTH], 0xa4
    mov byte [gs:0x0a + WIDTH], 'c'
    mov byte [gs:0x0b + WIDTH], 0xa4
    mov byte [gs:0x0c + WIDTH], 't'
    mov byte [gs:0x0d + WIDTH], 0xa4
    mov byte [gs:0x0e + WIDTH], ' '
    mov byte [gs:0x0f + WIDTH], 0xa4
    mov byte [gs:0x10 + WIDTH], 'M'
    mov byte [gs:0x11 + WIDTH], 0xa4
    mov byte [gs:0x12 + WIDTH], 'o'
    mov byte [gs:0x13 + WIDTH], 0xa4
    mov byte [gs:0x14 + WIDTH], 'd'
    mov byte [gs:0x15 + WIDTH], 0xa4
    mov byte [gs:0x16 + WIDTH], 'e'
    mov byte [gs:0x17 + WIDTH], 0xa4
    mov byte [gs:0x18 + WIDTH], ' '
    mov byte [gs:0x19 + WIDTH], 0xa4
    mov byte [gs:0x1a + WIDTH], 'L'
    mov byte [gs:0x1b + WIDTH], 0xa4
    mov byte [gs:0x1c + WIDTH], 'o'
    mov byte [gs:0x1d + WIDTH], 0xa4
    mov byte [gs:0x1e + WIDTH], 'a'
    mov byte [gs:0x1f + WIDTH], 0xa4
    mov byte [gs:0x20 + WIDTH], 'd'
    mov byte [gs:0x21 + WIDTH], 0xa4
    mov byte [gs:0x22 + WIDTH], 'e'
    mov byte [gs:0x23 + WIDTH], 0xa4
    mov byte [gs:0x24 + WIDTH], 'r'
    mov byte [gs:0x25 + WIDTH], 0xa4

    jmp $ ; 无限循环