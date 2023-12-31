#include <iostream>
#include "ClipboardEvent.h"

// Clipboard update callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int listenClipboardChange(void (*callback)(void))
{
    // Register a dummy window class
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"DUMMY_CLASS";
    if (!RegisterClassExW(&wc))
    {
        int ret = GetLastError();
        std::cerr << "Failed to register window class" << ret << "\n";
        return ret;
    }

    // Create a dummy window
    HWND hwnd = CreateWindowExW(0, L"DUMMY_CLASS", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (!hwnd)
    {
        int ret = GetLastError();
        std::cerr << "Failed to create window\n";
        return ret;
    }

    // Add the window to the clipboard viewer chain
    if (!AddClipboardFormatListener(hwnd))
    {
        int ret = GetLastError();
        std::cerr << "Failed to add clipboard format listener\n";
        return ret;
    }

    // Message loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_CLIPBOARDUPDATE)
        {
            callback();
        }
        else if (msg.message == WM_GRACEFUL_EXIT)
        {
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // Remove the window from the clipboard viewer chain
    RemoveClipboardFormatListener(hwnd);

    // Destroy the window
    DestroyWindow(hwnd);

    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

void TerminateWin32(uv_thread_t *tid)
{
    bool suc = PostThreadMessage((DWORD)GetThreadId(*tid), WM_GRACEFUL_EXIT, 0, 0);
    if (!suc)
    {
        DWORD er = GetLastError();
        std::cerr << "PostThreadMessage" << er << "\n";
    }
}