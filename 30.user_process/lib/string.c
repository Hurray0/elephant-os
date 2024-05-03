#include "string.h"
// #include "global.h"
#include "assert.h"

// 将dst_起始的size个字节置为value
void memset(void *dst_, uint8_t value, uint32_t size) {
  assert(dst_ != NULL);
  uint8_t *dst = (uint8_t *)dst_;
  while (size-- > 0) {
    *dst++ = value;
  }
}

// 将src_起始的size个字节复制到dst_起始地址
void memcpy(void *dst_, const void *src_, uint32_t size) {
  assert(dst_ != NULL && src_ != NULL);
  uint8_t *dst = (uint8_t *)dst_;
  const uint8_t *src = (uint8_t *)src_;
  while (size-- > 0) {
    *dst++ = *src++;
  }
}

// 比较两个内存区域的数据
int memcmp(const void *a_, const void *b_, uint32_t size) {
  assert(a_ != NULL && b_ != NULL);
  const char *a = (const char *)a_;
  const char *b = (const char *)b_;
  while (size-- > 0) {
    if (*a != *b) {
      return *a > *b ? 1 : -1;
    }
    ++a;
    ++b;
  }
  return 0;
}

// 将字符串src_拷贝到dst_中
char *strcpy(char *dst_, const char *src_) {
  assert(dst_ != NULL && src_ != NULL);
  char *r = dst_;
  while ((*dst_++ = *src_++))
    ;
  return r;
}

// 返回字符串长度
uint32_t strlen(const char *str) {
  assert(str != NULL);
  uint32_t r = 0;
  while (*(str + (r++)))
    ;
  return --r;
}

// 比较两个字符串是否相等
int8_t strcmp(const char *a, const char *b) {
  assert(a != NULL && b != NULL);
  while (*a != 0 && *a == *b) {
    a++;
    b++;
  }
  return *a < *b ? -1 : *a > *b;
}

// 从左往右查找字符串str中首次出现字符ch的地址
char *strchr(const char *string, const uint8_t ch) {
  assert(string != NULL);
  while (*string != 0) {
    if (*string == ch) {
      return (char *)string;
    }
    string++;
  }
  return NULL;
}

// 从右往左查找字符串str中首次出现字符ch的地址
char *strrchr(const char *string, const uint8_t ch) {
  assert(string != NULL);
  const char *last_char = NULL;
  while (*string != 0) {
    if (*string == ch) {
      last_char = string;
    }
    string++;
  }
  return (char *)last_char;
}

// 将字符串src_拼接到dst_后
char *strcat(char *dst_, const char *src_) {
  assert(dst_ != NULL && src_ != NULL);
  char *str = dst_;
  while (*str++)
    ;
  --str;
  while ((*str++ = *src_++))
    ;
  return dst_;
}

// 在字符串中查找字符ch出现的次数
uint32_t strchrs(const char *filename, uint8_t ch) {
  assert(filename != NULL);
  uint32_t ch_cnt = 0;
  const char *p = filename;
  while (*p != 0) {
    if (*p == ch) {
      ch_cnt++;
    }
    p++;
  }
  return ch_cnt;
}
