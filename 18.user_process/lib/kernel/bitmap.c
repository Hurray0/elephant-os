#include "bitmap.h"
#include "string.h"
// #include "print.h"
// #include "interrupt.h"
#include "debug.h"

// 初始化位图
void bitmap_init(struct bitmap *btmp) {
  memset(btmp->bits, 0, btmp->btmp_bytes_len);
}

// 判断bit_idx位是否为1,若为1,返回true,否则返回false
bool bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx) {
  uint32_t byte_idx = bit_idx / 8; // 向下取整用于索引数组下标
  uint32_t bit_odd = bit_idx % 8;  // 取余用于索引数组内的位

  return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd));
}

// 在位图中申请连续cnt个位,成功则返回其起始位下标,失败返回-1
int bitmap_scan(struct bitmap *btmp, uint32_t cnt) {
  uint32_t idx_byte = 0;
  // 先逐字节比较
  while ((0xff == btmp->bits[idx_byte]) && (idx_byte < btmp->btmp_bytes_len)) {
    idx_byte++;
  }

  ASSERT(idx_byte < btmp->btmp_bytes_len);
  if (idx_byte == btmp->btmp_bytes_len) { // 说明位图中的位全部为1
    return -1;
  }

  // 再逐位比较
  int idx_bit = 0;
  // 和btmp->bits[idx_byte]这个字节逐位对比
  while ((uint8_t)(BITMAP_MASK << idx_bit) & btmp->bits[idx_byte]) {
    idx_bit++;
  }

  int bit_idx_start = idx_byte * 8 + idx_bit; // 计算空闲位的下标
  if (cnt == 1) {
    return bit_idx_start;
  }

  uint32_t bit_left = (btmp->btmp_bytes_len * 8 - bit_idx_start); // 记录还有多少位可以判断
  uint32_t bit_next = bit_idx_start + 1;
  uint32_t count = 1; // 用于记录找到的空闲位的个数

  bit_idx_start = -1; // 先将其置为-1,若找不到连续的位就直接返回
  while (bit_left-- > 0) {
    if (!(bitmap_scan_test(btmp, bit_next))) { // 若next位为0
      count++;
    } else {
      count = 0;
    }
    if (count == cnt) { // 若找到连续的cnt个空位
      bit_idx_start = bit_next - cnt + 1;
      break;
    }
    bit_next++;
  }
  return bit_idx_start;
}

// 将位图btmp的bit_idx位设置为value
void bitmap_set(struct bitmap *btmp, uint32_t bit_idx, int8_t value) {
  ASSERT((value == 0) || (value == 1));
  uint32_t byte_idx = bit_idx / 8;
  uint32_t bit_odd = bit_idx % 8;

  if (value) { // 如果value为1
    btmp->bits[byte_idx] |= (BITMAP_MASK << bit_odd);
  } else { // 若为0
    btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_odd);
  }
}
