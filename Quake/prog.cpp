#include <Hx86/Hx86.h>
#include <Hx86/Hgui/Hgui.h>
#include <string.h>
#include <stdio.h>

HX86_DECLARE_APP(HX86_APP_GUI);

extern "C" void QuakeMain(int argc, char *argv[]);

static char *build_argv[2];

static bool pak_file_exists(const char* path) {
    struct stat st;
    return syscall_stat(path, &st) == 0;
}

static void OnExitClicked() {
    syscall_exit_group(1);
}

static void ShowMissingPakDialog(const char* pakpath) {
    init_graphics();

    Window* win = new Window(desktop, 200, 200, 480, 190);
    win->setWindowTitle("Quake - Missing Data");

    Label* msg = new Label(win, 20, 20, 440, 40, "pak0.pak not found!");
    msg->setSize(MEDIUM);

    char hintText[400];
    sprintf(hintText, "Expected: %s", pakpath);
    Label* hint = new Label(win, 20, 62, 440, 30, hintText);
    hint->setSize(SMALL);

    Label* hint2 = new Label(win, 20, 96, 440, 30, "Place pak0.pak there and relaunch.");
    hint2->setSize(SMALL);

    Button* btn = new Button(win, 190, 135, 100, 30, "Exit");
    btn->OnClick(OnExitClicked);

    win->AddChild(msg);
    win->AddChild(hint);
    win->AddChild(hint2);
    win->AddChild(btn);
    win->show();

    while (1) {
        syscall_sleep(16);
    }
}

extern "C" void _start(void* arg) {
    (void)arg;
    init_sys(0);

    // Resolve directory of the running binary via getcwd() so the pak file is
    // located in the id1/ folder alongside Quake.bin rather than an absolute path.
    char cwd[256];
    int len = syscall_getcwd(cwd, sizeof(cwd));
    if (len <= 0)
        strcpy(cwd, "/");

    char pakpath[300];
    int n = strlen(cwd);
    if (n > 0 && cwd[n - 1] == '/')
        sprintf(pakpath, "%sid1/pak0.pak", cwd);
    else
        sprintf(pakpath, "%s/id1/pak0.pak", cwd);

    if (!pak_file_exists(pakpath)) {
        printf("[Quake] %s not found\n", pakpath);
        ShowMissingPakDialog(pakpath);
    }

    build_argv[0] = "quake";
    build_argv[1] = NULL;

    printf("[Quake] Starting...\n");
    QuakeMain(1, build_argv);
    syscall_exit(0);
}
