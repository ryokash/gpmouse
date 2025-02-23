#include <stdio.h>
#include <stdint.h>
#include <iterator>
#include <thread>
#include <tuple>
#include <stdexcept>
#include <format>

#include <strsafe.h>
#include <wchar.h>
#include <windows.h>
#include <windowsx.h>

#include "framework.h"
#include "resource.h"
#include "gpmouse.h"
#include "config.h"

#define APP_MUTEX L"{022E64D2-8A69-49D1-8764-150040109CA2}"
#define WM_TASKTRAY (WM_APP + 1)

#define UNUSED(a)

// void Cls_OnTaskTray(HWND hwnd, UINT id, UINT uMsg)
#define HANDLE_WM_TASKTRAY(hwnd, wParam, lParam, fn) \
  ((fn)((hwnd), (UINT)(wParam), (UINT)(lParam)), 0L)

namespace {

constexpr wchar_t WINDOW_NAME[] = L"GPmouse";
constexpr wchar_t CLASS_NAME[] = L"GPmouseTray";
constexpr int TASKTRAY_ICONID = 1;

uint32_t g_status = GP_STATUS_INITIALIZING;


inline POINT GetCursorPos()
{
    POINT p = {};
    if (!::GetCursorPos(&p))
        ; // TODO: log
    return p;
}

std::tuple<bool, HANDLE> has_prev_instance()
{
    auto h = CreateMutexW(0, TRUE, APP_MUTEX);
    if (h == 0) 
        // TODO: FormatMessage ‚ðŽg‚¤
        throw std::runtime_error(std::format("Failed to create mutex with code {}.", GetLastError()));

    auto ec = GetLastError();
    return { ec == ERROR_ALREADY_EXISTS, h };
}

}  // namespace

BOOL Cls_OnCreate(HWND hwnd, LPCREATESTRUCT UNUSED(cs))
{
    auto instance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    // Add task tray icon.
    NOTIFYICONDATA nid = {
        sizeof(NOTIFYICONDATA),
        hwnd,
        TASKTRAY_ICONID,
        NIF_MESSAGE|NIF_ICON|NIF_TIP,
        WM_TASKTRAY,
        LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1)),
    };
    StringCchCopy(nid.szTip, std::size(nid.szTip), L"GPmouse");

    return Shell_NotifyIcon(NIM_ADD, &nid);
}

void Cls_OnDestroy(HWND hwnd)
{
    // Remove task tray icon.
    NOTIFYICONDATA nid = {
        sizeof(NOTIFYICONDATA),
        hwnd,
        TASKTRAY_ICONID,
    };
    Shell_NotifyIcon(NIM_DELETE, &nid);
    PostQuitMessage(0);
}

void Cls_OnClose(HWND hwnd)
{
    DestroyWindow(hwnd);
}

void Cls_OnCommand(HWND hwnd, int id, HWND hWndCtl, UINT codeNotify)
{
    switch (id) {
    case IDM_FOLDER:
        break;

    case IDM_QUIT:
        PostMessage(hwnd, WM_QUIT, 0, 0);
        break;

    default:
        break;
    }
}
void Cls_OnTaskTray(HWND hwnd, UINT id, UINT uMsg) 
{
    if (id != TASKTRAY_ICONID)
        return;
    
    switch (uMsg) {
    case WM_RBUTTONDOWN: {
        // Display menu when right button is clicked on task tray icon.
        auto point = GetCursorPos();
        auto instance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        auto hMenu = LoadMenu(instance, MAKEINTRESOURCE(IDR_MENU1));
        auto hSubMenu = GetSubMenu(hMenu, 0);
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hSubMenu, TPM_LEFTALIGN|TPM_BOTTOMALIGN, point.x,
            point.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);
        PostMessage(hwnd, WM_NULL, 0, 0);
    } 
        break;

    default:
        break;
    }
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        HANDLE_MSG(hwnd, WM_CREATE, Cls_OnCreate);
        HANDLE_MSG(hwnd, WM_DESTROY, Cls_OnDestroy);
        HANDLE_MSG(hwnd, WM_COMMAND, Cls_OnCommand);
        HANDLE_MSG(hwnd, WM_CLOSE, Cls_OnClose);
        HANDLE_MSG(hwnd, WM_TASKTRAY, Cls_OnTaskTray);
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}


#define UPDATE_GP_STATUS(status, value) \
do { \
    (status) = value; \
    WakeByAddressAll(&(status)); \
} while (0)

HWND create_tray_window(HINSTANCE instance)
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    if (!RegisterClass(&wc))
        return NULL;

    return CreateWindow(
        CLASS_NAME,
        WINDOW_NAME,
        WS_DISABLED,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        instance,
        NULL
    );
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE UNUSED(prev), LPTSTR cmdline, int UNUSED(cmdshow))
{
    using namespace gpmouse;

    auto [prev, mutex] = has_prev_instance();
    if (prev)
        return 1; // TODO:

    if (!xinput_initialize())
        return (int)GetLastError();

    try {
        configure();
    }
    catch (std::exception& exc) {
        MessageBoxA(0, exc.what(), "error", MB_OK);
        return -1;
    }
    catch (...) {
        return -1;
    }

    uint32_t status = GP_STATUS_INITIALIZING;
    concurrent_queue<xinput_t> queue;
    std::thread handler_thread(handle_xinput, &status, &queue);
    std::thread check_thread(check_xinput, &status, &queue);

    auto hwnd = create_tray_window(instance);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    // TODO: close window

    UPDATE_GP_STATUS(status, GP_STATUS_TERMINATING);

    check_thread.join();
    handler_thread.join();
    
    xinput_finalize();

    ReleaseMutex(mutex);
    CloseHandle(mutex);

    return (int)msg.wParam;
}
