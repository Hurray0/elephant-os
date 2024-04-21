SECTION MBR vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax

; -------------------------------------------------------
; 清屏，利用 0x06 号功能，上卷全部行，即可清屏
    mov ax, 0x600  ; 功能号 AH=0x06, AL = 上卷行数，0表示全部
    mov bx, 0x700  ; 上卷行属性
    mov cx, 0      ; 左上角(0,0)
    mov dx, 0x184f ; 右下角(80,25)

    int 0x10       ; 调用BIOS中断
; -------------------------------------------------------

    mov byte [gs:0x00], 'H'
    mov byte [gs:0x01], 0xA4

    mov byte [gs:0x02], 'e'
    mov byte [gs:0x03], 0xA4

    mov byte [gs:0x04], 'l'
    mov byte [gs:0x05], 0xA4

    mov byte [gs:0x06], 'l'
    mov byte [gs:0x07], 0xA4

    mov byte [gs:0x08], 'o'
    mov byte [gs:0x09], 0xA4

    mov byte [gs:0x0A], ','
    mov byte [gs:0x0B], 0xA4

    mov byte [gs:0x0C], 'H'
    mov byte [gs:0x0D], 0xA4

    mov byte [gs:0x0E], 'u'
    mov byte [gs:0x0F], 0xA4

    mov byte [gs:0x10], 'r'
    mov byte [gs:0x11], 0xA4

    mov byte [gs:0x12], 'r'
    mov byte [gs:0x13], 0xA4

    mov byte [gs:0x14], 'a'
    mov byte [gs:0x15], 0xA4

    mov byte [gs:0x16], 'y'
    mov byte [gs:0x17], 0xA4

    mov byte [gs:0x18], '!'
    mov byte [gs:0x19], 0xA4

    jmp $

    times 510-($-$$) db 0

    db 0x55, 0xaa

