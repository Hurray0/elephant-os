%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
; 输出背景色绿色，前景色红色，并且跳动的字符串"My OS Loader"
; 需要输出到第二行，第一行是BIOS的提示信息。当前一行长度为80，每个字符占2个字节，所以第二行的起始地址为0xA0
WIDTH equ 0xA0

mov byte [gs:0x00 + WIDTH], 'M'
mov byte [gs:0x01 + WIDTH], 0xa4
mov byte [gs:0x02 + WIDTH], 'y'
mov byte [gs:0x03 + WIDTH], 0xa4
mov byte [gs:0x04 + WIDTH], ' '
mov byte [gs:0x05 + WIDTH], 0xa4
mov byte [gs:0x06 + WIDTH], 'O'
mov byte [gs:0x07 + WIDTH], 0xa4
mov byte [gs:0x08 + WIDTH], 'S'
mov byte [gs:0x09 + WIDTH], 0xa4
mov byte [gs:0x0a + WIDTH], ' '
mov byte [gs:0x0b + WIDTH], 0xa4
mov byte [gs:0x0c + WIDTH], 'L'
mov byte [gs:0x0d + WIDTH], 0xa4
mov byte [gs:0x0e + WIDTH], 'o'
mov byte [gs:0x0f + WIDTH], 0xa4
mov byte [gs:0x10 + WIDTH], 'a'
mov byte [gs:0x11 + WIDTH], 0xa4
mov byte [gs:0x12 + WIDTH], 'd'
mov byte [gs:0x13 + WIDTH], 0xa4
mov byte [gs:0x14 + WIDTH], 'e'
mov byte [gs:0x15 + WIDTH], 0xa4
mov byte [gs:0x16 + WIDTH], 'r'
mov byte [gs:0x17 + WIDTH], 0xa4

jmp $                    ; 无限循环