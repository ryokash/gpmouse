#include <windows.h>
#include <xinput.h>
#include <stdint.h>

#include <iterator>
#include <vector>
#include <cassert>

#include <spdlog/sinks/rotating_file_sink.h>


#include "config.h"


#define XINPUT_GAMEPAD_GUIDE 0x0400


namespace gpmouse
{

struct virtual_key_code_t
{
	const char* name;
	uint8_t value;
};

constexpr virtual_key_code_t virtual_key_codes[256] = {
	{ "", 0 },
	{ "VK_LBUTTON", VK_LBUTTON },
	{ "VK_RBUTTON", VK_RBUTTON },
	{ "VK_CANCEL", VK_CANCEL },
	{ "VK_MBUTTON", VK_MBUTTON },
	{ "VK_XBUTTON1", VK_XBUTTON1 },
	{ "VK_XBUTTON2", VK_XBUTTON2 },
	// 0x07 : reserved
	{ "", 0x07 },
	{ "VK_BACK", VK_BACK },
	{ "VK_TAB", VK_TAB },
	// 0x0A - 0x0B : reserved
	{ "", 0x0A },
	{ "", 0x0B },
	{ "VK_CLEAR", VK_CLEAR },
	{ "VK_RETURN", VK_RETURN },
	// 0x0E - 0x0F : unassigned
	{ "", 0x0E },
	{ "", 0x0F },
	{ "VK_SHIFT", VK_SHIFT },
	{ "VK_CONTROL", VK_CONTROL },
	{ "VK_MENU", VK_MENU },
	{ "VK_PAUSE", VK_PAUSE },
	{ "VK_CAPITAL", VK_CAPITAL },
	{ "VK_KANA", VK_KANA },
	{ "VK_IME_ON", VK_IME_ON },
	{ "VK_JUNJA", VK_JUNJA },
	{ "VK_FINAL", VK_FINAL },
	{ "VK_KANJI", VK_KANJI },
	{ "VK_IME_OFF", VK_IME_OFF },
	{ "VK_ESCAPE", VK_ESCAPE },
	{ "VK_CONVERT", VK_CONVERT },
	{ "VK_NONCONVERT", VK_NONCONVERT },
	{ "VK_ACCEPT", VK_ACCEPT },
	{ "VK_MODECHANGE", VK_MODECHANGE },
	{ "VK_SPACE", VK_SPACE },
	{ "VK_PRIOR", VK_PRIOR },
	{ "VK_NEXT", VK_NEXT },
	{ "VK_END", VK_END },
	{ "VK_HOME", VK_HOME },
	{ "VK_LEFT", VK_LEFT },
	{ "VK_UP", VK_UP },
	{ "VK_RIGHT", VK_RIGHT },
	{ "VK_DOWN", VK_DOWN },
	{ "VK_SELECT", VK_SELECT },
	{ "VK_PRINT", VK_PRINT },
	{ "VK_EXECUTE", VK_EXECUTE },
	{ "VK_SNAPSHOT", VK_SNAPSHOT },
	{ "VK_INSERT", VK_INSERT },
	{ "VK_DELETE", VK_DELETE },
	{ "VK_HELP", VK_HELP },
	{ "VK_0", 0x30 },
	{ "VK_1", 0x31 },
	{ "VK_2", 0x32 },
	{ "VK_3", 0x33 },
	{ "VK_4", 0x34 },
	{ "VK_5", 0x35 },
	{ "VK_6", 0x36 },
	{ "VK_7", 0x37 },
	{ "VK_8", 0x38 },
	{ "VK_9", 0x39 },
	// 0x3A - 0x40 : unassigned
	{ "", 0x3A },
	{ "", 0x3B },
	{ "", 0x3C },
	{ "", 0x3D },
	{ "", 0x3E },
	{ "", 0x3F },
	{ "", 0x40 },
	{ "VK_A", 0x41 },
	{ "VK_B", 0x42 },
	{ "VK_C", 0x43 },
	{ "VK_D", 0x44 },
	{ "VK_E", 0x45 },
	{ "VK_F", 0x46 },
	{ "VK_G", 0x47 },
	{ "VK_H", 0x48 },
	{ "VK_I", 0x49 },
	{ "VK_J", 0x4A },
	{ "VK_K", 0x4B },
	{ "VK_L", 0x4C },
	{ "VK_M", 0x4D },
	{ "VK_N", 0x4E },
	{ "VK_O", 0x4F },
	{ "VK_P", 0x50 },
	{ "VK_Q", 0x51 },
	{ "VK_R", 0x52 },
	{ "VK_S", 0x53 },
	{ "VK_T", 0x54 },
	{ "VK_U", 0x55 },
	{ "VK_V", 0x56 },
	{ "VK_W", 0x57 },
	{ "VK_X", 0x58 },
	{ "VK_Y", 0x59 },
	{ "VK_Z", 0x5A },
	{ "VK_LWIN", VK_LWIN },
	{ "VK_RWIN", VK_RWIN },
	{ "VK_APPS", VK_APPS },
	// 0x5E : reserved
	{ "", 0x5E },
	{ "VK_SLEEP", VK_SLEEP },
	{ "VK_NUMPAD0", VK_NUMPAD0 },
	{ "VK_NUMPAD1", VK_NUMPAD1 },
	{ "VK_NUMPAD2", VK_NUMPAD2 },
	{ "VK_NUMPAD3", VK_NUMPAD3 },
	{ "VK_NUMPAD4", VK_NUMPAD4 },
	{ "VK_NUMPAD5", VK_NUMPAD5 },
	{ "VK_NUMPAD6", VK_NUMPAD6 },
	{ "VK_NUMPAD7", VK_NUMPAD7 },
	{ "VK_NUMPAD8", VK_NUMPAD8 },
	{ "VK_NUMPAD9", VK_NUMPAD9 },
	{ "VK_MULTIPLY", VK_MULTIPLY },
	{ "VK_ADD", VK_ADD },
	{ "VK_SEPARATOR", VK_SEPARATOR },
	{ "VK_SUBTRACT", VK_SUBTRACT },
	{ "VK_DECIMAL", VK_DECIMAL },
	{ "VK_DIVIDE", VK_DIVIDE },
	{ "VK_F1", VK_F1 },
	{ "VK_F2", VK_F2 },
	{ "VK_F3", VK_F3 },
	{ "VK_F4", VK_F4 },
	{ "VK_F5", VK_F5 },
	{ "VK_F6", VK_F6 },
	{ "VK_F7", VK_F7 },
	{ "VK_F8", VK_F8 },
	{ "VK_F9", VK_F9 },
	{ "VK_F10", VK_F10 },
	{ "VK_F11", VK_F11 },
	{ "VK_F12", VK_F12 },
	{ "VK_F13", VK_F13 },
	{ "VK_F14", VK_F14 },
	{ "VK_F15", VK_F15 },
	{ "VK_F16", VK_F16 },
	{ "VK_F17", VK_F17 },
	{ "VK_F18", VK_F18 },
	{ "VK_F19", VK_F19 },
	{ "VK_F20", VK_F20 },
	{ "VK_F21", VK_F21 },
	{ "VK_F22", VK_F22 },
	{ "VK_F23", VK_F23 },
	{ "VK_F24", VK_F24 },
	// #if(_WIN32_WINNT >= 0x0604)
	// 0x88 - 0x8F : UI navigation
	{ "VK_NAVIGATION_VIEW", VK_NAVIGATION_VIEW },
	{ "VK_NAVIGATION_MENU", VK_NAVIGATION_MENU },
	{ "VK_NAVIGATION_UP", VK_NAVIGATION_UP },
	{ "VK_NAVIGATION_DOWN", VK_NAVIGATION_DOWN },
	{ "VK_NAVIGATION_LEFT", VK_NAVIGATION_LEFT },
	{ "VK_NAVIGATION_RIGHT", VK_NAVIGATION_RIGHT },
	{ "VK_NAVIGATION_ACCEPT", VK_NAVIGATION_ACCEPT },
	{ "VK_NAVIGATION_CANCEL", VK_NAVIGATION_CANCEL },
	// #endif /* _WIN32_WINNT >= 0x0604 */
	{ "VK_NUMLOCK", VK_NUMLOCK },
	{ "VK_SCROLL", VK_SCROLL },
	//  Fujitsu/OASYS kbd definitions
	{ "VK_OEM_FJ_JISHO", VK_OEM_FJ_JISHO },
	{ "VK_OEM_FJ_MASSHOU", VK_OEM_FJ_MASSHOU },
	{ "VK_OEM_FJ_TOUROKU", VK_OEM_FJ_TOUROKU },
	{ "VK_OEM_FJ_LOYA", VK_OEM_FJ_LOYA },
	{ "VK_OEM_FJ_ROYA", VK_OEM_FJ_ROYA },
	// 0x97 - 0x9F : unassigned
	{ "", 0x97 },
	{ "", 0x98 },
	{ "", 0x99 },
	{ "", 0x9A },
	{ "", 0x9B },
	{ "", 0x9C },
	{ "", 0x9D },
	{ "", 0x9E },
	{ "", 0x9F },
	{ "VK_LSHIFT", VK_LSHIFT },
	{ "VK_RSHIFT", VK_RSHIFT },
	{ "VK_LCONTROL", VK_LCONTROL },
	{ "VK_RCONTROL", VK_RCONTROL },
	{ "VK_LMENU", VK_LMENU },
	{ "VK_RMENU", VK_RMENU },
	// #if(_WIN32_WINNT >= 0x0500)
	{ "VK_BROWSER_BACK", VK_BROWSER_BACK },
	{ "VK_BROWSER_FORWARD", VK_BROWSER_FORWARD },
	{ "VK_BROWSER_REFRESH", VK_BROWSER_REFRESH },
	{ "VK_BROWSER_STOP", VK_BROWSER_STOP },
	{ "VK_BROWSER_SEARCH", VK_BROWSER_SEARCH },
	{ "VK_BROWSER_FAVORITES", VK_BROWSER_FAVORITES },
	{ "VK_BROWSER_HOME", VK_BROWSER_HOME },
	{ "VK_VOLUME_MUTE", VK_VOLUME_MUTE },
	{ "VK_VOLUME_DOWN", VK_VOLUME_DOWN },
	{ "VK_VOLUME_UP", VK_VOLUME_UP },
	{ "VK_MEDIA_NEXT_TRACK", VK_MEDIA_NEXT_TRACK },
	{ "VK_MEDIA_PREV_TRACK", VK_MEDIA_PREV_TRACK },
	{ "VK_MEDIA_STOP", VK_MEDIA_STOP },
	{ "VK_MEDIA_PLAY_PAUSE", VK_MEDIA_PLAY_PAUSE },
	{ "VK_LAUNCH_MAIL", VK_LAUNCH_MAIL },
	{ "VK_LAUNCH_MEDIA_SELECT", VK_LAUNCH_MEDIA_SELECT },
	{ "VK_LAUNCH_APP1", VK_LAUNCH_APP1 },
	{ "VK_LAUNCH_APP2", VK_LAUNCH_APP2 },
	// #endif /* _WIN32_WINNT >= 0x0500 */
	// 0xB8 - 0xB9 : reserved
	{ "", 0xB8 },
	{ "", 0xB9 },
	{ "VK_OEM_1", VK_OEM_1 },
	{ "VK_OEM_PLUS", VK_OEM_PLUS },
	{ "VK_OEM_COMMA", VK_OEM_COMMA },
	{ "VK_OEM_MINUS", VK_OEM_MINUS },
	{ "VK_OEM_PERIOD", VK_OEM_PERIOD },
	{ "VK_OEM_2", VK_OEM_2 },
	{ "VK_OEM_3", VK_OEM_3 },
	// 0xC1 - 0xC2 : reserved
	{ "", 0xC1 },
	{ "", 0xC2 },
	// #if(_WIN32_WINNT >= 0x0604)
	// 0xC3 - 0xDA : Gamepad input
	{ "VK_GAMEPAD_A", VK_GAMEPAD_A },
	{ "VK_GAMEPAD_B", VK_GAMEPAD_B },
	{ "VK_GAMEPAD_X", VK_GAMEPAD_X },
	{ "VK_GAMEPAD_Y", VK_GAMEPAD_Y },
	{ "VK_GAMEPAD_RIGHT_SHOULDER", VK_GAMEPAD_RIGHT_SHOULDER },
	{ "VK_GAMEPAD_LEFT_SHOULDER", VK_GAMEPAD_LEFT_SHOULDER },
	{ "VK_GAMEPAD_LEFT_TRIGGER", VK_GAMEPAD_LEFT_TRIGGER },
	{ "VK_GAMEPAD_RIGHT_TRIGGER", VK_GAMEPAD_RIGHT_TRIGGER },
	{ "VK_GAMEPAD_DPAD_UP", VK_GAMEPAD_DPAD_UP },
	{ "VK_GAMEPAD_DPAD_DOWN", VK_GAMEPAD_DPAD_DOWN },
	{ "VK_GAMEPAD_DPAD_LEFT", VK_GAMEPAD_DPAD_LEFT },
	{ "VK_GAMEPAD_DPAD_RIGHT", VK_GAMEPAD_DPAD_RIGHT },
	{ "VK_GAMEPAD_MENU", VK_GAMEPAD_MENU },
	{ "VK_GAMEPAD_VIEW", VK_GAMEPAD_VIEW },
	{ "VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON", VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON },
	{ "VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON", VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON },
	{ "VK_GAMEPAD_LEFT_THUMBSTICK_UP", VK_GAMEPAD_LEFT_THUMBSTICK_UP },
	{ "VK_GAMEPAD_LEFT_THUMBSTICK_DOWN", VK_GAMEPAD_LEFT_THUMBSTICK_DOWN },
	{ "VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT", VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT },
	{ "VK_GAMEPAD_LEFT_THUMBSTICK_LEFT", VK_GAMEPAD_LEFT_THUMBSTICK_LEFT },
	{ "VK_GAMEPAD_RIGHT_THUMBSTICK_UP", VK_GAMEPAD_RIGHT_THUMBSTICK_UP },
	{ "VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN", VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN },
	{ "VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT", VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT },
	{ "VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT", VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT },
	// #endif /* _WIN32_WINNT >= 0x0604 */
	{ "VK_OEM_4", VK_OEM_4 },
	{ "VK_OEM_5", VK_OEM_5 },
	{ "VK_OEM_6", VK_OEM_6 },
	{ "VK_OEM_7", VK_OEM_7 },
	{ "VK_OEM_8", VK_OEM_8 },
	// 0xE0 : reserved
	{ "", 0xE0 },
	// Various extended or enhanced keyboards
	{ "VK_OEM_AX", VK_OEM_AX },
	{ "VK_OEM_102", VK_OEM_102 },
	{ "VK_ICO_HELP", VK_ICO_HELP },
	{ "VK_ICO_00", VK_ICO_00 },
	// #if(WINVER >= 0x0400)
	{ "VK_PROCESSKEY", VK_PROCESSKEY },
	// #endif /* WINVER >= 0x0400 */
	{ "VK_ICO_CLEAR", VK_ICO_CLEAR },
	// #if(_WIN32_WINNT >= 0x0500)
	{ "VK_PACKET", VK_PACKET },
	// #endif /* _WIN32_WINNT >= 0x0500 */
	// 0xE8 : unassigned
	{ "", 0xE8 },
	// Nokia/Ericsson definitions
	{ "VK_OEM_RESET", VK_OEM_RESET },
	{ "VK_OEM_JUMP", VK_OEM_JUMP },
	{ "VK_OEM_PA1", VK_OEM_PA1 },
	{ "VK_OEM_PA2", VK_OEM_PA2 },
	{ "VK_OEM_PA3", VK_OEM_PA3 },
	{ "VK_OEM_WSCTRL", VK_OEM_WSCTRL },
	{ "VK_OEM_CUSEL", VK_OEM_CUSEL },
	{ "VK_OEM_ATTN", VK_OEM_ATTN },
	{ "VK_OEM_FINISH", VK_OEM_FINISH },
	{ "VK_OEM_COPY", VK_OEM_COPY },
	{ "VK_OEM_AUTO", VK_OEM_AUTO },
	{ "VK_OEM_ENLW", VK_OEM_ENLW },
	{ "VK_OEM_BACKTAB", VK_OEM_BACKTAB },
	{ "VK_ATTN", VK_ATTN },
	{ "VK_CRSEL", VK_CRSEL },
	{ "VK_EXSEL", VK_EXSEL },
	{ "VK_EREOF", VK_EREOF },
	{ "VK_PLAY", VK_PLAY },
	{ "VK_ZOOM", VK_ZOOM },
	{ "VK_NONAME", VK_NONAME },
	{ "VK_PA1", VK_PA1 },
	{ "VK_OEM_CLEAR", VK_OEM_CLEAR },
	{ "", 0xFF },
};

const char* vk_name(uint8_t vk)
{
	return virtual_key_codes[vk].name;
}

std::vector<key_binding_t> g_key_bindings;
key_binding_t g_single_button[16];
std::vector<std::regex> g_apps;

void configure()
{
	using namespace std::regex_constants;

	auto num_keys = std::size(virtual_key_codes);
	assert(num_keys == 256);
	auto sz = sizeof(key_binding_t);
	auto sbsz = sizeof(g_single_button);
	assert(sz * std::size(g_single_button) == sbsz);

	modifiers_t alt = {}, shift = {}, ctrl = {};
	alt.alt = 1;
	shift.shift = 1;
	ctrl.ctrl = 1;

	key_binding_t single_button[16] = {
		{.buttons = XINPUT_GAMEPAD_DPAD_UP,     .keys = { VK_UP, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_DPAD_DOWN,   .keys = { VK_DOWN, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_DPAD_LEFT,   .keys = { VK_LEFT, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_DPAD_RIGHT,  .keys = { VK_RIGHT, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_START,       .modifiers = alt, .keys = { 0, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_BACK,        .keys = { VK_BACK, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_LEFT_THUMB,  .keys = { 0, 0, 0, 0 } }, // TODO: start multi touch
		{.buttons = XINPUT_GAMEPAD_RIGHT_THUMB, .keys = { 0, 0, 0, 0 } }, // TODO: end multi touch
		{.buttons = XINPUT_GAMEPAD_LEFT_SHOULDER,	.modifiers = ctrl,  .keys = { 0, 0, 0, 0 } }, // use as modifier
		{.buttons = XINPUT_GAMEPAD_RIGHT_SHOULDER,  .modifiers = shift, .keys = { 0, 0, 0, 0 } }, // use as modifier
		{.buttons = XINPUT_GAMEPAD_GUIDE, },
		{},
		{.buttons = XINPUT_GAMEPAD_A, .keys = { VK_ESCAPE, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_B, .keys = { VK_RBUTTON, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_X, .keys = { VK_LBUTTON, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_Y, .keys = { VK_MBUTTON, 0, 0, 0 } },
	};
	std::copy(single_button, single_button + 16, g_single_button);

	g_apps.emplace_back("^(HmFilerCLassic|firefox|msedge|chrome|vivaldi)\\.exe$", ECMAScript|icase);
	g_apps.emplace_back("^(firefox|msedge|chrome|vivaldi)\\.exe$", ECMAScript|icase);
	g_apps.emplace_back("^vivaldi\\.exe$", ECMAScript|icase);
	g_apps.emplace_back("^HmFilerClassic\\.exe$", ECMAScript|icase);
	g_apps.emplace_back("^HD-Player\\.exe$", ECMAScript|icase);

	auto& tabapps = g_apps[0];
	auto& browser = g_apps[1];
	auto& vivaldi = g_apps[2];
	auto& hmfiler = g_apps[3];
	auto& bluestacks = g_apps[4];

	key_binding_t k;

	k = {};
	k.buttons = XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_DPAD_RIGHT;
	k.modifiers.alt = 1;
	k.keys[0] = VK_TAB;
	g_key_bindings.push_back(k);

	// TODO: ボタンを放したときに Left Shoulder が alt になってしまって、メニューが開いてしまう。
	k = {};
	k.process = &tabapps;
	k.buttons = XINPUT_GAMEPAD_LEFT_SHOULDER|XINPUT_GAMEPAD_A;
	k.modifiers.ctrl = 1;
	k.keys[0] = VK_F4;
	g_key_bindings.push_back(k);

	// TODO: ボタンを放したときに Left Shoulder が alt になってしまって、メニューが開いてしまう。
	k = {};
	k.process = &tabapps;
	k.buttons = XINPUT_GAMEPAD_LEFT_SHOULDER|XINPUT_GAMEPAD_X;
	k.modifiers.ctrl = 1;
	k.keys[0] = VK_LBUTTON;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &tabapps;
	k.buttons = XINPUT_GAMEPAD_RIGHT_SHOULDER|XINPUT_GAMEPAD_DPAD_UP;
	k.modifiers.alt = 1;
	k.keys[0] = VK_RIGHT;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &tabapps;
	k.buttons = XINPUT_GAMEPAD_RIGHT_SHOULDER|XINPUT_GAMEPAD_DPAD_DOWN;
	k.modifiers.alt = 1;
	k.keys[0] = VK_LEFT;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &tabapps;
	k.buttons = XINPUT_GAMEPAD_RIGHT_SHOULDER|XINPUT_GAMEPAD_DPAD_LEFT;
	k.modifiers.ctrl = 1;
	k.modifiers.shift = 1;
	k.keys[0] = VK_TAB;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &tabapps;
	k.buttons = XINPUT_GAMEPAD_RIGHT_SHOULDER|XINPUT_GAMEPAD_DPAD_RIGHT;
	k.modifiers.ctrl = 1;
	k.keys[0] = VK_TAB;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &tabapps;
	k.buttons = XINPUT_GAMEPAD_LEFT_SHOULDER|XINPUT_GAMEPAD_A;
	k.modifiers.ctrl = 1;
	k.keys[0] = VK_F4;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &browser;
	k.buttons = XINPUT_GAMEPAD_A;
	k.modifiers.alt = 1;
	k.keys[0] = VK_LEFT;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &browser;
	k.buttons = XINPUT_GAMEPAD_RIGHT_SHOULDER|XINPUT_GAMEPAD_A;
	k.keys[0] = VK_ESCAPE;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &browser;
	k.buttons = XINPUT_GAMEPAD_LEFT_SHOULDER|XINPUT_GAMEPAD_RIGHT_SHOULDER|XINPUT_GAMEPAD_A;
	k.modifiers.ctrl = 1;
	k.modifiers.shift = 1;
	k.keys[0] = (uint8_t)'T';
	g_key_bindings.push_back(k);

	k = {};
	k.process = &bluestacks;
	k.buttons = XINPUT_GAMEPAD_BACK;
	k.keys[0] = VK_F11;
	g_key_bindings.push_back(k);

	k = {};
	k.process = &bluestacks;
	k.buttons = XINPUT_GAMEPAD_RIGHT_SHOULDER|XINPUT_GAMEPAD_DPAD_DOWN;
	k.keys[0] = VK_F12;
	g_key_bindings.push_back(k);

	for (auto& sb: g_single_button)
		if (sb.buttons != 0)
			g_key_bindings.push_back(sb);

	std::sort(
		g_key_bindings.begin(),
		g_key_bindings.end(),
		[](auto& a, auto& b){
			return a.buttons < b.buttons ||
				a.buttons == b.buttons && a.process > b.process;
		}
	);
}

std::shared_ptr<spdlog::logger> get_logger()
{
	auto max_size = 1048576 * 4;
	auto max_files = 10;
	
	static auto logger = spdlog::rotating_logger_mt("gpmouse", "logs/gpmouse.log", max_size, max_files);
#ifdef _DEBUG
	logger->set_level(spdlog::level::level_enum::debug);
#else
	logger->set_level(spdlog::level::level_enum::info);
#endif

	return logger;
}

} // namespace gpmouse
