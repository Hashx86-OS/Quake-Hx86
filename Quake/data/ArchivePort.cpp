/*
 * MIT License
 *
 * Copyright (c) 2025 Malaka Gunawardana
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// C-linkage platform glue for miniz and lhasa. Quake's compat.c provides
// C++-mangled libc replacements for the engine; the vendored C archive code
// needs unmangled symbols, so this file supplies those wrappers.

#include <Hx86/Hsyscalls/syscalls.h>

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>

#include "ctype.h"
#include "errno.h"
#include "time.h"

// Declared directly instead of including <Hx86/memory.h>: that header pulls in
// the C++ printf declaration, which conflicts with the C declaration in the
// Quake stdio header that lha_arch.h includes.
void* kmalloc(int size);
void* kcalloc(int count, int size);
void* krealloc(void* pointer, int size);
void kfree(void* pointer);

extern "C" {
#include "lha_arch.h"
}

extern "C" {

FILE* stderr = nullptr;
int errno = 0;
void free(void* pointer);

void* malloc(size_t size) {
    if (size == 0 || size > 0x7FFFFFFFu) return nullptr;
    return kmalloc((int)size);
}

void* calloc(size_t count, size_t size) {
    if (count == 0 || size == 0) return nullptr;
    if (count > 0x7FFFFFFFu / size) return nullptr;
    return kcalloc((int)count, (int)size);
}

void* realloc(void* pointer, size_t size) {
    if (size > 0x7FFFFFFFu) return nullptr;
    if (pointer == nullptr) return malloc(size);
    if (size == 0) {
        free(pointer);
        return nullptr;
    }
    return krealloc(pointer, (int)size);
}

void free(void* pointer) {
    if (pointer != nullptr) kfree(pointer);
}

int isspace(int character) {
    return character == ' ' || character == '\t' || character == '\n' ||
           character == '\r' || character == '\f' || character == '\v';
}

int isdigit(int character) { return character >= '0' && character <= '9'; }

int isalpha(int character) {
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

int isalnum(int character) { return isalpha(character) || isdigit(character); }

int isxdigit(int character) {
    return isdigit(character) || (character >= 'a' && character <= 'f') ||
           (character >= 'A' && character <= 'F');
}

int islower(int character) { return character >= 'a' && character <= 'z'; }

int isupper(int character) { return character >= 'A' && character <= 'Z'; }

int tolower(int character) {
    return (character >= 'A' && character <= 'Z') ? character - 'A' + 'a' : character;
}

int toupper(int character) {
    return (character >= 'a' && character <= 'z') ? character - 'a' + 'A' : character;
}

time_t mktime(struct tm* time) {
    if (time == nullptr) return 0;
    int year = time->tm_year + 1900;
    int month = time->tm_mon + 1;
    int day = time->tm_mday;
    if (month < 1 || month > 12 || day < 1 || day > 31) return 0;

    static const int kMonthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int64_t days = 0;
    for (int y = 1970; y < year; y++) {
        bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        days += leap ? 366 : 365;
    }
    for (int m = 1; m < month; m++) {
        days += kMonthDays[m - 1];
        if (m == 2) {
            bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            if (leap) days++;
        }
    }
    days += day - 1;
    int64_t seconds = days * 86400 + time->tm_hour * 3600 + time->tm_min * 60 + time->tm_sec;
    return (time_t)seconds;
}

FILE* fopen(const char* path, const char* mode) {
    if (path == nullptr || mode == nullptr) return nullptr;

    int flags = O_RDONLY;
    if (mode[0] == 'w') {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (mode[0] == 'a') {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    } else if (mode[0] == 'r' && mode[1] == '+') {
        flags = O_RDWR;
    }

    int32_t fd = syscall_open(path, flags);
    if (fd < 0) return nullptr;

    FILE* file = (FILE*)kmalloc(sizeof(FILE));
    if (file == nullptr) {
        syscall_close((uint32_t)fd);
        return nullptr;
    }
    file->fd = fd;
    file->eof = 0;
    file->error = 0;
    file->pos = 0;
    file->ungetbuf = 0;
    file->ungot = 0;
    return file;
}

int fclose(FILE* stream) {
    if (stream == nullptr) return -1;
    syscall_close((uint32_t)stream->fd);
    kfree(stream);
    return 0;
}

size_t fread(void* buffer, size_t size, size_t count, FILE* stream) {
    if (stream == nullptr || size == 0 || count == 0) return 0;
    if (count > 0x7FFFFFFFu / size) return 0;
    int32_t wanted = (int32_t)(size * count);
    int32_t got = syscall_read((uint32_t)stream->fd, (char*)buffer, (uint32_t)wanted);
    if (got <= 0) {
        stream->eof = 1;
        return 0;
    }
    stream->pos += got;
    if (got < wanted) stream->eof = 1;
    return (size_t)got / size;
}

size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream) {
    if (stream == nullptr || size == 0 || count == 0) return 0;
    if (count > 0x7FFFFFFFu / size) return 0;
    int32_t wanted = (int32_t)(size * count);
    int32_t wrote = syscall_write((uint32_t)stream->fd, (const char*)buffer, (uint32_t)wanted);
    if (wrote < 0) {
        stream->error = 1;
        return 0;
    }
    stream->pos += wrote;
    if (wrote != wanted) stream->error = 1;
    return (size_t)wrote / size;
}

int fseek(FILE* stream, long offset, int origin) {
    if (stream == nullptr) return -1;
    int32_t result = syscall_lseek((uint32_t)stream->fd, (int32_t)offset, origin);
    if (result < 0) return -1;
    stream->pos = result;
    stream->eof = 0;
    return 0;
}

long ftell(FILE* stream) {
    if (stream == nullptr) return -1;
    int32_t result = syscall_lseek((uint32_t)stream->fd, 0, SEEK_CUR);
    return result < 0 ? -1 : (long)result;
}

int feof(FILE* stream) { return stream != nullptr ? stream->eof : 1; }

int lha_arch_vasprintf(char** result, char* format, va_list args) {
    (void)format;
    (void)args;
    if (result != nullptr) *result = nullptr;
    return -1;
}

void lha_arch_set_binary(FILE* handle) { (void)handle; }

int lha_arch_mkdir(char* path, unsigned int unix_perms) {
    return syscall_mkdir(path, (int)unix_perms) == 0 ? 1 : 0;
}

int lha_arch_chown(char* filename, int unix_uid, int unix_gid) {
    (void)filename;
    (void)unix_uid;
    (void)unix_gid;
    return 1;
}

int lha_arch_chmod(char* filename, int unix_perms) {
    (void)filename;
    (void)unix_perms;
    return 1;
}

int lha_arch_utime(char* filename, unsigned int timestamp) {
    (void)filename;
    (void)timestamp;
    return 1;
}

int lha_arch_set_windows_timestamps(char* filename, uint64_t creation_time,
                                    uint64_t modification_time, uint64_t access_time) {
    (void)filename;
    (void)creation_time;
    (void)modification_time;
    (void)access_time;
    return 0;
}

FILE* lha_arch_fopen(char* filename, int unix_uid, int unix_gid, int unix_perms) {
    (void)unix_uid;
    (void)unix_gid;
    (void)unix_perms;
    return fopen(filename, "wb");
}

LHAFileType lha_arch_exists(char* filename) {
    struct stat info;
    if (filename == nullptr || syscall_stat(filename, &info) != 0) return LHA_FILE_NONE;
    if ((info.st_mode & 0xF000u) == 0x4000u) return LHA_FILE_DIRECTORY;
    return LHA_FILE_FILE;
}

int lha_arch_symlink(char* path, char* target) {
    (void)path;
    (void)target;
    return 0;
}

}  // extern "C"
