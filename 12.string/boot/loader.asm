%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR

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
    loop .find_max_mem_area ; 循环遍历所有ards结构, 直到ecx=0
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
    or  al,   00000010b
    out 0x92, al

; step2. 加载 GDT
    cli
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
    ; addr=0xccc
    mov ax,  SELECTOR_DATA    ; 数据段选择子, 0x0010, 即0x0002 << 3
    mov ds,  ax
    mov es,  ax
    mov ss,  ax
    mov esp, LOADER_STACK_TOP ; 设置栈顶指针, 0x00000900
    mov ax,  SELECTOR_VIDEO   ; 显存段选择子, 0x0018, 即0x0003 << 3
    mov gs,  ax

    ; 加载kernel
    mov esi, KERNEL_START_SECTOR
    mov edi, KERNEL_BIN_BASE_ADDR
    mov cl, 200

    call rd_disk_m_32


    ; 创建页目录及页表并初始化页内存位图
    call setup_page

    ; 要将描述符表地址及偏移量写入内存gdt_ptr,一会用新地址重新加载
    sgdt [gdt_ptr]	      ; 存储到原来gdt所有的位置

    ; 将gdt描述符中视频段描述符中的段基址+0xc0000000
    mov ebx, [gdt_ptr + 2]
    or dword [ebx + 0x18 + 4], 0xc0000000      ; 视频段是第3个段描述符,每个描述符是8字节,故0x18。
                            ; 段描述符的高4字节的最高位是段基址的31~24位

    ; 将gdt的基址加上0xc0000000使其成为内核所在的高地址
    add dword [gdt_ptr + 2], 0xc0000000

    add esp, 0xc0000000        ; 将栈指针同样映射到内核地址

    ; 把页目录地址赋给cr3
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax

    ; 打开cr0的pg位(第31位)
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; 在开启分页后,用gdt新的地址重新加载
    lgdt [gdt_ptr]             ; 重新加载

    ; 打印 "Virtual Memory"
    WIDTH equ 160             ; 80*2
    mov byte [gs:0x00 + WIDTH], 'V'
    mov byte [gs:0x01 + WIDTH], 0xa4
    mov byte [gs:0x02 + WIDTH], 'i'
    mov byte [gs:0x03 + WIDTH], 0xa4
    mov byte [gs:0x04 + WIDTH], 'r'
    mov byte [gs:0x05 + WIDTH], 0xa4
    mov byte [gs:0x06 + WIDTH], 't'
    mov byte [gs:0x07 + WIDTH], 0xa4

    ; 进入内核
    jmp SELECTOR_CODE:enter_kernel

    enter_kernel:
        call kernel_init           ; 调用内核初始化函数
        mov esp, 0xc009f000           ; 设置内核栈指针
        jmp KERNEL_ENTRY_POINT       ; 用地址0x1500访问测试

    ; jmp $ ; 无限循环

kernel_init:
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    mov dx, [KERNEL_BIN_BASE_ADDR + 42]
    mov ebx, [KERNEL_BIN_BASE_ADDR + 28]

    add ebx, KERNEL_BIN_BASE_ADDR
    mov cx, [KERNEL_BIN_BASE_ADDR + 44]

.each_segment:
    cmp byte [ebx], PT_NULL
    je .PTNULL

    ; 准备mem_cpy参数
    push dword [ebx + 16]
    mov eax, [ebx + 4]
    add eax, KERNEL_BIN_BASE_ADDR
    push eax
    push dword [ebx + 8]

    call mem_cpy
    add esp, 12

.PTNULL:
    add ebx, edx
    loop .each_segment
    ret

mem_cpy:
    cld
    push ebp
    mov ebp, esp
    push ecx

    mov edi, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    rep movsb

    pop ecx
    pop ebp
    ret


; ------------------------------------------------------------
; 创建页目录及页表
; ------------------------------------------------------------
setup_page:
    ; 先把页目录表清零
    mov ecx, 4096
    mov esi, 0
.clear_page_dir:
    mov  byte [PAGE_DIR_TABLE_POS + esi], 0
    inc  esi
    loop .clear_page_dir

    ; 开始创建页目录项（PDE）
.create_pde: ; 创建Page Directory Entry
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x1000             ; 页表的起始地址
    mov ebx, eax                ; 此处为ebx赋值，是为.create_pte做准备，ebx为基址

    ; 下面将页目录项0和0xc00都存为第一个页表的地址，
    ; 一个页表可表示4MB内存,这样0xc03fffff以下的地址和0x003fffff以下的地址都指向相同的页表，
    ; 这是为将地址映射为内核地址做准备
    or eax, PG_US_U | PG_RW_W | PG_P      ; 页目录项的属性RW和P位为1,US为1,表示用户属性,所有特权级别都可以访问.
    mov [PAGE_DIR_TABLE_POS + 0x0], eax   ; 第一个页目录项，在页目录表中的第一个目录项写入第一个页表的位置(0x101000)及属性(7)
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax ; 一个页表项占用4字节,0xc00表示第768个页表占用的目录项,0xc00以上的目录项用于内核空间,
					                      ; 也就是页表的0xc0000000~0xffffffff共计1G属于内核,0x0~0xbfffffff共计3G属于用户进程.
    sub eax, 0x1000                        ; 页目录项的地址减去0x1000,即为页表的地址

    mov [PAGE_DIR_TABLE_POS + 4092], eax  ; 最后一个页目录项指向页表的地址

    ; 创建页表项（PTE）
    mov ecx, 256
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P   ; 页表项的属性RW和P位为1,US为1,表示用户属性,所有特权级别都可以访问.
.create_pte:
    mov [ebx + esi*4], edx              ; 此时的ebx已经在上面通过eax赋值为0x101000，所以ebx+esi*4即为页表项的地址
    add edx, 4096                       ; 页表项的地址加上4KB，即下一个页表项的地址
    inc esi
    loop .create_pte

    ; 创建内核其它页表的页表项(PDE)
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000                 ; 此时eax为第二个页表的地址
    or eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254
    mov esi, 769
.create_kernel_pde:
    mov [ebx + esi*4], eax
    inc esi
    add eax, 0x1000
    loop .create_kernel_pde
    ret

; 保护模式的硬盘读取函数
rd_disk_m_32:
    ; 1: 检查disk status
    mov dx,0x1f7     ; 0x1f7=primary channel's status
.not_ready1:
    nop              ; 只是为了增加延迟
    in al,dx
    and al,0xc0      ; 0xc0=1100_0000b取bit 6~7
    cmp al,0x40      ; 检查bit 6, 设备是否就绪
    jnz .not_ready1  ;若未准备好，继续等
    ; 2: 设置要读取的扇区数
    mov dx,0x1f2         ; 0x1f2=primary channel's sector count, 8 位寄存器，最大值为 255 ，若指定为 0，则表示要操作 256 个扇区, 见`硬盘控制器主要端口寄存器`
    mov al,cl
    out dx,al            ;读取的扇区数
    ; 3: 将LBA地址存入0x1f3 ~ 0x1f6
    mov eax,esi
    ;LBA地址7~0位写入端口0x1f3
    mov dx,0x1f3      ;   0x1f3=primary channel's lba low
    out dx,al

    ;LBA地址15~8位写入端口0x1f4
    shr eax,8         ;   eax值右移8位
    mov dx,0x1f4      ;   0x1f4=primary channel's lba mid
    out dx,al

    ;LBA地址23~16位写入端口0x1f5
    shr eax,8
    mov dx,0x1f5      ;   0x1f5=primary channel's lba high
    out dx,al

    ; 4: 设置device端口
    shr eax,8
    and al,0x0f      ; lba第24~27位, 其他bit置为0
    or al,0xe0       ; 设置7～4位为1110,表示lba模式, 并使用主盘
    mov dx,0x1f6     ; 0x1f6=primary channel's device
    out dx,al

    ; 5：向0x1f7端口写入读命令，0x20
    mov dx,0x1f7     ; 0x1f7=primary channel's status
    mov al,0x20      ; 0x20, 读取扇区
    out dx,al

    mov bl,cl

.next_sector:
    ; 6: 检查disk status
.not_ready2:
    mov dx,0x1f7
    in al,dx         ; 因为status 寄存器依然是 0x1f7 端口, 所以不需要再为dx 重新赋值
    and al,0x88      ;第4位为1表示硬盘控制器已准备好数据传输，第7位为1表示硬盘忙
    cmp al,0x08
    jnz .not_ready2       ;若未准备好，继续等

    ; 7：从0x1f0端口读数据. data 寄存器是 16 位，即每次 in 操作只读入 2 字节
    mov cx, 256       ; cx是操作次数. 一个扇区有512字节，每次读入2个字，共需512/2次=256
    mov dx, 0x1f0    ; 0x1f0=primary channel's data
.go_on_read:
    in ax,dx
    mov [edi],ax      ; 使用edi, 因为KERNEL_BIN_BASE_ADDR超过了di
    add edi,2
    loop .go_on_read ; loop会cx-=1, 并判断cx是否为0进而继续循环还是往下走
    dec bl
    cmp bl,0
    jnz .next_sector
    ret