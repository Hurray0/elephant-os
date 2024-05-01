%include "boot.inc"  ; 引入boot.inc文件. 里面定义了一些宏，主要有:

SECTION MBR vstart=0x7c00
; -------------------------------------------------------
; 设置段寄存器
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800  ; 显存段地址
    mov gs, ax      ; 设置gs寄存器, 用于访问显存

; -------------------------------------------------------
; 清屏，利用 0x06 号功能，上卷全部行，即可清屏
    mov ax, 0x600  ; 功能号 AH=0x06, AL = 上卷行数，0表示全部
    mov bx, 0x700  ; 上卷行属性
    mov cx, 0      ; 左上角(0,0)
    mov dx, 0x184f ; 右下角(80,25)

    int 0x10       ; 调用BIOS中断

; -------------------------------------------------------
; 获取光标位置
    mov ah, 0x03  ; 功能号 AH=0x03
    mov bh, 0     ; 页号
    int 0x10      ; 调用BIOS中断
; 输出: CH=光标开始行号, CL=光标开始列号, DH=光标所在行号, DL=光标所在列号

; ; -------------------------------------------------------
; ; 设置光标位置. 如果不设置，光标会在清屏前的位置
;     mov ah, 0x02  ; 功能号 AH=0x02
;     mov bh, 0     ; 页号
;     mov dh, 0     ; 行号
;     mov dl, 0     ; 列号
;     int 0x10      ; 调用BIOS中断

; -------------------------------------------------------
; 打印字符串: 直接使用起始地址0xB8000~0xBFFFF的显存
; 一个字符占2个字节，低字节为字符，高字节为属性。
; 属性: 高4位为背景色(K是否闪烁,R,G,B)，低4位为前景色(I亮度,R,G,B)


    ; 写字符串"Hello, MBR!"到显存。背景色绿色，前景色红色，闪烁
    mov byte [gs:0x00], 'H'
    mov byte [gs:0x01], 0xa4
    mov byte [gs:0x02], 'e'
    mov byte [gs:0x03], 0xa4
    mov byte [gs:0x04], 'l'
    mov byte [gs:0x05], 0xa4
    mov byte [gs:0x06], 'l'
    mov byte [gs:0x07], 0xa4
    mov byte [gs:0x08], 'o'
    mov byte [gs:0x09], 0xa4
    mov byte [gs:0x0a], ','
    mov byte [gs:0x0b], 0xa4
    mov byte [gs:0x0c], ' '
    mov byte [gs:0x0d], 0xa4
    mov byte [gs:0x0e], 'M'
    mov byte [gs:0x0f], 0xa4
    mov byte [gs:0x10], 'B'
    mov byte [gs:0x11], 0xa4
    mov byte [gs:0x12], 'R'
    mov byte [gs:0x13], 0xa4
    mov byte [gs:0x14], '!'
    mov byte [gs:0x15], 0xa4

    mov esi,LOADER_START_SECTOR   ; 起始扇区lba地址
    mov di,LOADER_BASE_ADDR            ; 写入的地址
    mov cl,4                      ; 待读入的扇区数
    call rd_disk_m_16

    jmp LOADER_BASE_ADDR + 0x300           ; 跳转到LOADER_BASE_ADDR处执行

; -------------------------------------------------------
; 读取磁盘n个扇区

rd_disk_m_16:
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
    mov [di],ax      ; di初始值是DISK_BUFFER
    add di,2
    loop .go_on_read ; loop会cx-=1, 并判断cx是否为0进而继续循环还是往下走
    dec bl
    cmp bl,0
    jnz .next_sector
    ret

    times 510-($-$$) db 0
    db 0x55,0xaa