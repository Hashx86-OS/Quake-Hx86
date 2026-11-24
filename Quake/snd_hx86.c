/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_hx86.c -- Hashx86 streaming audio backend.
//
// snd_mix.c paints into a small local ring. SNDDMA_Submit() copies newly
// painted frames into a kernel-owned ring through Hsys_audioWrite, and
// SNDDMA_GetDMAPos() reports the mixer's consumed byte position.

#include "quakedef.h"

#include <Hx86/Hsyscalls/syscalls.h>

#define SND_HX86_RATE 44100
#define SND_HX86_FRAMES 16384
#define SND_HX86_CHANNELS 2
#define SND_HX86_SAMPLE_BYTES 2
#define SND_HX86_FRAME_BYTES (SND_HX86_CHANNELS * SND_HX86_SAMPLE_BYTES)

static byte snd_ring[SND_HX86_FRAMES * SND_HX86_FRAME_BYTES];
static int32_t snd_stream = -1;
static uint32_t snd_submitted_frames;

qboolean SNDDMA_Init(void) {
    shm = &sn;
    memset((void*)shm, 0, sizeof(*shm));
    shm->gamealive = true;
    shm->soundalive = true;
    shm->splitbuffer = false;
    shm->channels = SND_HX86_CHANNELS;
    shm->samplebits = SND_HX86_SAMPLE_BYTES * 8;
    shm->speed = SND_HX86_RATE;
    shm->samples = SND_HX86_FRAMES * SND_HX86_CHANNELS;
    shm->submission_chunk = 1;
    shm->samplepos = 0;
    shm->buffer = snd_ring;

    snd_stream = syscall_audio_open(shm->speed, shm->channels, shm->samplebits);
    if (snd_stream < 0) {
        shm->buffer = NULL;
        return false;
    }

    snd_submitted_frames = 0;
    memset(snd_ring, 0, sizeof(snd_ring));
    Con_Printf("[SND] Hx86 audio: %d Hz, %d channels, %d-bit\n", shm->speed, shm->channels,
               shm->samplebits);
    return true;
}

int SNDDMA_GetDMAPos(void) {
    if (snd_stream < 0 || !shm || !shm->buffer) return 0;

    int32_t position = syscall_audio_get_position(snd_stream);
    if (position < 0) return shm->samplepos;

    shm->samplepos = ((uint32_t)position / SND_HX86_SAMPLE_BYTES) % shm->samples;
    return shm->samplepos;
}

void SNDDMA_Shutdown(void) {
    if (snd_stream >= 0) {
        syscall_audio_close(snd_stream);
        snd_stream = -1;
    }
    if (shm) shm->buffer = NULL;
    snd_submitted_frames = 0;
}

void SNDDMA_BeginPainting(void) {}

void SNDDMA_Submit(void) {
    if (snd_stream < 0 || !shm || !shm->buffer) return;

    uint32_t end = (uint32_t)paintedtime;
    if (end < snd_submitted_frames) {
        // paintedtime was reset (level change or long pause).
        snd_submitted_frames = end;
    }

    while (snd_submitted_frames < end) {
        uint32_t ring_pos = snd_submitted_frames & (SND_HX86_FRAMES - 1);
        uint32_t count = end - snd_submitted_frames;
        uint32_t until_end = SND_HX86_FRAMES - ring_pos;
        if (count > until_end) count = until_end;

        uint32_t bytes = count * SND_HX86_FRAME_BYTES;
        int32_t written = syscall_audio_write(
            snd_stream, shm->buffer + ring_pos * SND_HX86_FRAME_BYTES, bytes);
        if (written <= 0) break;

        uint32_t frames = (uint32_t)written / SND_HX86_FRAME_BYTES;
        snd_submitted_frames += frames;
        if (frames < count) break;
    }
}
