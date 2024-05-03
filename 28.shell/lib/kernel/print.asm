; 内核打印功能实现
TI_GDT         equ 0
RPL0           equ 0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

section .data
put_int_buffer dd 0, 0

[bits 32]
section .text
; put_char，将栈中的一个字符写入光标所在处
global  put_char
global  put_str
global  put_int

; 打印32位无符号整数，16进制
put_int:
    pushad                     ; 保存寄存器
    mov    ebp, esp            ; ebp指向栈顶
    mov    eax, [ebp + 4 * 9]  ; 获取参数. 4 * 9是因为pushad压栈了8个寄存器，加上返回地址共9个
    mov    edx, eax            ; 使用edx做中转
    mov    edi, 7              ; edi指向put_int_buffer的最后一个位置
    mov    ecx, 8              ; 循环8次，每次处理4位
    mov    ebx, put_int_buffer ; ebx指向put_int_buffer

.16based_4bits:
    and edx, 0x0000000F ; 取低4位
    cmp edx, 9          ; 判断是否大于9
    jg  .is_A2F         ; 大于9，转换为A-F
    add edx, '0'        ; 小于9，转换为字符
    jmp .store

.is_A2F:
    sub edx, 10  ; 减去10
    add edx, 'A' ; 转换为A-F

.store:
    mov  [ebx + edi], dl  ; dl是低8位，存入put_int_buffer
    dec  edi
    shr  eax,         4
    mov  edx,         eax
    loop .16based_4bits

.ready_print:
    inc edi ; 此时edi为-1，需要加1, 指向第一个字符

.skip_prefix_0:
    cmp edi, 8 ; 如果已经到第9位，说明前面全是0，需要直接输出0
    je  .full0

.go_on_skip: ; 跳过前导0. eg: 0000000F -> F
    mov cl, [put_int_buffer + edi]
    inc edi                        ; edi + 1
    cmp cl, '0'
    je  .skip_prefix_0
    dec edi
    jmp .put_each_num

.full0:
    mov cl, '0'
.put_each_num: ; 逐个打印
    push ecx                         ; 函数参数压栈，也就是要打印的字符
    call put_char
    add  esp, 4
    inc  edi
    mov  cl,  [put_int_buffer + edi] ; 取下一个字符
    cmp  edi, 8                      ; 判断是否打印完毕, 8位数则打印完毕
    jl   .put_each_num
    popad
    ret

; 字符串打印函数，基于put_char封装
put_str:
    push ebx             ; 保存寄存器
    push ecx
    xor  ecx, ecx        ; ecx = 0
    mov  ebx, [esp + 12] ; 获取参数，字符串地址

.go_on:
    mov  cl,  [ebx]
    cmp  cl,  0     ; 判断是否结束\0
    jz   .str_over
    push ecx        ; 函数参数压栈
    call put_char
    add  esp, 4
    inc  ebx
    jmp  .go_on

.str_over:
    pop ecx
    pop ebx
    ret

put_char:
    pushad
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    ; 获取当前光标位置
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    in  al, dx
    mov ah, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in  al, dx

    mov bx,  ax         ; bx = 光标位置
    mov ecx, [esp + 36] ; 获取参数，字符

    cmp cl, 0xd             ; 回车键
    jz  .is_carriage_return
    cmp cl, 0xa             ; 换行键
    jz  .is_line_feed

    cmp cl, 0x8       ; 退格键
    jz  .is_backspace
    jmp .put_other    ; 其他字符

.is_backspace:
    dec bx    ; 光标左移
    shl bx, 1 ; 乘以2, 表示光标对应显存的偏移字节（一个字符占2个字节）

    mov byte [gs:bx], 0x20 ; 写入空格
    inc bx
    mov byte [gs:bx], 0x07 ; 颜色
    shr bx,           1    ; 除以2，恢复光标位置
    jmp .set_cursor

.put_other:
    shl bx,           1    ; 乘以2, 因为一个字符占2个字节
    mov [gs:bx],      cl   ; 写入字符
    inc bx
    mov byte [gs:bx], 0x07 ; 颜色
    shr bx,           1    ; 除以2
    inc bx
    cmp bx,           2000 ; 判断是否超出屏幕, 80 * 25 = 2000, 如果超出则滚屏（\r\n+滚屏）
    jl  .set_cursor

.is_line_feed:       ; 换行
.is_carriage_return: ; 回车, 只需要处理光标位置到行首
    xor dx, dx
    mov ax, bx
    mov si, 80

    div si

    sub bx, dx

.is_carriage_return_end: ; 回车键处理完毕，处理换行
    add bx, 80
    cmp bx, 2000 ; 判断是否超出屏幕
.is_line_feed_end:
    jl .set_cursor

.roll_screeen:
    cld          ; 清空方向
    mov ecx, 960 ; 搬运次数： 需要搬运的字符 2000 - 80 = 1920, 共3840字节, 每次搬运4字节，所以需要搬运960次

    mov       esi, 0xc00b80a0 ; 源地址
    mov       edi, 0xc00b8000 ; 目的地址
    rep movsd                 ; 搬运

    mov ebx, 3840 ; 最后一行首字符位置
    mov ecx, 80   ; 80个字符

.cls: ; 清空最后一行
    mov  word [gs:ebx], 0x0720 ; 空格
    add  ebx,           2
    loop .cls
    mov  bx,            1920   ; 光标位置

.set_cursor: ; 设置光标位置, bx = 光标位置
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al

.put_char_done:
    popad
    ret

global set_cursor
set_cursor:
    pushad
    mov bx, [esp+36]
    ;;;;;;; 1 先设置高8位 ;;;;;;;;
    mov dx, 0x03d4   ;索引寄存器
    mov al, 0x0e     ;用于提供光标位置的高8位
    out dx, al
    mov dx, 0x03d5   ;通过读写数据端口0x3d5来获得或设置光标位置
    mov al, bh
    out dx, al

    ;;;;;;; 2 再设置低8位 ;;;;;;;;;
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al
    popad
    ret

global cls_screen
cls_screen:
    pushad
    ;;;;;;;;;;;;;;;
        ; 由于用户程序的cpl为3,显存段的dpl为0,故用于显存段的选择子gs在低于自己特权的环境中为0,
        ; 导致用户程序再次进入中断后,gs为0,故直接在put_str中每次都为gs赋值.
    mov ax, SELECTOR_VIDEO ; 不能直接把立即数送入gs,须由ax中转
    mov gs, ax

    mov ebx, 0
    mov ecx, 80*25
    .cls:
    mov  word [gs:ebx], 0x0720 ;0x0720是黑底白字的空格键
    add  ebx,           2
    loop .cls
    mov  ebx,           0

    .set_cursor: ;直接把set_cursor搬过来用,省事
    ;;;;;;; 1 先设置高8位 ;;;;;;;;
    mov dx, 0x03d4 ;索引寄存器
    mov al, 0x0e   ;用于提供光标位置的高8位
    out dx, al
    mov dx, 0x03d5 ;通过读写数据端口0x3d5来获得或设置光标位置
    mov al, bh
    out dx, al

    ;;;;;;; 2 再设置低8位 ;;;;;;;;;
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al
    popad
    ret