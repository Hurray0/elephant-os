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

    mov eax, LOADER_START_SECTOR   ; 起始扇区的LBA地址
    mov bx, LOADER_BASE_ADDR       ; 写入的地址
    mov cx, 4                      ; 读取的扇区数。【这里和之前的不一样，改大了一些】
    call rd_disk_m_16              ; 读取磁盘的起始部分，16位模式

    jmp LOADER_BASE_ADDR           ; 跳转到LOADER_BASE_ADDR处执行

; -------------------------------------------------------
; 读取磁盘n个扇区
rd_disk_m_16:
    ; eax: LBA扇区号，bx: 写入的地址，cx: 读取的扇区数
    mov esi, eax    ; 备份eax
    mov di, cx      ; 备份cx

    ; 读写硬盘:
    ; step1: 设置读取的扇区数
    mov dx, 0x1f2  ; 硬盘控制器寄存器(Sector Count Register)
    mov al, cl
    out dx, al     ; 设置读取的扇区数
    mov eax, esi   ; 恢复eax

    ; step2: 将LBA地址写入0x1f3~0x1f6
    ; LBA地址7~0位写入0x1f3
    mov dx, 0x1f3  ; 硬盘控制器寄存器(LBA Low Register)
    out dx, al     ; 写入LBA地址7~0位

    ; LBA地址15~8位写入0x1f4
    mov dx, 0x1f4  ; 硬盘控制器寄存器(LBA Mid Register)
    mov cl, 8      ; 右移8位
    shr eax, cl    ; eax右移8位
    out dx, al     ; 写入LBA地址15~8位

    ; LBA地址23~16位写入0x1f5
    mov dx, 0x1f5  ; 硬盘控制器寄存器(LBA High Register)
    shr eax, cl    ; eax右移8位
    out dx, al     ; 写入LBA地址23~16位

    ; LBA地址27~24位写入0x1f6
    mov dx, 0x1f6  ; 硬盘控制器寄存器(Device/Head Register)
    shr eax, cl    ; eax右移8位
    and al, 0x0f   ; 只取低4位
    or al, 0xe0    ; 设置7~4位为1110，表示LBA模式
    out dx, al     ; 写入LBA地址

    ; step3: 发出读取命令。向0x1f7写入0x20表示读取
    mov dx, 0x1f7  ; 硬盘控制器寄存器(Command Register)
    mov al, 0x20   ; 读取命令
    out dx, al     ; 发出读取命令

    ; step4: 等待硬盘准备好
    .not_ready:
        ; 同一端口，写时表示写入命令字，读时表示读入硬盘状态
        nop        ; delay
        in al, dx  ; 读取硬盘状态
        and al, 0x88 ; 只取BSY和DRQ位. 第4位为1表示硬盘已准备好数据传输，第7位为1表示硬盘忙
        cmp al, 0x08 ; 检查DRQ位是否为1
        jne .not_ready ; 如果DRQ位不为1，继续等待

    ; step5: 读取数据。从0x1f0读取数据
    mov ax, di     ; 读取的扇区数
    mov dx, 256    ; 一个扇区有256个字节
    mul dx         ; 计算读取的字节数
    mov cx, ax     ; 读取的字节数，存入cx，用于循环次数

    mov dx, 0x1f0  ; 硬盘数据寄存器
    .go_on_read:
        in ax, dx    ; 读取数据
        mov [bx], ax ; 写入内存
        add bx, 2    ; 每次读取2个字节
        loop .go_on_read ; 循环读取
        ret

; -------------------------------------------------------

    ; jmp $      ; 无限循环。这里不会执行到，因为跳转到LOADER_BASE_ADDR处执行

    times 510-($-$$) db 0

    db 0x55, 0xaa

