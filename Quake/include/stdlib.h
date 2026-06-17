#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stddef.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define RAND_MAX 32767

int atoi(const char *nptr);
long atol(const char *nptr);
double atof(const char *nptr);

int rand(void);
void srand(unsigned int seed);

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *calloc(size_t nmemb, size_t size);

int abs(int j);
long labs(long j);

void exit(int status);

#endif
