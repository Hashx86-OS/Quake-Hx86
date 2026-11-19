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

// C-linkage functions the vendored archive code needs that Quake's compat.c
// does not export with C linkage. Kept separate from ArchivePort.cpp so no
// header declares these names before the extern "C" definitions.

#include <Hx86/memory.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

namespace {

struct FormatSink {
    char* buffer;
    size_t capacity;
    size_t length;
};

void PutChar(FormatSink* sink, char character) {
    if (sink->length + 1 < sink->capacity) sink->buffer[sink->length] = character;
    sink->length++;
}

void PutText(FormatSink* sink, const char* text) {
    while (*text != '\0') PutChar(sink, *text++);
}

void PutUnsigned(FormatSink* sink, uint32_t value, uint32_t base, bool upper, int width,
                 char padding) {
    char digits[32];
    const char* alphabet = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int count = 0;
    do {
        digits[count++] = alphabet[value % base];
        value /= base;
    } while (value != 0);
    while (count < width) digits[count++] = padding;
    while (count > 0) PutChar(sink, digits[--count]);
}

int FormatV(char* buffer, size_t size, const char* format, va_list args) {
    FormatSink sink = {buffer, size, 0};
    while (*format != '\0') {
        if (*format != '%') {
            PutChar(&sink, *format++);
            continue;
        }
        format++;
        char padding = ' ';
        int width = 0;
        if (*format == '0') {
            padding = '0';
            format++;
        }
        while (*format >= '0' && *format <= '9') {
            width = width * 10 + (*format++ - '0');
        }
        bool isLong = false;
        if (*format == 'l') {
            isLong = true;
            format++;
        }
        switch (*format) {
            case 's':
                PutText(&sink, va_arg(args, const char*));
                break;
            case 'c':
                PutChar(&sink, (char)va_arg(args, int));
                break;
            case 'd':
            case 'i': {
                int32_t value = isLong ? (int32_t)va_arg(args, long) : va_arg(args, int);
                if (value < 0) {
                    PutChar(&sink, '-');
                    PutUnsigned(&sink, (uint32_t)(-value), 10, false, width - 1, padding);
                } else {
                    PutUnsigned(&sink, (uint32_t)value, 10, false, width, padding);
                }
                break;
            }
            case 'u': {
                uint32_t value = isLong ? (uint32_t)va_arg(args, unsigned long)
                                        : va_arg(args, unsigned int);
                PutUnsigned(&sink, value, 10, false, width, padding);
                break;
            }
            case 'x':
            case 'X': {
                uint32_t value = isLong ? (uint32_t)va_arg(args, unsigned long)
                                        : va_arg(args, unsigned int);
                PutUnsigned(&sink, value, 16, *format == 'X', width, padding);
                break;
            }
            case 'p':
                PutText(&sink, "0x");
                PutUnsigned(&sink, (uint32_t)(uintptr_t)va_arg(args, void*), 16, false, 8, '0');
                break;
            case '%':
                PutChar(&sink, '%');
                break;
            case '\0':
                format--;
                break;
            default:
                PutChar(&sink, '%');
                PutChar(&sink, *format);
                break;
        }
        if (*format != '\0') format++;
    }
    if (size != 0) {
        size_t end = sink.length < size - 1 ? sink.length : size - 1;
        sink.buffer[end] = '\0';
    }
    return (int)sink.length;
}

}  // namespace

extern "C" {

int sprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = FormatV(buffer, (size_t)-1, format, args);
    va_end(args);
    return result;
}

char* strrchr(const char* text, int character) {
    char wanted = (char)character;
    const char* found = nullptr;
    for (;;) {
        if (*text == wanted) found = text;
        if (*text == '\0') return (char*)found;
        text++;
    }
}

char* strdup(const char* text) {
    if (text == nullptr) return nullptr;
    size_t length = 0;
    while (text[length] != '\0') length++;
    char* copy = (char*)kmalloc((int)(length + 1));
    if (copy == nullptr) return nullptr;
    for (size_t i = 0; i <= length; i++) copy[i] = text[i];
    return copy;
}

}  // extern "C"
