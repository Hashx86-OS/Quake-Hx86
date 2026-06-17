#ifndef _SETJMP_H_
#define _SETJMP_H_

typedef struct { void *__context[6]; } jmp_buf[1];

#ifdef __cplusplus
extern "C" {
#endif

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#ifdef __cplusplus
}
#endif

#endif
