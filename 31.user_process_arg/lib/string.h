#ifndef __LIB_STRING_H
#define __LIB_STRING_H
#include "stdint.h"

// #define DEBUG
#ifdef DEBUG
// 这里的__func__代表函数名，是个const char *，需要转换成char *
#include "print.h"
#define DEBUG_CMD put_str("debug_str: ");put_str((char *)__func__);put_str("\n");
#else
#define DEBUG_CMD ;
#endif

void memset(void *dst_, uint8_t value, uint32_t size);
void memcpy(void *dst_, const void *src_, uint32_t size);
int memcmp(const void *a_, const void *b_, uint32_t size);
char *strcpy(char *dst_, const char *src_);
uint32_t strlen(const char *str);
int8_t strcmp(const char *a, const char *b);
char *strchr(const char *string, const uint8_t ch);
char *strrchr(const char *string, const uint8_t ch);
char *strcat(char *dst_, const char *src_);
uint32_t strchrs(const char *filename, uint8_t ch);

// void test_string_functions(void);

#endif
