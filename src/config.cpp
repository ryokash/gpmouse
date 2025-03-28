#include <windows.h>
#include <xinput.h>
#include <stdint.h>

#include <iterator>
#include <vector>
#include <filesystem>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

#define TOML_TOML11
#ifdef TOML_TOML11
	#include <toml.hpp>
#else
	#include <toml++/toml.hpp>
#endif // def TOML_TOML11

#include <magic_enum.hpp>
#include <spdlog/sinks/rotating_file_sink.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "config.h"


#define XINPUT_GAMEPAD_GUIDE 0x0400
#define MAX_PATH_LENGTH 32767


namespace gpmouse
{

struct virtual_key_code_t
{
	const char* name;
	uint8_t value;
};

constexpr virtual_key_code_t virtual_key_codes[] = {
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
	// duplicate items
	{ "VK_HANGEUL", VK_HANGEUL },
	{ "VK_HANGUL", VK_HANGUL },
	{ "VK_HANJA", VK_HANJA },
	//
	{ "VK_PAGEUP", VK_PRIOR },
	{ "VK_PAGEDOWN", VK_NEXT },
};

const char* vk_name(uint8_t vk)
{
	return virtual_key_codes[vk].name;
}

std::vector<key_binding_t> g_key_bindings;
key_binding_t g_single_button[16];
stick_params_t g_stick_params[XUSER_MAX_COUNT];

//std::vector<std::regex> g_apps;
struct app_t {
	std::string name;
	uint8_t priority;
	std::regex pattern;
};
std::map<std::string, app_t> g_apps;

std::wstring application_directory()
{
	std::array<wchar_t, MAX_PATH_LENGTH + 1> buf;
	auto len = GetModuleFileNameW(0, buf.data(), buf.size());
	if (len == buf.size())
		return L""; // TODO:

	for (auto p = buf.data() + len, head = buf.data(); p >= head; --p) {
		if (*p == L'\\')
			break;
		*p = L'\0';
	}

	return std::wstring(buf.data());
}

void default_config()
{
	constexpr uint8_t ctrl = key_binding_t::CONTROL;
	constexpr uint8_t alt = key_binding_t::ALT;
	constexpr uint8_t shift = key_binding_t::SHIFT;
	constexpr uint8_t win = key_binding_t::WINDOWS;

	key_binding_t single_button[16] = {
		{.buttons = XINPUT_GAMEPAD_DPAD_UP,     .keys = { VK_UP, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_DPAD_DOWN,   .keys = { VK_DOWN, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_DPAD_LEFT,   .keys = { VK_LEFT, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_DPAD_RIGHT,  .keys = { VK_RIGHT, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_START,       .modifiers = alt, .keys = { 0, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_BACK,        .keys = { VK_BACK, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_LEFT_THUMB,  .keys = { 0, 0, 0, 0 } }, // TODO: start multi touch
		{.buttons = XINPUT_GAMEPAD_RIGHT_THUMB, .keys = { 0, 0, 0, 0 } }, // TODO: end multi touch
		{.buttons = XINPUT_GAMEPAD_LEFT_SHOULDER,	.modifiers = ctrl, }, // use as modifier
		{.buttons = XINPUT_GAMEPAD_RIGHT_SHOULDER,  .modifiers = shift }, // use as modifier
		{.buttons = XINPUT_GAMEPAD_GUIDE,			.modifiers = win },
		{},
		{.buttons = XINPUT_GAMEPAD_A, .keys = { VK_ESCAPE, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_B, .keys = { VK_RBUTTON, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_X, .keys = { VK_LBUTTON, 0, 0, 0 } },
		{.buttons = XINPUT_GAMEPAD_Y, .keys = { VK_MBUTTON, 0, 0, 0 } },
	};
	std::copy(single_button, single_button + 16, g_single_button);
}

std::vector<std::string> split_string(const std::string& s, const std::string& delims = " ,&|")
{
	std::vector<std::string> ret;
	return boost::split(ret, s, boost::is_any_of(delims));
}

uint16_t parse_button(const std::string& s)
{
	static constexpr struct {
		const char* name;
		uint16_t value;
	}
	buttons[] = {
		{ "UP",				XINPUT_GAMEPAD_DPAD_UP },
		{ "DOWN",			XINPUT_GAMEPAD_DPAD_DOWN },
		{ "LEFT",			XINPUT_GAMEPAD_DPAD_LEFT },
		{ "RIGHT",			XINPUT_GAMEPAD_DPAD_RIGHT },
		{ "START",			XINPUT_GAMEPAD_START },
		{ "BACK",			XINPUT_GAMEPAD_BACK },
		{ "LT",				XINPUT_GAMEPAD_LEFT_THUMB },
		{ "LEFT_THUMB",		XINPUT_GAMEPAD_LEFT_THUMB },
		{ "RT",				XINPUT_GAMEPAD_RIGHT_THUMB },
		{ "RIGHT_THUMB",	XINPUT_GAMEPAD_RIGHT_THUMB },
		{ "LS",				XINPUT_GAMEPAD_LEFT_SHOULDER },
		{ "LEFT_SHOULDER",	XINPUT_GAMEPAD_LEFT_SHOULDER },
		{ "RS",				XINPUT_GAMEPAD_RIGHT_SHOULDER },
		{ "RIGHT_SHOULDER", XINPUT_GAMEPAD_RIGHT_SHOULDER },
		{ "GUIDE",			XINPUT_GAMEPAD_GUIDE },
		{ "A",				XINPUT_GAMEPAD_A },
		{ "B",				XINPUT_GAMEPAD_B },
		{ "X",				XINPUT_GAMEPAD_X },
		{ "Y",				XINPUT_GAMEPAD_Y },
	};

	auto us = boost::to_upper_copy(s);
	for (auto& b: buttons)
		if (us == b.name)
			return b.value;
	return 0;
}

uint8_t parse_vk_code(const std::string& s)
{
	auto us = boost::to_upper_copy(s);
	for (auto& vk: virtual_key_codes)
		if (us == vk.name)
			return vk.value;

	us = "VK_" + us;
	for (auto& vk: virtual_key_codes)
		if (us == vk.name)
			return vk.value;
	
	return 0;
}

#ifdef TOML_TOML11
template <typename TC, typename K>
float as_float(const toml::basic_value<TC>& v, const K& k, float default_value)
{
	if (v.is_empty())
		return default_value;

	try {
		return toml::find<float>(v, k);
	}
	catch (toml::type_error& exc) {
		return toml::find<int64_t>(v, k);
	}
	catch (std::out_of_range& exc) {
		return default_value;
	}
}

template <typename TC>
//stick_t load_stick_params(const toml::basic_value<TC>& v, uint32_t deadzone=2500,
//	acceleration_t accel_type=acceleration_t::exponential,
//	float base_speed=0.8, float accel_max=8, float deaccel_max=4, 
//	trigger_function_t lt=trigger_function_t::nop, 
//	trigger_function_t rt=trigger_function_t::nop)
stick_t load_stick_params(const toml::basic_value<TC>& v, const stick_t& defval=stick_t())
{
	namespace me = magic_enum;

	static const char* trigger_funcion_aliases[][2] = {
		{ "accel", "acceleration" },
		{ "deaccel", "deacceleration" },
	};

	stick_t c = defval;

	if (v.is_empty())
		return c;

	c.deadzone = toml::find_or<uint32_t>(v, "deadzone", defval.deadzone);
	c.accel_type = defval.accel_type;
	c.base_speed = as_float(v, "base_speed", defval.base_speed);
	c.accel_max = as_float(v, "accel_max", defval.accel_max);
	c.deaccel_max = as_float(v, "deaccel_max", defval.deaccel_max);

	try {
		auto lt = toml::find<std::string>(v, "left_trigger");
		for (auto pair: trigger_funcion_aliases)
			if (boost::iequals(lt, pair[0])) {
				lt = pair[1];
				break;
			}
		c.left_trigger = me::enum_cast<trigger_function_t>(lt, me::case_insensitive).value();
	}
	catch (std::out_of_range&) {
		c.left_trigger = defval.left_trigger;
	}

	try {
		auto rt = toml::find<std::string>(v, "right_trigger");
		for (auto pair: trigger_funcion_aliases)
			if (boost::iequals(rt, pair[0])) {
				rt = pair[1];
				break;
			}
		c.right_trigger = me::enum_cast<trigger_function_t>(rt, me::case_insensitive).value();
	}
	catch (std::out_of_range&) {
		c.left_trigger = defval.right_trigger;
	}

	return c;
}

void configure_input(const toml::value& cfg)
{
	using namespace std::regex_constants;

	g_apps.clear();

	auto accel = magic_enum::enum_cast<trigger_function_t>("accel");
	auto acceleration = magic_enum::enum_cast<trigger_function_t>("acceleration");

	auto cfg = toml::parse(cfg_file, toml::spec::v(1, 1, 0));

	auto cursor = toml::find_or_default<toml::value>(cfg, "cursor");
	g_stick_params[0].cursor
		= g_stick_params[1].cursor
		= g_stick_params[2].cursor
		= g_stick_params[3].cursor 
		= load_stick_params(cursor, { .deadzone=1000, .base_speed=0.3, });

	auto scroll = toml::find_or_default<toml::value>(cfg, "scroll");
	g_stick_params[0].scroll
		= g_stick_params[1].scroll
		= g_stick_params[2].scroll
		= g_stick_params[3].scroll
		= load_stick_params(scroll, { .deadzone = 5000, .base_speed = 0.01, .accel_max = 16, });

	memset(g_single_button, 0, sizeof(g_single_button));
	auto buttons = toml::find<std::vector<toml::value>>(cfg, "bindings", "buttons");
	for (auto& b: buttons) {
		auto button = parse_button(b["button"].as_string());
		
		DWORD i;
		if (!BitScanForward(&i, button))
			throw std::runtime_error("unknown button");

		auto& k = g_single_button[i];
		k.buttons = button;
		k.priority = USHRT_MAX;
		
		auto modifiers = b["modifiers"];
		if (modifiers.is_string()) {
			for (auto m: split_string(modifiers.as_string()))
				k.add_modifier(m);
		}
		else if (!modifiers.is_empty()) {
			for (auto m: modifiers.as_array())
				k.add_modifier(m.as_string());
		}

		auto keys = b["keys"];
		if (keys.is_string()) {
			auto ks = split_string(keys.as_string());
			if (ks.size() > std::size(k.keys))
				throw std::runtime_error("too many keys");
			for (int i = 0; i < ks.size(); ++i)
				k.keys[i] = parse_vk_code(ks[i]);
		}
		else if (!keys.is_empty()) {
			auto ks = keys.as_array();
			if (ks.size() > std::size(k.keys))
				throw std::runtime_error("too many keys");
			for (int i = 0; i < ks.size(); ++i)
				k.keys[i] = parse_vk_code(ks[i].as_string());
		}
	}

	auto apps_cfg = toml::find<std::vector<toml::value>>(cfg, "bindings", "applications");
	for (auto& app: apps_cfg) {
		auto name = app["name"].as_string();
		app_t a {
			name,
			toml::find_or(app, "priority", UCHAR_MAX - 1), 
			std::regex(app["pattern"].as_string(), ECMAScript|icase),
		};
		g_apps.emplace(name, std::move(a));
	}

	auto bindings = toml::find<std::vector<toml::value>>(cfg, "bindings", "binding");
	for (auto& binding: bindings) {
		key_binding_t k = {};

		auto priority = toml::find_or(binding, "priority", 255);
		k.priority = (priority << 8);

		auto app = toml::find_or_default<std::string>(binding, "app");
		if (!app.empty()) {
			auto ri = g_apps.find(app);
			if (ri == g_apps.end()) {
				// TODO: log? throw?
				continue;
			}
			k.executable = &ri->second.pattern;
			k.priority += ri->second.priority;
		}
		else
			k.priority += UCHAR_MAX;
		k.foreground_window(toml::find_or_default<bool>(binding, "foreground_window"));
		k.oneshot(toml::find_or_default<bool>(binding, "oneshot"));

		auto m = binding["modifiers"];
		if (m.is_string()) {
			for (auto mod: split_string(m.as_string()))
				k.add_modifier(mod);
		}
		else if (!m.is_empty()) {
			auto mods = m.as_array();
			for (auto& mod: mods)
				k.add_modifier(mod.as_string());
		}

		auto b = binding["buttons"];
		if (b.is_string()) {
			for (auto button: split_string(b.as_string()))
				k.buttons |= parse_button(button);
		}
		else if (!b.is_empty()) {
			auto buttons = b.as_array();
			for (auto& button: buttons)
				k.buttons |= parse_button(button.as_string());
		}

		auto keys = binding["keys"];
		if (keys.is_string()) {
			auto ks = split_string(keys.as_string());
			if (ks.size() > std::size(k.keys))
				throw std::runtime_error("too many keys");
			for (int i = 0; i < ks.size(); ++i)
				k.keys[i] = parse_vk_code(ks[i]);
		}
		else if (!keys.is_empty()) {
			auto ks = keys.as_array();
			assert(std::size(k.keys) == 4);
			if (ks.size() > std::size(k.keys))
				throw std::runtime_error("too many keys");
			for (int i = 0; i < ks.size(); ++i)
				k.keys[i] = parse_vk_code(ks[i].as_string());
		}

		g_key_bindings.push_back(std::move(k));
	}

	for (auto& sb: g_single_button)
		if (sb.buttons != 0)
			g_key_bindings.push_back(sb);

	std::sort(
		g_key_bindings.begin(),
		g_key_bindings.end(),
		[](auto& a, auto& b){
			return a.buttons < b.buttons ||
				a.buttons == b.buttons && a.priority < b.priority; 
		}
	);
} // configure_input()


std::shared_ptr<spdlog::logger> get_logger(const std::string& dir, size_t max_size, size_t max_files)
{
	namespace fs = std::filesystem;

	static std::shared_ptr<spdlog::logger> logger;
	if (!logger) {
		auto path = fs::path(dir)/"gpmouse.log";
		logger = spdlog::rotating_logger_mt("gpmouse", path.string(), max_size, max_files);
	}
	return logger;
}

#pragma warning(push)
#pragma warning(disable:4996)
std::string expand_environment_variables(const std::string& s)
{
	using namespace std::regex_constants;

	std::regex env("<[^>]*>", ECMAScript | icase);

	std::smatch matches;
	if (std::regex_search(s, matches, env)) {
		std::string ret;
		auto p = s.cbegin();
		auto out = std::back_inserter(ret);

		for (auto& m: matches) {
			if (m.length() == 2)
				throw std::runtime_error("found empty variable name in log directory setting");
			std::copy(p, m.first, out);
			p += m.length();

			auto varname = m.str().substr(1, m.length() - 2);
			auto varval = std::getenv(varname.c_str());
			if (varval == NULL)
				throw std::runtime_error(std::format("no environment variable named '{}'", varname));
			std::copy(varval, varval + strlen(varval), out);
		}

		std::copy(p, s.cend(), out);

		return ret;
	}
	else
		return s;
}
#pragma warning(pop)

void configure_log(const toml::value& cfg)
{
	auto logging_cfg = toml::find<toml::value>(cfg, "logging");

	auto dir = toml::find_or<std::string>(logging_cfg, "directory", "<temp>");
	dir = expand_environment_variables(dir);

	auto max_size = toml::find_or<size_t>(logging_cfg, "max_size", 4 * 1024 * 1024);
	auto max_files = toml::find_or<size_t>(logging_cfg, "max_files", 10);

	auto logger = get_logger(dir, max_size, max_files);


	auto level_str = toml::find_or_default<std::string>(logging_cfg, "level");
	if (!level_str.empty()) {
		auto level = spdlog::level::from_str(level_str);
		logger->set_level(level);
	}

	auto pattern = toml::find_or_default<std::string>(logging_cfg, "pattern");
	if (!pattern.empty())
		logger->set_pattern(pattern);

	auto format = toml::find_or_default<std::string>(logging_cfg, "format");
	if (!format.empty()) {
		// logger->set_formatter(
	}
} // configure_log()

#else

#endif // def TOML_TOML11

std::shared_ptr<spdlog::logger> get_logger()
{
	return get_logger("", 0, 0);
}

void configure()
{
	namespace fs = std::filesystem;

	auto cfg_file = fs::path(application_directory()) / L"gpmouse.toml";

	if (!fs::is_regular_file(cfg_file)) {
		default_config();
		return;
	}

	auto cfg = toml::parse(cfg_file, toml::spec::v(1, 1, 0));

	configure_log(cfg);
	configure_input(cfg);
}

} // namespace gpmouse
