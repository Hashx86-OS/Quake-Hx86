#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

int strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, int n);
char *strcat(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, int n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, int n);

void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#endif
