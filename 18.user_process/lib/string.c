#include "string.h"
#include "debug.h"
// #include "global.h"

// 将dst_起始的size个字节置为value
void memset(void *dst_, uint8_t value, uint32_t size) {
  DEBUG_CMD
  ASSERT(dst_ != NULL);
  uint8_t *dst = (uint8_t *)dst_;
  while (size-- > 0) {
    *dst++ = value;
  }
}

// 将src_起始的size个字节复制到dst_起始地址
void memcpy(void *dst_, const void *src_, uint32_t size) {
  DEBUG_CMD
  ASSERT(dst_ != NULL && src_ != NULL);
  uint8_t *dst = (uint8_t *)dst_;
  const uint8_t *src = (uint8_t *)src_;
  while (size-- > 0) {
    *dst++ = *src++;
  }
}

// 比较两个内存区域的数据
int memcmp(const void *a_, const void *b_, uint32_t size) {
  DEBUG_CMD
  ASSERT(a_ != NULL && b_ != NULL);
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
  DEBUG_CMD
  ASSERT(dst_ != NULL && src_ != NULL);
  char *r = dst_;
  while ((*dst_++ = *src_++))
    ;
  return r;
}

// 返回字符串长度
uint32_t strlen(const char *str) {
  DEBUG_CMD
  ASSERT(str != NULL)
  uint32_t r = 0;
  while (*(str + (r++)))
    ;
  return --r;
}

// 比较两个字符串是否相等
int8_t strcmp(const char *a, const char *b) {
  DEBUG_CMD
  ASSERT(a != NULL && b != NULL);
  while (*a != 0 && *a == *b) {
    a++;
    b++;
  }
  return *a < *b ? -1 : *a > *b;
}

// 从左往右查找字符串str中首次出现字符ch的地址
char *strchr(const char *string, const uint8_t ch) {
  DEBUG_CMD
  ASSERT(string != NULL);
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
  DEBUG_CMD
  ASSERT(string != NULL);
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
  DEBUG_CMD
  ASSERT(dst_ != NULL && src_ != NULL);
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
  DEBUG_CMD
  ASSERT(filename != NULL);
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

// void test_string_functions(void) {
//   // Test memset
//   char str1[10];
//   memset(str1, 'A', 5);
//   str1[5] = '\0';
//   ASSERT(strcmp(str1, "AAAAA") == 0);
//   put_str("memset done\n");

//   // Test memcpy
//   char str2[10];
//   char src[] = "Hello";
//   memcpy(str2, src, 5);
//   str2[5] = '\0';
//   ASSERT(strcmp(str2, "Hello") == 0);
//   put_str("memcpy done\n");

//   // Test memcmp
//   char str3[] = "Hello";
//   char str4[] = "World";
//   int result = memcmp(str3, str4, 5);
//   ASSERT(result < 0);
//   put_str("memcmp done\n");

//   // Test strcpy
//   char str5[10];
//   char src2[] = "Hello";
//   strcpy(str5, src2);
//   ASSERT(strcmp(str5, "Hello") == 0);
//   put_str("strcpy done\n");

//   // Test strlen
//   char str6[] = "Hello";
//   uint32_t length = strlen(str6);
//   ASSERT(length == 5);
//   put_str("strlen done\n");

//   // Test strcmp
//   char str7[] = "Hello";
//   char str8[] = "World";
//   int8_t cmp_result = strcmp(str7, str8);
//   ASSERT(cmp_result < 0);
//   put_str("strcmp done\n");

//   // Test strchr
//   char str9[] = "Hello";
//   char ch = 'e';
//   char *ch_ptr = strchr(str9, ch);
//   ASSERT(ch_ptr != NULL && *ch_ptr == 'e');
//   put_str("strchr done\n");

//   // Test strrchr
//   char str10[] = "Hello";
//   char ch2 = 'l';
//   char *ch_ptr2 = strrchr(str10, ch2);
//   ASSERT(ch_ptr2 != NULL && *ch_ptr2 == 'l');
//   put_str("strrchr done\n");

//   // Test strcat
//   char str11[20] = "Hello";
//   char src3[] = "World";
//   strcat(str11, src3);
//   ASSERT(strcmp(str11, "HelloWorld") == 0);
//   put_str("strcat done\n");

//   // Test strchrs
//   char str12[] = "Hello";
//   char ch3 = 'l';
//   uint32_t ch_count = strchrs(str12, ch3);
//   ASSERT(ch_count == 2);
//   put_str("strchrs done\n");
// }