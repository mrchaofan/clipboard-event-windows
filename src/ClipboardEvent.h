#ifndef CLIPBOARD_EVENT_H
#define CLIPBOARD_EVENT_H
#include <Windows.h>

#define WM_GRACEFUL_EXIT (WM_USER + 1)

int listenClipboardChange(void (*)(void));

#endif