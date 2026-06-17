#include <Hx86/Hx86.h>

HX86_DECLARE_APP(HX86_APP_GUI);

extern "C" void QuakeMain(int argc, char *argv[]);

static char *build_argv[2];

extern "C" void _start(void* arg) {
    (void)arg;
    init_sys(0);

    build_argv[0] = "quake";
    build_argv[1] = NULL;

    printf("[Quake] Starting...\n");
    QuakeMain(1, build_argv);
    syscall_exit(0);
}
