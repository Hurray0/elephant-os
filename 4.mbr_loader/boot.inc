; ------------------- loader和kernel的加载 -------------------
LOADER_BASE_ADDR equ 0x900   ; loader的基地址，本来应该从0x500开始，但是这里为了预留空间，所以从0x900开始
LOADER_START_SECTOR equ 0x2  ; 从第2个扇区开始读取loader，防止覆盖MBR(0号扇区)