#include <Hx86/Hx86.h>
#include <stdarg.h>
#include <stddef.h>

int vsprintf(char* buffer, const char* format, va_list args) {
    int idx = 0;
    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
                case 'd': case 'i': {
                    int num = va_arg(args, int);
                    char tmp[12];
                    int tIdx = 11;
                    tmp[tIdx] = '\0';
                    int neg = (num < 0) ? 1 : 0;
                    if (neg) num = -num;
                    if (num == 0) tmp[--tIdx] = '0';
                    else { while (num > 0) { tmp[--tIdx] = (num % 10) + '0'; num /= 10; } }
                    if (neg) tmp[--tIdx] = '-';
                    for (int k = tIdx; tmp[k]; k++) buffer[idx++] = tmp[k];
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char tmp[12];
                    int tIdx = 11;
                    tmp[tIdx] = '\0';
                    if (num == 0) tmp[--tIdx] = '0';
                    else { while (num > 0) { tmp[--tIdx] = (num % 10) + '0'; num /= 10; } }
                    for (int k = tIdx; tmp[k]; k++) buffer[idx++] = tmp[k];
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    for (int k = 0; s[k]; k++) buffer[idx++] = s[k];
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    buffer[idx++] = c;
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char tmp[9];
                    int tIdx = 8;
                    tmp[tIdx] = '\0';
                    const char* hex = "0123456789abcdef";
                    if (num == 0) tmp[--tIdx] = '0';
                    else { while (num > 0) { tmp[--tIdx] = hex[num % 16]; num /= 16; } }
                    for (int k = tIdx; tmp[k]; k++) buffer[idx++] = tmp[k];
                    break;
                }
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    char tmp[9];
                    int tIdx = 8;
                    tmp[tIdx] = '\0';
                    const char* hex = "0123456789ABCDEF";
                    if (num == 0) tmp[--tIdx] = '0';
                    else { while (num > 0) { tmp[--tIdx] = hex[num % 16]; num /= 16; } }
                    for (int k = tIdx; tmp[k]; k++) buffer[idx++] = tmp[k];
                    break;
                }
                case 'f': {
                    double num = va_arg(args, double);
                    char tmp[32];
                    int tIdx = 0;
                    int neg = 0;
                    if (num < 0) { neg = 1; num = -num; }
                    long intPart = (long)num;
                    double fracPart = num - (double)intPart;
                    if (fracPart < 0) fracPart = 0;
                    if (neg) tmp[tIdx++] = '-';
                    char ibuf[16];
                    int iIdx = 15;
                    ibuf[iIdx] = '\0';
                    if (intPart == 0) ibuf[--iIdx] = '0';
                    else { while (intPart > 0) { ibuf[--iIdx] = (intPart % 10) + '0'; intPart /= 10; } }
                    for (int k = iIdx; ibuf[k]; k++) tmp[tIdx++] = ibuf[k];
                    tmp[tIdx++] = '.';
                    for (int d = 0; d < 6; d++) {
                        fracPart *= 10;
                        int digit = (int)fracPart;
                        if (digit > 9) digit = 9;
                        tmp[tIdx++] = digit + '0';
                        fracPart -= digit;
                    }
                    tmp[tIdx] = '\0';
                    for (int k = 0; tmp[k]; k++) buffer[idx++] = tmp[k];
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    unsigned int addr = (unsigned int)ptr;
                    buffer[idx++] = '0';
                    buffer[idx++] = 'x';
                    char tmp[9];
                    int tIdx = 8;
                    tmp[tIdx] = '\0';
                    const char* hex = "0123456789abcdef";
                    if (addr == 0) tmp[--tIdx] = '0';
                    else { while (addr > 0) { tmp[--tIdx] = hex[addr % 16]; addr /= 16; } }
                    for (int k = tIdx; tmp[k]; k++) buffer[idx++] = tmp[k];
                    break;
                }
                case '%': buffer[idx++] = '%'; break;
                default: buffer[idx++] = '%'; buffer[idx++] = format[i]; break;
            }
        } else {
            buffer[idx++] = format[i];
        }
    }
    buffer[idx] = '\0';
    return idx;
}

int sprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(buffer, format, args);
    va_end(args);
    return ret;
}

int toupper(int c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

double fabs(double x) {
    return x < 0 ? -x : x;
}

double sqrt(double x) {
    double result;
    __asm__ volatile ("fsqrt" : "=t" (result) : "0" (x));
    return result;
}

double floor(double x) {
    double result;
    unsigned short oldcw, newcw;
    __asm__ volatile ("fstcw %0" : "=m" (oldcw));
    newcw = (oldcw & ~0x0C00) | 0x0400;  // Set rounding mode to round-down
    __asm__ volatile ("fldcw %0" : : "m" (newcw));
    __asm__ volatile ("frndint" : "=t" (result) : "0" (x));
    __asm__ volatile ("fldcw %0" : : "m" (oldcw));
    return result;
}

double ceil(double x) {
    double result;
    unsigned short oldcw, newcw;
    __asm__ volatile ("fstcw %0" : "=m" (oldcw));
    newcw = (oldcw & ~0x0C00) | 0x0800;  // Set rounding mode to round-up
    __asm__ volatile ("fldcw %0" : : "m" (newcw));
    __asm__ volatile ("frndint" : "=t" (result) : "0" (x));
    __asm__ volatile ("fldcw %0" : : "m" (oldcw));
    return result;
}

double sin(double x) {
    double result;
    __asm__ volatile ("fsin" : "=t" (result) : "0" (x));
    return result;
}

double cos(double x) {
    double result;
    __asm__ volatile ("fcos" : "=t" (result) : "0" (x));
    return result;
}

double tan(double x) {
    double result;
    __asm__ volatile (
        "fptan\n\t"
        "fstp %%st(0)"   // pop the 1.0 that fptan pushes, leaving tan(x) in st(0)
        : "=t" (result) : "0" (x)
    );
    return result;
}

double atan(double x) {
    double result;
    __asm__ volatile ("fld1; fpatan" : "=t" (result) : "0" (x) : "st(1)");
    return result;
}

double atan2(double y, double x) {
    double result;
    __asm__ volatile ("fpatan" : "=t" (result) : "0" (y), "u" (x) : "st(1)");
    return result;
}

double fmod(double x, double y) {
    if (y == 0.0) return 0.0;
    double result = x;
    while (result >= y) result -= y;
    while (result < 0) result += y;
    return result;
}

double exp(double x);
double log(double x);

double pow(double x, double y) {
    if (x <= 0) return 0;
    return exp(y * log(x));
}

double exp(double x) {
    double result;
    __asm__ volatile (
        "fldl2e\n\t"
        "fmulp\n\t"
        "fld %%st(0)\n\t"
        "frndint\n\t"
        "fxch %%st(1)\n\t"
        "fsub %%st(1), %%st(0)\n\t"
        "f2xm1\n\t"
        "fld1\n\t"
        "faddp\n\t"
        "fscale\n\t"
        "fstp %%st(1)"
        : "=t" (result) : "0" (x)
    );
    return result;
}

double log(double x) {
    double result;
    __asm__ volatile ("fldln2; fxch; fyl2x" : "=t" (result) : "0" (x) : "st(1)");
    return result;
}

int abs(int x) {
    return x < 0 ? -x : x;
}

long labs(long x) {
    return x < 0 ? -x : x;
}

double atof(const char* str) {
    double result = 0.0;
    double sign = 1.0;
    if (*str == '-') { sign = -1.0; str++; }
    else if (*str == '+') str++;
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }
    if (*str == '.') {
        str++;
        double frac = 1.0;
        while (*str >= '0' && *str <= '9') {
            frac /= 10.0;
            result += (*str - '0') * frac;
            str++;
        }
    }
    return result * sign;
}

int atoi(const char* str) {
    return (int)atof(str);
}

long atol(const char* str) {
    return (long)atof(str);
}

int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isalpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isxdigit(int c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == '\0') ? (char*)s : 0;
}

char* strrchr(const char* s, int c) {
    const char* found = 0;
    while (*s) {
        if (*s == (char)c) found = s;
        s++;
    }
    if (c == '\0') return (char*)s;
    return (char*)found;
}

int strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (toupper(*s1) != toupper(*s2)) return -1;
        s1++; s2++;
    }
    return (*s1) ? 1 : (*s2) ? -1 : 0;
}

int strncasecmp(const char* s1, const char* s2, int n) {
    while (n > 0 && *s1 && *s2) {
        if (toupper(*s1) != toupper(*s2)) return -1;
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return (*s1) ? 1 : (*s2) ? -1 : 0;
}

char* strncpy(char* dest, const char* src, int n) {
    int i;
    for (i = 0; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

void* malloc(size_t size) {
    return kmalloc((int)size);
}

void free(void* ptr) {
    kfree(ptr);
}

void* realloc(void* ptr, size_t size) {
    return krealloc(ptr, (int)size);
}

void* calloc(size_t nmemb, size_t size) {
    return kcalloc((int)nmemb, (int)size);
}

#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include "math.h"
#include <setjmp.h>

static unsigned int rand_next = 1;

int rand(void) {
    rand_next = rand_next * 1103515245 + 12345;
    return (unsigned int)(rand_next / 65536) % 32768;
}

void srand(unsigned int seed) {
    rand_next = seed;
}

int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
int strlen(const char *s);

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;
    while (*haystack) {
        const char *h = haystack, *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return NULL;
}

int strncmp(const char *s1, const char *s2, int n) {
    while (n > 0 && *s1 && *s2 && *s1 == *s2) { s1++; s2++; n--; }
    if (n == 0) return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int open(const char *pathname, int flags, ...) {
    return syscall_open(pathname, flags);
}

int close(int fd) {
    return syscall_close(fd);
}

int read(int fd, void *buf, int count) {
    return syscall_read(fd, (char*)buf, count);
}

int write(int fd, const void *buf, int count) {
    return syscall_write(fd, (const char*)buf, count);
}

int lseek(int fd, int offset, int whence) {
    int ret;
    __asm__ volatile ("int $0x80"
        : "=a" (ret)
        : "0" (19), "b" (fd), "c" (offset), "d" (whence)
        : "memory");
    return ret;
}

FILE *fopen(const char *path, const char *mode) {
    int flags = O_RDONLY;
    if (mode[0] == 'w') flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (mode[0] == 'a') flags = O_WRONLY | O_CREAT | O_APPEND;
    else if (mode[0] == 'r' && mode[1] && mode[1] == '+') flags = O_RDWR;
    if (mode[0] == 'w' || mode[0] == 'a') {
        if (mode[1] == 'b' || mode[1] == '+') {}
    }
    if (mode[0] == 'r' && mode[1] == 'b') {}
    int fd = syscall_open(path, flags);
    if (fd < 0) return NULL;
    FILE *f = (FILE*)kmalloc(sizeof(FILE));
    if (!f) { syscall_close(fd); return NULL; }
    f->fd = fd;
    f->eof = 0;
    f->error = 0;
    f->pos = 0;
    return f;
}

int fclose(FILE *f) {
    if (!f) return EOF;
    syscall_close(f->fd);
    kfree(f);
    return 0;
}

int fputc(int c, FILE *f) {
    if (!f) return EOF;
    char ch = (char)c;
    int ret = syscall_write(f->fd, &ch, 1);
    if (ret != 1) { f->error = 1; return EOF; }
    f->pos++;
    return (unsigned char)ch;
}

int fputs(const char *s, FILE *f) {
    if (!f) return EOF;
    int len = strlen(s);
    int ret = syscall_write(f->fd, s, len);
    if (ret != len) { f->error = 1; return EOF; }
    f->pos += ret;
    return ret;
}

int fprintf(FILE *f, const char *format, ...) {
    if (!f) return 0;
    va_list args;
    va_start(args, format);
    char buffer[2048];
    int len = vsprintf(buffer, format, args);
    va_end(args);
    int ret = syscall_write(f->fd, buffer, len);
    if (ret != len) f->error = 1;
    f->pos += ret;
    return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *f) {
    if (!f) return 0;
    int total = (int)(size * nmemb);
    int ret = syscall_read(f->fd, (char*)ptr, total);
    if (ret <= 0) { f->eof = 1; return 0; }
    f->pos += ret;
    return ret / (int)size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *f) {
    if (!f) return 0;
    int total = (int)(size * nmemb);
    int ret = syscall_write(f->fd, (const char*)ptr, total);
    if (ret != total) f->error = 1;
    f->pos += ret;
    return ret / (int)size;
}

char *fgets(char *s, int size, FILE *f) {
    if (!f || size <= 0) return NULL;
    int i = 0;
    while (i < size - 1) {
        char ch;
        int ret = syscall_read(f->fd, &ch, 1);
        if (ret <= 0) { f->eof = 1; break; }
        f->pos++;
        s[i++] = ch;
        if (ch == '\n') break;
    }
    if (i == 0) return NULL;
    s[i] = '\0';
    return s;
}

int fgetc(FILE *f) {
    if (!f) return EOF;
    unsigned char ch;
    int ret = syscall_read(f->fd, (char*)&ch, 1);
    if (ret <= 0) { f->eof = 1; return EOF; }
    f->pos++;
    return ch;
}

int ungetc(int c, FILE *f) {
    (void)c; (void)f;
    return EOF;
}

int feof(FILE *f) {
    return f ? f->eof : 1;
}

int ferror(FILE *f) {
    return f ? f->error : 1;
}

void clearerr(FILE *f) {
    if (f) { f->eof = 0; f->error = 0; }
}

int fflush(FILE *f) {
    (void)f;
    return 0;
}

int fseek(FILE *f, long offset, int whence) {
    if (!f) return -1;
    int ret = lseek(f->fd, (int)offset, whence);
    if (ret < 0) return -1;
    f->pos = ret;
    f->eof = 0;
    return 0;
}

long ftell(FILE *f) {
    if (!f) return -1;
    return (long)lseek(f->fd, 0, SEEK_CUR);
}

void rewind(FILE *f) {
    if (f) { fseek(f, 0, SEEK_SET); clearerr(f); }
}

int remove(const char *path) {
    (void)path;
    return 0;
}

int unlink(const char *pathname) {
    (void)pathname;
    return 0;
}

int rename(const char *old, const char *new_path) {
    (void)old; (void)new_path;
    return -1;
}

int sscanf(const char *str, const char *format, ...);

static int skip_whitespace_f(FILE *f) {
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (!isspace(c)) return ungetc(c, f);
    }
    return EOF;
}

static int read_int_f(FILE *f, int *val) {
    skip_whitespace_f(f);
    int c = fgetc(f);
    if (c == EOF) return 0;
    int neg = 0;
    if (c == '-') { neg = 1; c = fgetc(f); }
    else if (c == '+') c = fgetc(f);
    if (!isdigit(c)) return 0;
    int v = 0;
    while (isdigit(c)) { v = v * 10 + (c - '0'); c = fgetc(f); }
    if (c != EOF) ungetc(c, f);
    *val = neg ? -v : v;
    return 1;
}

static int read_float_f(FILE *f, float *val) {
    skip_whitespace_f(f);
    int c = fgetc(f);
    if (c == EOF) return 0;
    int neg = 0;
    if (c == '-') { neg = 1; c = fgetc(f); }
    else if (c == '+') c = fgetc(f);
    if (!isdigit(c) && c != '.') return 0;
    double v = 0.0;
    while (isdigit(c)) { v = v * 10.0 + (c - '0'); c = fgetc(f); }
    if (c == '.') {
        c = fgetc(f);
        double frac = 1.0;
        while (isdigit(c)) { frac /= 10.0; v += (c - '0') * frac; c = fgetc(f); }
    }
    if (c != EOF) ungetc(c, f);
    *val = (float)(neg ? -v : v);
    return 1;
}

static int read_string_f(FILE *f, char *s, int maxlen) {
    skip_whitespace_f(f);
    int i = 0;
    int c;
    while ((c = fgetc(f)) != EOF && !isspace(c) && i < maxlen - 1) {
        s[i++] = (char)c;
    }
    if (c != EOF) ungetc(c, f);
    s[i] = '\0';
    return (i > 0) ? 1 : 0;
}

int fscanf(FILE *f, const char *format, ...) {
    if (!f) return 0;
    va_list args;
    va_start(args, format);
    int count = 0;
    while (*format) {
        if (isspace(*format)) { format++; continue; }
        if (*format != '%') {
            int c = fgetc(f);
            if (c != *format) { if (c != EOF) ungetc(c, f); break; }
            format++;
            continue;
        }
        format++;
        int width = 0;
        while (isdigit(*format)) { width = width * 10 + (*format - '0'); format++; }
        switch (*format) {
            case 'd': case 'i': {
                int *val = va_arg(args, int*);
                if (read_int_f(f, val)) count++;
                else goto done;
                break;
            }
            case 'f': {
                float *val = va_arg(args, float*);
                if (read_float_f(f, val)) count++;
                else goto done;
                break;
            }
            case 's': {
                char *s = va_arg(args, char*);
                int maxlen = width > 0 ? width : 1024;
                if (read_string_f(f, s, maxlen)) count++;
                else goto done;
                break;
            }
            case 'c': {
                char *c = va_arg(args, char*);
                int ch = fgetc(f);
                if (ch != EOF) { *c = (char)ch; count++; }
                else goto done;
                break;
            }
            case 'n': {
                int *val = va_arg(args, int*);
                *val = f->pos;
                break;
            }
        }
        format++;
    }
done:
    va_end(args);
    return count;
}

static int sscanf_peek(const char **s) {
    if (**s == '\0') return EOF;
    return (unsigned char)**s;
}

static int sscanf_getc(const char **s) {
    if (**s == '\0') return EOF;
    return (unsigned char)*(*s)++;
}

static void sscanf_ungetc(const char **s) {
    if (*s > (const char*)0) (*s)--;
}

static int sscanf_skip_ws(const char **s) {
    int c;
    while ((c = sscanf_peek(s)) != EOF && isspace(c)) sscanf_getc(s);
    return c;
}

static int sscanf_read_int(const char **s, int *val) {
    sscanf_skip_ws(s);
    int c = sscanf_getc(s);
    if (c == EOF) return 0;
    int neg = 0;
    if (c == '-') { neg = 1; c = sscanf_getc(s); }
    else if (c == '+') c = sscanf_getc(s);
    if (!isdigit(c)) return 0;
    int v = 0;
    while (isdigit(c)) { v = v * 10 + (c - '0'); c = sscanf_getc(s); }
    if (c != EOF) sscanf_ungetc(s);
    *val = neg ? -v : v;
    return 1;
}

int sscanf(const char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);
    const char *s = str;
    int count = 0;
    while (*format) {
        if (isspace(*format)) { format++; continue; }
        if (*format != '%') {
            if (sscanf_getc(&s) != *format) break;
            format++;
            continue;
        }
        format++;
        switch (*format) {
            case 'd': case 'i': {
                int *val = va_arg(args, int*);
                if (sscanf_read_int(&s, val)) count++;
                else goto done;
                break;
            }
            case 's': {
                char *val = va_arg(args, char*);
                sscanf_skip_ws(&s);
                int i = 0;
                int c;
                while ((c = sscanf_peek(&s)) != EOF && !isspace(c) && i < 1023) {
                    val[i++] = (char)sscanf_getc(&s);
                }
                val[i] = '\0';
                if (i > 0) count++;
                else goto done;
                break;
            }
            case 'c': {
                char *val = va_arg(args, char*);
                int c = sscanf_getc(&s);
                if (c != EOF) { *val = (char)c; count++; }
                else goto done;
                break;
            }
            case 'f': {
                float *val = va_arg(args, float*);
                sscanf_skip_ws(&s);
                int c = sscanf_getc(&s);
                if (c == EOF) goto done;
                int neg = 0;
                if (c == '-') { neg = 1; c = sscanf_getc(&s); }
                else if (c == '+') c = sscanf_getc(&s);
                if (!isdigit(c) && c != '.') goto done;
                double v = 0.0;
                while (isdigit(c)) { v = v * 10.0 + (c - '0'); c = sscanf_getc(&s); }
                if (c == '.') {
                    c = sscanf_getc(&s);
                    double frac = 1.0;
                    while (isdigit(c)) { frac /= 10.0; v += (c - '0') * frac; c = sscanf_getc(&s); }
                }
                if (c != EOF) sscanf_ungetc(&s);
                *val = (float)(neg ? -v : v);
                count++;
                break;
            }
        }
        format++;
    }
done:
    va_end(args);
    return count;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) { s1++; s2++; }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int VCR_Init(void) {
    return -1;
}

typedef struct { void *c[6]; } _jb;
#define JB_EBX 0
#define JB_ESI 1
#define JB_EDI 2
#define JB_EBP 3
#define JB_EIP 4
#define JB_ESP 5

__attribute__((used)) int setjmp(jmp_buf env) {
    _jb *b = (_jb *)env;
    register int ebx asm("ebx");
    register int esi asm("esi");
    register int edi asm("edi");
    register int ebp asm("ebp");
    b->c[JB_EBX] = (void*)ebx;
    b->c[JB_ESI] = (void*)esi;
    b->c[JB_EDI] = (void*)edi;
    b->c[JB_EBP] = (void*)ebp;
    __asm__ volatile ("movl (%%esp), %0\n\t" : "=r"(b->c[JB_EIP]));
    __asm__ volatile ("movl %%esp, %0\n\t" : "=r"(b->c[JB_ESP]));
    return 0;
}

__attribute__((used)) void longjmp(jmp_buf env, int val) {
    _jb *b = (_jb *)env;
    __asm__ volatile (
        "movl %0, %%ebx\n\t"
        "movl %1, %%esi\n\t"
        "movl %2, %%edi\n\t"
        "movl %3, %%ebp\n\t"
        "movl %4, %%esp\n\t"
        :
        : "r"(b->c[JB_EBX]), "r"(b->c[JB_ESI]), "r"(b->c[JB_EDI]),
          "r"(b->c[JB_EBP]), "r"(b->c[JB_ESP])
        : "memory"
    );
    __asm__ volatile ("movl %0, %%eax\n\t" : : "r"(val ? val : 1));
    __asm__ volatile ("jmp *%0\n\t" : : "r"(b->c[JB_EIP]));
    __builtin_unreachable();
}

void exit(int status) {
    syscall_exit(status);
}

void perror(const char *s) {
    if (s) printf("%s: error\n", s);
    else printf("error\n");
}

char *strerror(int errnum) {
    (void)errnum;
    return "Unknown error";
}
