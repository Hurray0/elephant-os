#include "debug.h"
#include "init.h"
#include "print.h"
#include "string.h"

static void test_string_functions(void); // 注意不能直接实现，否则main的地址就不是0xc0001500了


int main(void) {
  put_str("I am kernel\n");
  init_all();
  // 因为目前是单线程，打开之后就没法测试ASSERT了，会一直处理中断，所以这里注释掉
  //   asm volatile("sti");  // 为演示中断处理,在此临时开中断
  test_string_functions();
  while (1)
    ;
  return 0;
}

static void test_string_functions(void) {
  // Test memset
  char str1[10];
  memset(str1, 'A', 5);
  str1[5] = '\0';
  ASSERT(strcmp(str1, "AAAAA") == 0);
  put_str("memset done\n");

  // Test memcpy
  char str2[10];
  char src[] = "Hello";
  memcpy(str2, src, 5);
  str2[5] = '\0';
  ASSERT(strcmp(str2, "Hello") == 0);
  put_str("memcpy done\n");

  // Test memcmp
  char str3[] = "Hello";
  char str4[] = "World";
  int result = memcmp(str3, str4, 5);
  ASSERT(result < 0);
  put_str("memcmp done\n");

  // Test strcpy
  char str5[10];
  char src2[] = "Hello";
  strcpy(str5, src2);
  ASSERT(strcmp(str5, "Hello") == 0);
  put_str("strcpy done\n");

  // Test strlen
  char str6[] = "Hello";
  uint32_t length = strlen(str6);
  ASSERT(length == 5);
  put_str("strlen done\n");

  // Test strcmp
  char str7[] = "Hello";
  char str8[] = "World";
  int8_t cmp_result = strcmp(str7, str8);
  ASSERT(cmp_result < 0);
  put_str("strcmp done\n");

  // Test strchr
  char str9[] = "Hello";
  char ch = 'e';
  char *ch_ptr = strchr(str9, ch);
  ASSERT(ch_ptr != NULL && *ch_ptr == 'e');
  put_str("strchr done\n");

  // Test strrchr
  char str10[] = "Hello";
  char ch2 = 'l';
  char *ch_ptr2 = strrchr(str10, ch2);
  ASSERT(ch_ptr2 != NULL && *ch_ptr2 == 'l');
  put_str("strrchr done\n");

  // Test strcat
  char str11[20] = "Hello";
  char src3[] = "World";
  strcat(str11, src3);
  ASSERT(strcmp(str11, "HelloWorld") == 0);
  put_str("strcat done\n");

  // Test strchrs
  char str12[] = "Hello";
  char ch3 = 'l';
  uint32_t ch_count = strchrs(str12, ch3);
  ASSERT(ch_count == 2);
  put_str("strchrs done\n");
}