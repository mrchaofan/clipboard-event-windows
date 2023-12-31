#ifndef CLIPBOARD_EVENT_H
#define CLIPBOARD_EVENT_H
#include <uv.h>

#define WM_GRACEFUL_EXIT (WM_USER + 1)

int listenClipboardChange(void (*)(void));

void TerminateWin32(uv_thread_t *tid);
#endif