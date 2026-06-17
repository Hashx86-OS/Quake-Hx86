#include "quakedef.h"
#include <Hx86/Hx86.h>
#include <stdarg.h>

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x40
#define O_TRUNC 0x200

qboolean isDedicated;
int nostdout = 0;
char *basedir = "";
static int fpu_control_word;
cvar_t sys_linerefresh = {"sys_linerefresh","0"};

extern void Sys_SetFPCW(void);

static double sys_time_base = 0.0;
static double sys_time_scale = 0.0;

void Sys_Printf(char *fmt, ...) {
    va_list argptr;
    char text[1024];
    va_start(argptr, fmt);
    vsprintf(text, fmt, argptr);
    va_end(argptr);
    printf("%s", text);
}

void Sys_Quit(void) {
    Host_Shutdown();
    syscall_exit(0);
}

void Sys_Init(void) {
    Sys_SetFPCW();
    {
        uint32_t lo, hi;
        __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
        sys_time_base = (double)(((uint64_t)hi << 32) | lo);
        sys_time_scale = 1.0 / 1000000000.0;
    }
}

void Sys_Error(char *error, ...) {
    va_list argptr;
    char string[1024];
    va_start(argptr, error);
    vsprintf(string, error, argptr);
    va_end(argptr);
    printf("Error: %s\n", string);
    Host_Shutdown();
    syscall_exit(1);
}

void Sys_Warn(char *warning, ...) {
    va_list argptr;
    char string[1024];
    va_start(argptr, warning);
    vsprintf(string, warning, argptr);
    va_end(argptr);
    printf("Warning: %s", string);
}

int Sys_FileTime(char *path) {
    struct stat buf;
    if (syscall_stat(path, &buf) == -1) return -1;
    return 0;
}

void Sys_mkdir(char *path) {
    (void)path;
}

static int syscall_lseek(int fd, int offset, int whence) {
    int ret;
    __asm__ volatile ("int $0x80"
        : "=a" (ret)
        : "0" (19), "b" (fd), "c" (offset), "d" (whence)
        : "memory");
    return ret;
}

int Sys_FileOpenRead(char *path, int *handle) {
    int h = syscall_open(path, O_RDONLY);
    *handle = h;
    if (h == -1) return -1;
    struct stat fileinfo;
    syscall_stat(path, &fileinfo);
    return fileinfo.st_size;
}

int Sys_FileOpenWrite(char *path) {
    return syscall_open(path, O_RDWR | O_CREAT | O_TRUNC);
}

int Sys_FileWrite(int handle, void *src, int count) {
    return syscall_write(handle, (const char*)src, count);
}

void Sys_FileClose(int handle) {
    syscall_close(handle);
}

void Sys_FileSeek(int handle, int position) {
    syscall_lseek(handle, position, 0);
}

int Sys_FileRead(int handle, void *dest, int count) {
    return syscall_read(handle, (char*)dest, count);
}

void Sys_DebugLog(char *file, char *fmt, ...) {
    va_list argptr;
    static char data[1024];
    int fd;
    va_start(argptr, fmt);
    vsprintf(data, fmt, argptr);
    va_end(argptr);
    fd = syscall_open(file, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd >= 0) {
        syscall_write(fd, data, strlen(data));
        syscall_close(fd);
    }
}

void Sys_EditFile(char *filename) {
    (void)filename;
}

double Sys_FloatTime(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    double t = (double)(((uint64_t)hi << 32) | lo);
    return (t - sys_time_base) * sys_time_scale;
}

void Sys_Sleep(void) {
    syscall_sleep(1);
}

char *Sys_ConsoleInput(void) {
    return NULL;
}

void Sys_LineRefresh(void) {
}

void Sys_MakeCodeWriteable(unsigned long startaddr, unsigned long length) {
    (void)startaddr;
    (void)length;
}

void Sys_HighFPPrecision(void) {
    // Restore double-precision (53-bit mantissa), round-to-nearest
    unsigned short cw;
    __asm__ volatile ("fstcw %0" : "=m" (cw));
    cw &= ~0x0F00;   // Clear precision and rounding bits
    cw |= 0x0200;    // Precision: double (53-bit)
    // Rounding: round-to-nearest (bits 10-11 = 00)
    __asm__ volatile ("fldcw %0" : : "m" (cw));
}

void Sys_LowFPPrecision(void) {
    // Set single-precision (24-bit mantissa), chop/truncate mode
    unsigned short cw;
    __asm__ volatile ("fstcw %0" : "=m" (cw));
    cw &= ~0x0F00;   // Clear precision and rounding bits
    // Precision: single (24-bit) = bits 8-9 = 00
    cw |= 0x0C00;    // Rounding: chop/truncate = bits 10-11 = 11
    __asm__ volatile ("fldcw %0" : : "m" (cw));
}

void Sys_SetFPCW(void) {
    __asm__ volatile ("fstcw %0" : "=m" (fpu_control_word));
    unsigned short cw = fpu_control_word;
    cw &= ~0x300;
    cw |= 0x200;
    __asm__ volatile ("fldcw %0" : : "m" (cw));
}

void floating_point_exception_handler(int whatever) {
    (void)whatever;
}

#define MINIMUM_MEMORY 0x550000


extern "C" void QuakeMain(int argc, char *argv[]) {
    double time, oldtime, newtime;
    quakeparms_t parms;
    int j;

    memset(&parms, 0, sizeof(parms));

    COM_InitArgv(argc, argv);
    parms.argc = com_argc;
    parms.argv = com_argv;

    parms.memsize = 24 * 1024 * 1024;
    j = COM_CheckParm("-mem");
    if (j)
        parms.memsize = (int)(Q_atof(com_argv[j + 1]) * 1024 * 1024);
    parms.membase = kmalloc(parms.memsize);
    if (!parms.membase) {
        printf("[Quake] FATAL: Failed to allocate %d bytes for heap\n", parms.memsize);
        syscall_exit(1);
    }

    parms.basedir = basedir;

    Host_Init(&parms);
    Sys_Init();

    // Queue WASD bindings AFTER quake.rc/default.cfg via the command buffer
    // so they override the default Quake bindings (which use arrow keys)
    Cbuf_AddText("bind w +forward\n");
    Cbuf_AddText("bind s +back\n");
    Cbuf_AddText("bind a +moveleft\n");
    Cbuf_AddText("bind d +moveright\n");
    Cbuf_AddText("bind SPACE +jump\n");
    Cbuf_AddText("bind ALT +strafe\n");
    Cbuf_AddText("bind CTRL +speed\n");

    if (COM_CheckParm("-nostdout"))
        nostdout = 1;
    else
        printf("Hx86 Quake -- Version %0.3f\n", (double)VERSION);

    oldtime = Sys_FloatTime() - 0.1;
    while (1) {
        newtime = Sys_FloatTime();
        time = newtime - oldtime;

        if (time > sys_ticrate.value * 2)
            oldtime = newtime;
        else
            oldtime += time;

        Host_Frame(time);

        if (sys_linerefresh.value)
            Sys_LineRefresh();
    }
}
