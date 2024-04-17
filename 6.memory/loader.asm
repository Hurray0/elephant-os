%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
; jmp loader_start  ; 跳转逻辑放到mbr.asm里了，这里不需要了。为了对齐内存

; 构建 GDT(全局描述符表) 及其内部的描述符
GDT_BASE:        dd  0x00000000       ; dd 为 4 字节, 是double word的缩写
           dd 0x00000000
CODE_DESC: dd 0x0000FFFF ; 大小限制的低4字节为0xFFFF, 代码段的段基址为0x0000
           dd DESC_CODE_HIGH4
DATA_STACK_DESC: dd 0x0000FFFF
                  dd DESC_DATA_HIGH4
VIDEO_DESC: dd 0x80000007 ; limit=(0xbffff-0xb8000)/4k=0x7; 基址为0xb8000，所以这里取0x8000
            dd DESC_VIDEO_HIGH4 ; 此时 dpl 为 0

GDT_SIZE        equ $ - GDT_BASE
GDT_LIMIT       equ GDT_SIZE - 1
times 60        dq  0                             ; 保留60个描述符的空间. dq 为 8 字节, 是double quadword的缩写

; total_mem_bytes 用于保存内存容量，以字节为单位，此位置比较好记
; 当前偏移 loader.bin头文件0x200字节
; loader.bin 的加载地址为0x900
; 故total_mem_bytes的地址为0x900 + 0x200 = 0xb00
; 将来内核中咱们会引用此地址
total_mem_bytes dd  0

SELECTOR_CODE   equ (0x0001 << 3) + TI_GDT + RPL0 ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
SELECTOR_DATA   equ (0x0002 << 3) + TI_GDT + RPL0 ; 相当于(DATA_STACK_DESC - GDT_BASE)/8 + TI_GDT + RPL0
SELECTOR_VIDEO  equ (0x0003 << 3) + TI_GDT + RPL0 ; 相当于(VIDEO_DESC - GDT_BASE)/8 + TI_GDT + RPL0

; 以下是 gdt 的指针，前两个字节是 gdt 的大小，后四个字节是 gdt 的基地址
gdt_ptr         dw  GDT_LIMIT
        dd GDT_BASE

; 人工对齐: total_mem_bytes4+gdt_ptr6+ards_buf244+ards_nr2，共256字节
ards_buf times 244 db 0
ards_nr dw 0

; addr=0x900 + 0x200 + 0x100 = 0xc00
loader_start:
; ------------------------------------------------------------
; INT 15h eax = 0000E820h, edx = 534D4150h ('SMAP') 获取内存布局
; ------------------------------------------------------------
    xor ebx, ebx        ; ebx = 0, 第一次调用时，ebx必须为0
    mov edx, 0x534D4150 ; edx = 'SMAP'
    mov di,  ards_buf   ; ards结构缓冲区
.e820_mem_get_loop: ; 循环获取内存布局
    mov eax, 0x0000E820          ; BIOS功能号。执行int 0x15后，eax的值会变为0x534D4150，因此需要重新赋值
    mov ecx, 20                  ; ARDS地址范围描述符(ARDS)结构大小为20字节
    int 0x15                     ; BIOS调用
    jc  .e820_failed_so_try_e801 ; 若CF=1，则表示调用失败，尝试e801
    add di,  cx                  ; 更新ARDS缓冲区指针
    inc word [ards_nr]           ; 更新ARDS数量
    cmp ebx, 0                   ; 若ebx=0且cf不为1，则表示获取结束
    jnz .e820_mem_get_loop
    ; 在所有ards结构中，找出(base_add_low + length_low)的最大值，即内存容量
    mov cx,  [ards_nr]
    ; 遍历每一个ards结构体，循环次数是ards的数量
    mov ebx, ards_buf            ; ebx指向ards_buf
    xor edx, edx                 ; edx = 0
.find_max_mem_area: ; 无需判断 type 是否为1，最大的内存块一定是可被使用的
    mov eax, [ebx]     ; 获取 base_addr_low
    add eax, [ebx + 8] ; 加上 length_low
    add ebx, 20        ; 下一个 ards 结构
    cmp edx, eax       ; 冒泡排序，找出最大，edx寄存器始终是最大的内存块
    jge .next_ards     ; 若edx >= eax，则jmp到.next_ards
    mov edx, eax       ; edx为总内存大小
.next_ards:
    loop .find_max_mem_area  ; 循环遍历所有ards结构, 直到ecx=0
    jmp  .mem_get_ok

; ------------------------------------------------------------
; INT 15h eax = E801h 获取内存大小，最大支持4GB
; 返回后，ax cx 值一样，以KB为单位，bx dx值一样，以64KB为单位
; ------------------------------------------------------------
.e820_failed_so_try_e801:
    mov eax, 0x0000E801       ; BIOS功能号
    int 0x15                  ; BIOS调用
    jc  .e801_failed_so_try88 ; 若CF=1，则表示调用失败，尝试88
    ; 1. 先算出低15MB的内存. ax和cx的值是一样的，以KB为单位，将其转换为字节
    mov cx,  0x400
    mul cx                    ; ax = ax * cx
    shl edx, 16               ; edx = dx << 16
    and eax, 0x0000FFFF       ; ax = ax & 0x0000FFFF
    or  edx, eax              ; edx = edx | ax
    add edx, 0x100000         ; ax只是15MB，所以要加上1MB
    mov esi, edx              ; 先把低15MB的内存保存到esi中

    ; 2. 再算出15MB以上的内存
    xor eax, eax
    mov ax,  bx      ; ax = bx
    mov ecx, 0x10000 ; 64KB
    mul ecx          ; ax = ax * cx. 32位乘法，结果保存在edx:eax中
    add esi, eax     ; esi = esi + eax. 由于此方法只能获取到4GB的内存，所以32位eax足够
    mov edx, esi
    jmp .mem_get_ok

; ------------------------------------------------------------
; INT 15h eax = 0x88 获取内存大小，最大支持64MB
; ------------------------------------------------------------
.e801_failed_so_try88:
    ; int 15后，ax存入的是以KB为单位的内存大小
    mov ah,  0x88
    int 0x15
    jc  .error_hlt
    and eax, 0x0000FFFF

    ; 16位乘法，被乘数是ax, 积为32位，高16位在dx中，低16位在ax中
    mov cx,  0x400    ; 1KB = 0x400字节
    mul cx
    shl edx, 16       ; dx = dx << 16
    or  edx, eax      ; 把积的低16位组合刀edx中，为32位的积
    add edx, 0x100000 ; 1MB, 0x88子功能只会返回1MB以上的内存，所以这里加上1MB

.mem_get_ok:
    mov [total_mem_bytes], edx ; 保存内存容量

; ------------------------ 准备进入保护模式 ----------------------------
; 1. 打开 A20
; 2. 加载 GDT
; 3. 将 cr0 的 PE 位置 1

; step1. 打开 A20
    in  al,   0x92
    or  al,   0000_0010b
    out 0x92, al

; step2. 加载 GDT
    lgdt [gdt_ptr] ; 加载 GDT, 告诉 CPU GDT 的位置和大小。这个指令是写入 GDTR 寄存器

; step3. 将 cr0 的 PE 位置 1
    mov eax, cr0
    or  eax, 0x00000001
    mov cr0, eax

    jmp dword SELECTOR_CODE:p_mode_start ; 刷新流水线，进入保护模式
    ; 这里不省略代码或使用jmp p_mode_start而使用远转移：
    ; 是因为既要改变代码段描述符缓冲寄存器的值，又要刷新流水线（当前流水线上运行的16位代码）
    ; call/jmp/ret等可以改变cs值，可以刷新流水线
    ; 重新引用一个段，可以更新段缓冲寄存器的值

.error_hlt:
    hlt

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