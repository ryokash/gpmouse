// gpmouse.cpp : Defines the entry point for the application.
//
#define ENABLE_GUIDE_BUTTON

#include "framework.h"
#include "gpmouse.h"
#include "config.h"
#include <string>
#include <thread>
#include <array>
#include <bitset>
#include <regex>
#include <format>
#include <unordered_map>
#include <stdint.h>
#include <assert.h>

#include <windows.h>
#include <psapi.h>
#include <xinput.h>
#include <shellapi.h>

#pragma comment(lib, "Synchronization.lib")
#pragma comment(lib, "xinput.lib")
#ifdef ENABLE_GUIDE_BUTTON
    static DWORD (WINAPI *XInputGetStateEx)(DWORD user_index, XINPUT_STATE* state);
    static HMODULE xinput_dll = 0;
#   define XInputGetState(a, b) XInputGetStateEx((a), (b))
#else
#   pragma comment(lib, "xinput9_1_0.lib")
#endif

#define MAX_LOADSTRING 100

using namespace gpmouse;



#define INTERVAL 10
#define FPS (1000/INTERVAL)

struct point_t
{
    float x;
    float y;
};
bool operator==(const point_t& a, const point_t& b)
{
    return a.x == b.x && a.y == b.y;
}

enum class mouse_button_t : uint32_t
{
    invalid = 0,
    left = 0x0002,
    right = 0x0008,
    middle = 0x0020,
};

enum class stick_mode_t : uint32_t
{
    mouse = 0,
    touch = 2,
    multi_touch = 3,
    MAX = 3,
};
struct input_state_t
{
    stick_mode_t stick_mode;
    std::bitset<256> keys;
    bool single; // ボタンの同時押しがなされていない
    //std::bitset<4> modifiers;
    uint16_t modifiers;
    uint16_t vk_buttons[16]; // 今現在ボタンがどのキーにバインドされているか。
    point_t touch_start[2];
    POINTER_TOUCH_INFO touch[2];
};
input_state_t g_input_state = {};



UINT send_input(UINT n, INPUT* inputs)
{
#ifdef _DEBUG
    auto logger = get_logger();
    logger->debug("---------- send_input ------------");
    logger->debug("number of input: {}", n);
    for (auto i = inputs; i - inputs < n; ++i) {
        if (i->type == INPUT_MOUSE) {
            // TODO:
            logger->debug("<mouse_input>");
        }
        else if (i->type == INPUT_KEYBOARD) {
            bool up = i->ki.dwFlags == KEYEVENTF_KEYUP;
            logger->debug("<kbd_input {} {}>", vk_name(i->ki.wVk), up ? "Up" : "Down");
        }
        //logger->debug("i.type
    }
    logger->debug("----------------------------------");
#endif
    return SendInput(n, inputs, sizeof(INPUT));
}
UINT send_input(std::vector<INPUT>& inputs)
{
    return send_input(inputs.size(), inputs.data());
}

void calibrate_stick_params(stick_params_t& params, const XINPUT_GAMEPAD& in)
{
    params.cursor.cx = in.sThumbLX;
    params.cursor.cy = in.sThumbLY;
    params.scroll.cx = in.sThumbRX;
    params.scroll.cy = in.sThumbRY;
    params.initialized = true;
    // TODO: check スレッドと handle スレッド両方で参照するので、メモリバリアが必要
}

void move_cursor(const point_t& move)
{
    INPUT i = {};
    i.type = INPUT_MOUSE;
    i.mi.dx = (int)move.x;
    i.mi.dy = (int)(move.y * -1);
    i.mi.dwFlags = MOUSEEVENTF_MOVE;
    send_input(1, &i);
}
void left_stick(const stick_t& cfg, const XINPUT_GAMEPAD& input)
{
    auto x = input.sThumbLX - cfg.cx;
    auto y = input.sThumbLY - cfg.cy;

    if (x*x + y*y <= cfg.deadzone * cfg.deadzone)
        return;

    float accel = 0.0f;
    if (cfg.left_trigger == trigger_function_t::acceleration)
        accel = std::max<float>(accel, input.bLeftTrigger);
    if (cfg.right_trigger == trigger_function_t::acceleration)
        accel = std::max<float>(accel, input.bRightTrigger);
    //OutputDebugStringA(std::format("raw accel: {}\n", accel).c_str());

    if (cfg.accel_type == accel_type_t::linear)
        accel = accel * (cfg.accel_max - 1) / 255 + 1;
    else {
        //OutputDebugStringA(std::format("exp(accel): {}, max: {}, exp(255): {}\n", exp((double)accel), cfg.accel_max, exp(255)).c_str());
        //accel = exp(accel - 255) * (cfg.accel_max - 1) + 1;
        accel = pow(cfg.accel_max, accel/255.);
    }
    //OutputDebugStringA(std::format("accel: {}\n", accel).c_str());

    float brake = 0.0f;
    if (cfg.left_trigger == trigger_function_t::deacceleration)
        brake = std::max<float>(brake, input.bLeftTrigger);
    if (cfg.right_trigger == trigger_function_t::deacceleration)
        brake = std::max<float>(brake, input.bRightTrigger);
    brake = brake * (cfg.deaccel_max - 1) / 255 + 1;

    // Remark: dy の計算で SM_CXSCREEN を使っているのは間違いではない。
    //   SM_CXSCREEN で計算すると、縦と横でカーソルのスピードが違ってしまうため、
    //   Y軸方向も SM_CXSCREEN で計算している。スクロールの方は、縦と横で
    //   違っていても問題がないため、SM_CYSCREEN を使う。
    auto dx = cfg.base_speed * x * accel / (GetSystemMetrics(SM_CXSCREEN) * brake);
    auto dy = cfg.base_speed * y * accel / (GetSystemMetrics(SM_CXSCREEN) * brake);

    if (g_input_state.stick_mode == stick_mode_t::mouse) {
        INPUT i = {};
        i.type = INPUT_MOUSE;
        i.mi.dx = (int)dx;
        i.mi.dy = (int)(dy * -1);
        i.mi.dwFlags = MOUSEEVENTF_MOVE;
        //i.mi.time = 0;
        send_input(1, &i);
    }
    else {
    }
}
void vscroll(float s)
{
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.mouseData = (DWORD)s;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.time = 0;
    send_input(1, &input);
}
void hscroll(float s)
{
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.mouseData = (DWORD)s;
    input.mi.dwFlags = MOUSEEVENTF_HWHEEL;
    input.mi.time = 0;
    send_input(1, &input);
}
void right_stick(const stick_t& cfg, const XINPUT_GAMEPAD& input)
{
    auto x = input.sThumbRX - cfg.cx;
    auto y = input.sThumbRY - cfg.cy;

    if (x * x + y * y <= cfg.deadzone * cfg.deadzone)
        return;

    float accel = 0.0f;
    if (cfg.left_trigger == trigger_function_t::acceleration)
        accel = std::max<float>(accel, input.bLeftTrigger);
    if (cfg.right_trigger == trigger_function_t::acceleration)
        accel = std::max<float>(accel, input.bRightTrigger);
    accel = accel * (cfg.accel_max - 1) / 255 + 1;

    float brake = 0.0f;
    if (cfg.left_trigger == trigger_function_t::deacceleration)
        brake = std::max<float>(brake, input.bLeftTrigger);
    if (cfg.right_trigger == trigger_function_t::deacceleration)
        brake = std::max<float>(brake, input.bRightTrigger);
    brake = brake * (cfg.deaccel_max - 1) / 255 + 1;

    if (g_input_state.stick_mode == stick_mode_t::mouse) {
        hscroll((float)(x * cfg.base_speed * accel)/(FPS * brake));
        vscroll((float)(y * cfg.base_speed * accel)/(FPS * brake));
    }
    else {
        g_input_state.stick_mode = stick_mode_t::multi_touch;

        if (g_input_state.touch_start[0] == g_input_state.touch_start[1]) {
            // タッチの開始
        }
        else {
            //auto new_location = {  }
            //sendtouch
        }
    }
}


DWORD get_process_id_under_cursor()
{
    POINT pt;
    GetCursorPos(&pt);

    auto hwnd = WindowFromPoint(pt);
    if (!hwnd)
        return 0;

    DWORD pid;
    if (GetWindowThreadProcessId(hwnd, &pid) == 0)
        return 0;
    
    return pid;
}

DWORD get_foreground_process_id()
{
    auto hwnd = GetForegroundWindow();
    if (!hwnd)
        return 0;

    DWORD pid;
    if (GetWindowThreadProcessId(hwnd, &pid) == 0)
        return 0;

    return pid;
}

std::string get_executable_name(DWORD process_id)
{
    struct handle_t {
        handle_t(HANDLE handle):
            _handle(handle) {}
        ~handle_t() {
            CloseHandle(_handle);
        }
        operator HANDLE() const { return _handle; }
        operator bool() const { return _handle != 0; }
        HANDLE _handle;
    }
    process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, process_id);
    if (!process)
        return "";

    char buf[MAX_PATH];
    if (GetModuleFileNameExA(process, 0, buf, ARRAYSIZE(buf)) == MAX_PATH)
        return "";

    // Though there should be at least one '\\' in buf from windows path format specification.
    auto p = strrchr(buf, '\\');
    if (p == 0)
        return "";

    return p + 1;
}


void gp_handle_analogue_input(const stick_params_t& config, const XINPUT_GAMEPAD& input)
{
    left_stick(config.cursor, input);
    right_stick(config.scroll, input);
}

keystate_t g_prev_keys = {};

keystate_t translate_input(WORD input)
{
#ifdef _DEBUG
    auto log = get_logger();
#endif
    key_binding_t v{.buttons=input};
    auto [lb, ub] = std::equal_range(g_key_bindings.begin(), g_key_bindings.end(), v);
    
    auto N = __popcnt16(input);

    keystate_t keys = {};
    if (lb == ub) { // the combination of buttons is not defined
        // 個々のボタンのキーバインディングを組み合わせる
        DWORD i, mask = input;
        while (BitScanForward(&i, mask)) {
            auto& b = g_single_button[i];
            b.fill(keys);
            mask ^= (1ul << i);
        }
    }
    //else if (lb->process.empty()) { // input にアプリケーション固有のバインディングはない        
    else if (lb->executable == 0) { // input にアプリケーション固有のバインディングはない        
        lb->fill(keys);
    }
    else {
        auto pid = get_process_id_under_cursor();
        auto cursor_process = get_executable_name(pid);

        pid = get_foreground_process_id();
        auto foreground_process = get_executable_name(pid);

#ifdef _DEBUG
        log->info("finding custom rule");
        log->info("cursor: \"{}\", foreground: \"{}\", input: {:04X}", cursor_process, foreground_process, input);
#endif

        for (auto i = lb; i != ub; ++i) {
#if defined(_DEBUG) // && _MSC_VER == ??
            if (i->executable == 0)
                log->info("No custom rule matched");
            else
                log->info("Testing \"{}\"", *(std::string*)((char*)i->executable + 48));
#endif
            auto& process = i->foreground_window() ? foreground_process : cursor_process;
            if (i->executable != 0 && !std::regex_match(process, *i->executable))
                continue;
            i->fill(keys);
            break;
        }
    }
    return keys;
}

int keycount(const keystate_t& current, const keystate_t& prev)
{
    int N = 0;
    for (int i = 0; i < 4; ++i) {
        N += __popcnt64(~current.keys[i] & prev.keys[i]);
        N += __popcnt64(current.keys[i] & ~prev.keys[i]);
    }
    return N;
}

void make_mouse_button_input(INPUT& i, uint8_t vk, bool up)
{
    memset(&i, 0, sizeof(INPUT));
    i.type = INPUT_MOUSE;

    if (vk == VK_XBUTTON1 || vk == VK_XBUTTON2) {
        i.mi.mouseData = vk - VK_XBUTTON1 + 1;
        i.mi.dwFlags = up ? MOUSEEVENTF_XUP : MOUSEEVENTF_XDOWN;
    }
    else {
        auto button = vk == VK_LBUTTON ? mouse_button_t::left :
                        vk == VK_RBUTTON ? mouse_button_t::right :
                        vk == VK_MBUTTON ? mouse_button_t::middle : mouse_button_t::invalid;
        assert(button != mouse_button_t::invalid);

        i.mi.dwFlags = up ? ((uint32_t)button << 1) : (uint32_t)button;
    }
}

void make_kbd_input(INPUT& i, uint8_t vk, bool up) {
    auto logger = get_logger();
    logger->debug("{:<20} {}", vk_name(vk), up ? "Up" : "Down");

    memset(&i, 0, sizeof(INPUT));
    i.type = INPUT_KEYBOARD;
    i.ki.wVk = vk;
    i.ki.wScan = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    if (up)
        i.ki.dwFlags = KEYEVENTF_KEYUP;
    if (is_extended_key(vk))
        i.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
}

void gp_handle_buttons_input(WORD buttons, keystate_t& state)
{
    auto logger = get_logger();
    logger->debug("buttons: {:04X}", buttons);

    auto input = translate_input(buttons);
    auto N = keycount(input, state);
    if (N == 0)
        return;

    logger->debug("number of inputs: {}", N);
    std::vector<INPUT> inputs(N);
    int n = 0;
    DWORD k;

    // TODO: SHIFT が押しっぱなしでも up down されている
    logger->debug("------ buttons -------");
    logger->debug("     {:<16} {:<16} {:<16} {:<16}", "input", "state", "up", "down");
    logger->debug("[00] {:016X} {:016X} {:016X} {:016X}", input.keys[0], state.keys[0], ~input.keys[0] & state.keys[0], input.keys[0] & ~state.keys[0]);
    logger->debug("[40] {:016X} {:016X} {:016X} {:016X}", input.keys[1], state.keys[1], ~input.keys[1] & state.keys[1], input.keys[1] & ~state.keys[1]);
    logger->debug("[80] {:016X} {:016X} {:016X} {:016X}", input.keys[2], state.keys[2], ~input.keys[2] & state.keys[2], input.keys[2] & ~state.keys[2]);
    logger->debug("[C0] {:016X} {:016X} {:016X} {:016X}", input.keys[3], state.keys[3], ~input.keys[3] & state.keys[3], input.keys[3] & ~state.keys[3]);
    logger->debug("---------------------");

    // mouse up
    for (int i = 0; i < 4; ++i) {
        auto r = (~input.keys[i] & state.keys[i]) & MOUSE_EVENTS_MASK[i];
        logger->debug("mouse up: {:016X} = {:016X} & {:016X} & {:016X}", r, ~input.keys[i], state.keys[i], MOUSE_EVENTS_MASK[i]);
        while (BitScanForward64(&k, r)) {
            uint8_t vk = 64 * i + k;
            make_mouse_button_input(inputs[n++], vk, true);
            r ^= (1ull << k);
        }
    }
    // non-modifier key up
    for (int i = 0; i < 4; ++i) {
        auto r = (~input.keys[i] & state.keys[i]) & keys_mask(i);
        logger->debug("non-modifier up: {:016X} = {:016X} & {:016X} & {:016X}", r, ~input.keys[i], state.keys[i], keys_mask(i));
        while (BitScanForward64(&k, r)) {
            uint8_t vk = 64 * i + k;
            make_kbd_input(inputs[n++], vk, true);
            r ^= (1ull << k);
        }
    }
    // modifier key up
    for (int i = 0; i < 4; ++i) {
        auto r = (~input.keys[i] & state.keys[i]) & MODIFIERS_MASK[i];
        logger->debug("modifier up: {:016X} = {:016X} & {:016X} & {:016X}", r, ~input.keys[i], state.keys[i], MODIFIERS_MASK[i]);
        while (BitScanForward64(&k, r)) {
            uint8_t vk = 64 * i + k;
            make_kbd_input(inputs[n++], vk, true);
            r ^= (1ull << k);
        }
    }
    // modifier key down
    for (int i = 0; i < 4; ++i) {
        auto r = (input.keys[i] & ~state.keys[i]) & MODIFIERS_MASK[i];
        logger->debug("modifier down: {:016X}", r);
        while (BitScanForward64(&k, r)) {
            uint8_t vk = 64 * i + k;
            make_kbd_input(inputs[n++], vk, false);
            r ^= (1ull << k);
        }
    }
    // non-modifier key down
    for (int i = 0; i < 4; ++i) {
        auto r = (input.keys[i] & ~state.keys[i]) & keys_mask(i);
        while (BitScanForward64(&k, r)) {
            uint8_t vk = 64 * i + k;
            make_kbd_input(inputs[n++], vk, false);
            r ^= (1ull << k);
        }
    }
    // mouse down
    for (int i = 0; i < 4; ++i) {
        auto r = (input.keys[i] & ~state.keys[i]) & MOUSE_EVENTS_MASK[i];
        while (BitScanForward64(&k, r)) {
            uint8_t vk = 64 * i + k;
            make_mouse_button_input(inputs[n++], vk, false);
            r ^= (1ull << k);
        }
    }
    send_input(inputs);

    state = input;
}

void repeat_keys(const keystate_t& state)
{
    // TODO: repeatable キーはアプリケーションによって変えた方が良い。
    //       アプリケーション側で勝手にリピートすることがあるため。
    int N = 0;
    for (int i = 0, n = 0; i < std::size(state.keys); ++i) {
        auto bits = state.keys[i] & repeatable_keys(i);
        N += __popcnt64(bits);
    }
    std::vector<INPUT> inputs(N);

    for (int i = 0, n = 0; i < std::size(state.keys); ++i) {
        auto bits = state.keys[i] & repeatable_keys(i);
        DWORD k;
        while (BitScanForward64(&k, bits)) {
            uint8_t vk = 64 * i + k;
            make_kbd_input(inputs[n++], vk, false);
            bits ^= (1ull << k);
        }
    }
    send_input(inputs);
}

void handle_xinput(uint32_t* pstatus, concurrent_queue<xinput_t>* _queue)
{
    const uint32_t handle_interval = 1000/16;
    const uint64_t repeat_mask = 0x1;

    auto status = *pstatus;
    auto& queue = *_queue;

    xinput_t input;
    keystate_t prev[XUSER_MAX_COUNT];

    if (!InitializeTouchInjection(2, TOUCH_FEEDBACK_DEFAULT))
        return; // TODO:

    try {
        for (uint64_t n = 0;; ++n) {
            if (WaitOnAddress(pstatus, &status, sizeof(uint32_t), handle_interval)) {
                if (status != *pstatus)
                    break;
            }

            if (queue.try_pop(input)) {
                n = 0;
                do {
                    gp_handle_buttons_input(input.buttons, prev[input.device]);
                }
                while (queue.try_pop(input));
            }
            else if ((n & repeat_mask) == 0) {
                for (auto& state: prev)
                    if (!state.empty())
                        repeat_keys(state);
            }
        }
    }
    catch (std::exception& exc) {
        auto log = get_logger();
        log->error("exception in handle thread: {}", exc.what());
        exit(1);
    }
    catch (...) {
        auto log = get_logger();
        log->error("unknown exception in handle thread");
        exit(100);
    }
    auto log = get_logger();
    log->info("Exit handler thread");
}

void check_xinput(uint32_t* pstatus, concurrent_queue<xinput_t>* _queue)
{
    DWORD status = *pstatus;
    DWORD packet_numbers[XUSER_MAX_COUNT] = {};
    auto& queue = *_queue;

    XINPUT_STATE input;
    for (;;) {
        if (WaitOnAddress(pstatus, &status, sizeof(uint32_t), 8)) {
            if (status != *pstatus)
                break;
        }

        auto timestamp = GetTickCount();

        for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
            auto& s_params = g_stick_params[i];

            auto err = XInputGetState(i, &input);
            if (err == ERROR_SUCCESS) {
                auto& in = input.Gamepad;

                if (!s_params.initialized || in.wButtons == (XINPUT_GAMEPAD_START|XINPUT_GAMEPAD_BACK))
                    calibrate_stick_params(s_params, input.Gamepad);
                gp_handle_analogue_input(s_params, in);

                if (packet_numbers[i] != input.dwPacketNumber) {
                    packet_numbers[i] = input.dwPacketNumber;
                    xinput_t item{ i, timestamp, in.wButtons };
                    queue.push(item);
                }
            }
            else if (s_params.initialized) {
                s_params.initialized = false;
                packet_numbers[i] = 0;
            }
        }
    }
}

bool xinput_initialize()
{
#ifdef ENABLE_GUIDE_BUTTON
    xinput_dll = LoadLibraryW(L"xinput1_4.dll");
    if (xinput_dll == 0)
        return false;
    XInputGetStateEx = (DWORD (WINAPI*)(DWORD, XINPUT_STATE*)) GetProcAddress(xinput_dll, (char*)100);
    return XInputGetStateEx != 0;
#else
    return true;
#endif
}

bool xinput_finalize()
{
#ifdef ENABLE_GUIDE_BUTTON
    return FreeLibrary(xinput_dll);
#else
    return true;
#endif
}
