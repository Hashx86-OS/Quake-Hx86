#include "quakedef.h"
#include "d_local.h"
#include <Hx86/Hx86.h>

static uint32_t *framebuffer = 0;
static int fb_width = 320;
static int fb_height = 200;
static byte palette32[1024]; // 256 * 4 bytes (RGBA)

byte vid_current_palette[768];
unsigned short d_8to16table[256];
unsigned d_8to24table[256];

int d_con_indirect = 0;

int vid_modenum;
int vid_testingmode;
double vid_testendtime;
extern double realtime;
int vid_realmode;

extern int D_SurfaceCacheForRes(int width, int height);
extern void D_InitCaches(void *buffer, int size);
extern int vid_surfcachesize;

static void Build8to24Table(unsigned char *palette) {
    for (int i = 0; i < 256; i++) {
        unsigned char r = palette[i * 3];
        unsigned char g = palette[i * 3 + 1];
        unsigned char b = palette[i * 3 + 2];
        palette32[i * 4 + 0] = r;
        palette32[i * 4 + 1] = g;
        palette32[i * 4 + 2] = b;
        palette32[i * 4 + 3] = 0xFF;
        d_8to24table[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
}

static void VID_SetupMode(int width, int height) {
    int bufSize = width * height;
    int zbufSize = width * height * 2;

    vid.buffer = (pixel_t *)Hunk_HighAllocName(bufSize + zbufSize, "video");
    if (!vid.buffer)
        Sys_Error("VID_SetupMode: Failed to allocate %d bytes", bufSize + zbufSize);

    d_pzbuffer = (short *)(vid.buffer + bufSize);
    d_zrowbytes = width * 2;
    d_zwidth = width;

    memset(vid.buffer, 0, bufSize);
    memset(d_pzbuffer, 0, zbufSize);

    vid.width = width;
    vid.height = height;
    vid.rowbytes = width;
    vid.aspect = 1.0;
    vid.numpages = 1;
    vid.recalc_refdef = 1;
    vid.conbuffer = vid.buffer;
    vid.conwidth = width;
    vid.conheight = height;
    vid.conrowbytes = width;
    vid.maxwarpwidth = width;
    vid.maxwarpheight = height;
    vid.direct = 0;
}

void VID_Init(unsigned char *palette) {
    FramebufferInfo fb = syscall_get_framebuffer();
    framebuffer = (uint32_t*)fb.buffer;
    fb_width = (int)fb.width;
    fb_height = (int)fb.height;

    printf("[Quake] Framebuffer: %dx%d @ 0x%x\n", fb_width, fb_height, (unsigned int)fb.buffer);

    Q_memcpy(vid_current_palette, palette, 768);
    Build8to24Table(palette);

    int vid_w = fb_width;
    int vid_h = fb_height;
    if (vid_w > 800) vid_w = 800;
    if (vid_h > 600) vid_h = 600;

    VID_SetupMode(vid_w, vid_h);

    int cachesize = D_SurfaceCacheForRes(vid.width, vid.height);
    void *cachebuf = Hunk_HighAllocName(cachesize, "surface_cache");
    if (cachebuf)
        D_InitCaches(cachebuf, cachesize);
    else
        printf("[Quake] Warning: Could not allocate %d byte surface cache\n", cachesize);

    vid_modenum = 0;
    Cvar_SetValue("vid_mode", 0);

    vid.colormap = host_colormap;

    vid_menudrawfn = NULL;
    vid_menukeyfn = NULL;
}

int VID_SetMode(int modenum, unsigned char *palette) {
    (void)modenum;
    if (palette) {
        Q_memcpy(vid_current_palette, palette, 768);
        Build8to24Table(palette);
    }
    return 1;
}

void VID_Shutdown(void) {
    vid_testingmode = 0;
}

void VID_SetPalette(unsigned char *palette) {
    if (palette != vid_current_palette)
        Q_memcpy(vid_current_palette, palette, 768);
    Build8to24Table(vid_current_palette);
}

void VID_ShiftPalette(unsigned char *palette) {
    VID_SetPalette(palette);
}

void VID_HandlePause(qboolean pause) {
    (void)pause;
}

void VID_Update(vrect_t *rects) {
    (void)rects;

    if (!framebuffer || !vid.buffer) return;

    pixel_t *src = vid.buffer;
    int srcW = vid.width;
    int srcH = vid.height;

    int destW = fb_width;
    int destH = fb_height;

    for (int y = 0; y < destH; y++) {
        int srcY = (y * srcH) / destH;
        int dstOff = y * destW;
        int srcOff = srcY * srcW;

        if (srcW == destW) {
            uint8_t *s = src + srcOff;
            uint32_t *d = framebuffer + dstOff;
            for (int x = 0; x < destW; x++) {
                d[x] = d_8to24table[s[x]];
            }
        } else {
            for (int x = 0; x < destW; x++) {
                int srcX = (x * srcW) / destW;
                framebuffer[dstOff + x] = d_8to24table[src[srcOff + srcX]];
            }
        }
    }
}

void D_BeginDirectRect(int x, int y, byte *pbitmap, int width, int height) {
    (void)x; (void)y; (void)pbitmap; (void)width; (void)height;
}

void D_EndDirectRect(int x, int y, int width, int height) {
    (void)x; (void)y; (void)width; (void)height;
}

void VID_InitExtra(void) {
}

void VGA_Init(void) {
}

int VID_NumModes(void) {
    return 1;
}

char *VID_ModeInfo(int modenum, char **ppheader) {
    (void)modenum;
    if (ppheader) *ppheader = NULL;
    return "Hx86 Framebuffer";
}

void VID_TestMode_f(void) {}
void VID_NumModes_f(void) {}
void VID_DescribeCurrentMode_f(void) {}
void VID_DescribeMode_f(void) {}
void VID_DescribeModes_f(void) {}

qboolean VGA_CheckAdequateMem(int width, int height, int rowbytes, int allocnewbuffer) {
    (void)width; (void)height; (void)rowbytes; (void)allocnewbuffer;
    return true;
}
