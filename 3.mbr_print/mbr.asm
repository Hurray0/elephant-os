SECTION MBR vstart=0x7c00
; -------------------------------------------------------
; 设置段寄存器
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00

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
    mov ax, 0xb800
    mov gs, ax

    ; 写字符串"Hello, Hurray!"到显存。背景色绿色，前景色红色，闪烁
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
    mov byte [gs:0x0e], 'H'
    mov byte [gs:0x0f], 0xa4
    mov byte [gs:0x10], 'u'
    mov byte [gs:0x11], 0xa4
    mov byte [gs:0x12], 'r'
    mov byte [gs:0x13], 0xa4
    mov byte [gs:0x14], 'r'
    mov byte [gs:0x15], 0xa4
    mov byte [gs:0x16], 'a'
    mov byte [gs:0x17], 0xa4
    mov byte [gs:0x18], 'y'
    mov byte [gs:0x19], 0xa4
    mov byte [gs:0x1a], '!'
    mov byte [gs:0x1b], 0xa4

; -------------------------------------------------------

    jmp $      ; 无限循环

    times 510-($-$$) db 0

    db 0x55, 0xaa

