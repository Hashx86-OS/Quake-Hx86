#include "data/Archive.h"

#include <Hx86/Hx86.h>
#include <Hx86/Hgui/Hgui.h>
#include <stdio.h>
#include <string.h>

HX86_DECLARE_APP(HX86_APP_GUI);

extern "C" void QuakeMain(int argc, char *argv[]);

static char *build_argv[2];

static char g_installDir[256];
static Window* g_dialog = nullptr;
static Label* g_progressLabel = nullptr;
static Button* g_downloadButton = nullptr;
static Button* g_exitButton = nullptr;
static volatile bool g_installDone = false;
static bool g_busy = false;

static bool pak_file_exists(const char* path) {
    struct stat st;
    return syscall_stat(path, &st) == 0;
}

static void OnExitClicked() {
    syscall_exit_group(1);
}

static void SetProgress(const char* text) {
    if (g_progressLabel != nullptr) g_progressLabel->setText(text);
}

static void DataInstallProgress(const char* text) {
    SetProgress(text);
}

static void OnDownloadClicked() {
    if (g_busy) return;

    g_busy = true;
    if (g_downloadButton != nullptr) g_downloadButton->setEnabled(false);
    if (g_exitButton != nullptr) g_exitButton->setEnabled(false);
    SetProgress("Starting download...");

    char error[224];
    if (!ArchiveInstallSharewareData(g_installDir, DataInstallProgress, error, sizeof(error))) {
        char message[256];
        if (error[0] != '\0') {
            sprintf(message, "Download failed: %s", error);
        } else {
            strcpy(message, "Download failed.");
        }
        printf("[Quake] %s\n", message);
        SetProgress(message);
        g_busy = false;
        if (g_downloadButton != nullptr) g_downloadButton->setEnabled(true);
        if (g_exitButton != nullptr) g_exitButton->setEnabled(true);
        return;
    }

    printf("[Quake] Shareware data installed\n");
    SetProgress("Game data installed. Starting Quake...");
    g_installDone = true;
}

static void ShowMissingPakDialog(const char* pakpath) {
    init_graphics();

    g_dialog = new Window(desktop, 160, 150, 560, 340);
    g_dialog->setWindowTitle("Quake - Game Data Required");

    Label* title = new Label(g_dialog, 20, 20, 520, 30, "pak0.pak not found.");
    title->setSize(MEDIUM);

    char expectedText[400];
    sprintf(expectedText, "Expected: %s", pakpath);
    Label* expected = new Label(g_dialog, 20, 56, 520, 22, expectedText);
    expected->setSize(TINY);

    Label* explanation =
        new Label(g_dialog, 20, 92, 520, 22, "Quake needs the shareware episode 1 game data.");
    explanation->setSize(SMALL);

    Label* source =
        new Label(g_dialog, 20, 120, 520, 22,
                  "Download the original quake106.zip from gamers.org?");
    source->setSize(SMALL);

    Label* notice =
        new Label(g_dialog, 20, 146, 520, 40,
                  "Hashx86 does not host it. Episode 1 only; no commercial use.");
    notice->setSize(TINY);

    g_downloadButton = new Button(g_dialog, 20, 196, 190, 32, "Download Shareware");
    g_exitButton = new Button(g_dialog, 222, 196, 90, 32, "Exit");

    g_progressLabel = new Label(g_dialog, 20, 244, 520, 60, "");
    g_progressLabel->setSize(TINY);

    g_dialog->AddChild(title);
    g_dialog->AddChild(expected);
    g_dialog->AddChild(explanation);
    g_dialog->AddChild(source);
    g_dialog->AddChild(notice);
    g_dialog->AddChild(g_downloadButton);
    g_dialog->AddChild(g_exitButton);
    g_dialog->AddChild(g_progressLabel);

    g_downloadButton->OnClick(OnDownloadClicked);
    g_exitButton->OnClick(OnExitClicked);

    g_dialog->show();

    while (!g_installDone) {
        syscall_sleep(16);
    }

    // Give the GUI event callback a moment to unwind before tearing the
    // window down, then hand the framebuffer to Quake.
    syscall_sleep(50);
    if (g_dialog != nullptr) {
        g_dialog->Close();
        g_dialog = nullptr;
    }
}

extern "C" void _start(void* arg) {
    (void)arg;
    init_sys(0);

    // Resolve the directory of the running binary via getcwd() so the data
    // installer and Quake's basedir both use the folder containing Quake.bin.
    char cwd[256];
    int len = syscall_getcwd(cwd, sizeof(cwd));
    if (len <= 0) strcpy(cwd, "/");
    strcpy(g_installDir, cwd);

    char pakpath[300];
    int n = strlen(cwd);
    if (n > 0 && cwd[n - 1] == '/')
        sprintf(pakpath, "%sid1/pak0.pak", cwd);
    else
        sprintf(pakpath, "%s/id1/pak0.pak", cwd);

    if (!pak_file_exists(pakpath)) {
        printf("[Quake] %s not found\n", pakpath);
        ShowMissingPakDialog(pakpath);
        if (!pak_file_exists(pakpath)) {
            printf("[Quake] Game data is still missing, exiting\n");
            syscall_exit_group(1);
        }
    }

    build_argv[0] = "quake";
    build_argv[1] = NULL;

    printf("[Quake] Starting...\n");
    QuakeMain(1, build_argv);
    syscall_exit_group(0);
}
