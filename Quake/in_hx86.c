#include "quakedef.h"
#include <Hx86/Hx86.h>

cvar_t m_filter = {"m_filter","1"};

qboolean mouse_avail = true;
int mouse_buttons = 3;
int mouse_oldbuttonstate = 0;
int mouse_buttonstate = 0;
float mouse_x, mouse_y;
float old_mouse_x, old_mouse_y;

static uint8_t prevKeyStates[128];
static InputState cachedInput;
static int inputCached = 0;

static int scancode_to_quake[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 127, 9,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13, 133, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 134, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 134, '*', 132, ' ', 0, 135, 136, 137, 138, 139,
    140, 141, 142, 143, 144, 145, 146, 151, 128, 150, 0, 130, 0, 131,
    129, 152, 129, 149, 147, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void IN_StartupMouse(void) {
    if (COM_CheckParm("-nomouse"))
        return;
    Con_Printf("Mouse available\n");
    mouse_avail = true;
    mouse_buttons = 3;
}

extern kbutton_t in_mlook;

void IN_Init(void) {
    Cvar_RegisterVariable(&m_filter);
    IN_StartupMouse();
    memset(prevKeyStates, 0, sizeof(prevKeyStates));
    in_mlook.state = 1;
}

void IN_Shutdown(void) {
}

void IN_Commands(void) {
    if (!inputCached) { syscall_get_input(&cachedInput); inputCached = 1; }

    mouse_buttonstate = cachedInput.mouseButtons;

    for (int i = 0; i < mouse_buttons; i++) {
        if ((mouse_buttonstate & (1 << i)) && !(mouse_oldbuttonstate & (1 << i)))
            Key_Event(K_MOUSE1 + i, true);
        if (!(mouse_buttonstate & (1 << i)) && (mouse_oldbuttonstate & (1 << i)))
            Key_Event(K_MOUSE1 + i, false);
    }
    mouse_oldbuttonstate = mouse_buttonstate;
}

void IN_MouseMove(usercmd_t *cmd) {
    if (!mouse_avail) return;

    if (!inputCached) { syscall_get_input(&cachedInput); inputCached = 1; }

    int mx = cachedInput.mouseDX;
    int my = cachedInput.mouseDY;

    if (m_filter.value) {
        mouse_x = (mx + old_mouse_x) * 0.5f;
        mouse_y = (my + old_mouse_y) * 0.5f;
    } else {
        mouse_x = (float)mx;
        mouse_y = (float)my;
    }
    old_mouse_x = (float)mx;
    old_mouse_y = (float)my;

    mouse_x *= sensitivity.value;
    mouse_y *= sensitivity.value;

    if ((in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1)))
        cmd->sidemove += m_side.value * mouse_x;
    else
        cl.viewangles[YAW] -= m_yaw.value * mouse_x;

    if (in_mlook.state & 1)
        V_StopPitchDrift();

    if ((in_mlook.state & 1) && !(in_strafe.state & 1)) {
        cl.viewangles[PITCH] += m_pitch.value * mouse_y;
        if (cl.viewangles[PITCH] > 80) cl.viewangles[PITCH] = 80;
        if (cl.viewangles[PITCH] < -70) cl.viewangles[PITCH] = -70;
    } else {
        if ((in_strafe.state & 1) && noclip_anglehack)
            cmd->upmove -= m_forward.value * mouse_y;
        else
            cmd->forwardmove -= m_forward.value * mouse_y;
    }
}

void IN_Move(usercmd_t *cmd) {
    IN_MouseMove(cmd);
}

void IN_ClearStates(void) {
    memset(prevKeyStates, 0, sizeof(prevKeyStates));
    mouse_oldbuttonstate = 0;
    mouse_buttonstate = 0;
    mouse_x = mouse_y = 0;
    old_mouse_x = old_mouse_y = 0;
}

void Sys_SendKeyEvents(void) {
    syscall_get_input(&cachedInput);
    inputCached = 1;

    for (int i = 0; i < 128; i++) {
        if (cachedInput.keyStates[i] != prevKeyStates[i]) {
            int quakeKey = scancode_to_quake[i];
            if (quakeKey != 0) {
                Key_Event(quakeKey, cachedInput.keyStates[i]);
            }
        }
    }
    memcpy(prevKeyStates, cachedInput.keyStates, 128);
}
