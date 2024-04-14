; ------------------- loader和kernel的加载 -------------------
LOADER_BASE_ADDR equ 0x900   ; loader的基地址，本来应该从0x500开始，但是这里为了预留空间，所以从0x900开始
LOADER_START_SECTOR equ 0x2  ; 从第2个扇区开始读取loader，防止覆盖MBR(0号扇区)

; ------------------- gdt 描述符属性 -------------------
; 0~15: 段界限(低位)              # 20位地址线，最大寻址空间为4GB
; 16~31: 段基址(低位)          # 32位地址线，最大寻址空间为4GB
; 32~39: 段基址(中位)
; 40~43: Type                         # 包括读写权限等
; 44: S                               # 1表示代码或数据段，0表示系统段
; 45~46: DPL                          # 描述符特权级
; 47: P                              # 段存在位
; 48~51: 段界限(高位)              # 20位地址线，最大寻址空间为4GB
; 52: AVL                         # cpu不用此位，暂置为0
; 53: L                          # 64位代码标记，此处标记为0便可。
; 54: D/B                     # 默认操作数大小，0表示16位操作数，1表示32位操作数
; 55: G                    # 段粒度，0表示1B，1表示4KB
; 56~63: 段基址(高位)       # 32位地址线，最大寻址空间为4GB

DESC_G_4K   equ	  1_00000000000000000000000b    ;  4KB粒度标记，此处标记为1便可。
DESC_D_32   equ	   1_0000000000000000000000b    ;  32位代码标记，此处标记为1便可，如果是16位代码则为0。
DESC_L	    equ	    0_000000000000000000000b	;  64位代码标记，此处标记为0便可。
DESC_AVL    equ	     0_00000000000000000000b	;  cpu不用此位，暂置为0
DESC_LIMIT_CODE2 equ 1111_0000000000000000b     ;  段界限的高4位
DESC_LIMIT_DATA2 equ DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2 equ 0000_000000000000000b     ;  段界限的高4位
DESC_P    equ 1_000000000000000b                ;  段存在位
DESC_DPL_0 equ 00_0000000000000b                ;  DPL为Descriptor Privilege Level的缩写，即描述符特权级
DESC_DPL_1 equ 01_0000000000000b
DESC_DPL_2 equ 10_0000000000000b
DESC_DPL_3 equ 11_0000000000000b
DESC_S_CODE equ 1_000000000000b                 ;  系统段或数据段。1表示代码或数据段，0表示系统段
DESC_S_DATA equ DESC_S_CODE
DESC_S_sys equ 0_000000000000b
DESC_TYPE_CODE equ 1000_00000000b ; x=1 c=0 r=0 a=0 代码段是可执行的，非依从的，不可读的，已访问位a清0
DESC_TYPE_DATA equ 0010_00000000b ; x=0 e=0 w=1 a=0 数据段是不可执行的，向上扩展的，可写的，已访问位a清0
; DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
; DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
; DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b

; ------------------- 选择子属性 -------------------
; 选择子请求特权级, RPL0~RPL3
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b
; 选择子表指示, TI=0表示GDT, TI=1表示LDT
TI_GDT equ 000b
TI_LDT equ 100b
