#ifndef _STDIO_H_
#define _STDIO_H_

#include <stddef.h>
#include <stdarg.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define EOF (-1)

typedef struct {
    int fd;
    int eof;
    int error;
    int pos;
    char ungetbuf;
    int ungot;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

void printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int fprintf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);

FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
int fflush(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void clearerr(FILE *stream);

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

int fgetc(FILE *stream);
char *fgets(char *s, int size, FILE *stream);
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
int ungetc(int c, FILE *stream);
#define getc(stream) fgetc(stream)
#define putchar(c) fputc(c, stdout)

int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);

int remove(const char *pathname);
int rename(const char *oldpath, const char *newpath);

void perror(const char *s);

#endif
