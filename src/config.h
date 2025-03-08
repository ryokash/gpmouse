#ifndef GPMOUSE_CONFIG_H
#define GPMOUSE_CONFIG_H
#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <regex>

#include <xinput.h>

#include <boost/algorithm/string.hpp>
#include <spdlog/spdlog.h>


namespace gpmouse
{

constexpr inline uint64_t mask_bit(int i, uint8_t vk)
{
	return (vk >> 6) == i ? (1ull << (vk & 0x3f)) : 0;
}
constexpr inline uint64_t mouse_events_mask(int i)
{
	return mask_bit(i, VK_LBUTTON)|mask_bit(i, VK_RBUTTON)|mask_bit(i, VK_MBUTTON)|mask_bit(i, VK_XBUTTON1)|mask_bit(i, VK_XBUTTON2);
}

constexpr inline uint64_t modifiers_mask(int i)
{
	return 
		mask_bit(i, VK_SHIFT) | 
		mask_bit(i, VK_CONTROL) | 
		mask_bit(i, VK_MENU) | 
		mask_bit(i, VK_LSHIFT) | 
		mask_bit(i, VK_RSHIFT) | 
		mask_bit(i, VK_LCONTROL) | 
		mask_bit(i, VK_RCONTROL) | 
		mask_bit(i, VK_LMENU) | 
		mask_bit(i, VK_RMENU) | 
		mask_bit(i, VK_LWIN) | 
		mask_bit(i, VK_RWIN);
}

constexpr inline uint64_t extended_keys_mask(int i)
{
	return
		mask_bit(i, VK_RMENU) |
		mask_bit(i, VK_RCONTROL) |
		mask_bit(i, VK_LWIN) |
		mask_bit(i, VK_RWIN) |
		mask_bit(i, VK_INSERT) |
		mask_bit(i, VK_DELETE) |
		mask_bit(i, VK_HOME) |
		mask_bit(i, VK_END) |
		mask_bit(i, VK_PRIOR) |
		mask_bit(i, VK_NEXT) |
		mask_bit(i, VK_UP) |
		mask_bit(i, VK_DOWN) |
		mask_bit(i, VK_LEFT) |
		mask_bit(i, VK_RIGHT) |
		mask_bit(i, VK_NUMLOCK) |
		mask_bit(i, VK_PAUSE) | // TODO: Ctrl+Pause is extended key ??
		mask_bit(i, VK_PRINT) |
		mask_bit(i, VK_DIVIDE) |
		mask_bit(i, VK_RETURN);
}

static constexpr uint64_t MOUSE_EVENTS_MASK[4] = {
	mouse_events_mask(0),
	mouse_events_mask(1),
	mouse_events_mask(2),
	mouse_events_mask(3),
};
static constexpr uint64_t MODIFIERS_MASK[4] = {
	modifiers_mask(0),
	modifiers_mask(1),
	modifiers_mask(2),
	modifiers_mask(3),
};
static constexpr uint64_t EXTENDED_KEYS_MASK[4] = {
	extended_keys_mask(0),
	extended_keys_mask(1),
	extended_keys_mask(2),
	extended_keys_mask(3),
};

inline bool has_key(const uint64_t keys[4], uint8_t vk)
{
	return keys[vk >> 6] & (1ull << (vk & 0x3f));
}
inline bool is_extended_key(uint8_t vk)
{
	return has_key(EXTENDED_KEYS_MASK, vk);
}


constexpr inline uint64_t keys_mask(int i) {
	return (~mouse_events_mask(i)) & (~modifiers_mask(i));
}
inline uint64_t repeatable_keys(int i) {
	// repetable keys
	// VK_BACK		(0x08)
	// VK_TAB		(0x09)
	// VK_RETURN	(0x0d)
	// VK_SPACE (0x20) - VK_DOWN (0x28)
	// '0' (0x30) - '9' (0x39)
	// 'A' (0x41) - 'Z' (0x5a)
	// VK_NUMPAD0 (0x60) - VK_DIVIDE (0x69)
	////// VK_F1 (0x70) - VK_F24 (0x87)
	// VK_OEM_1 (0xba) - VK_OEM_3 (0xc0)
	// VK_OEM_4 (0xdb) - VK_OEM_8 (0xdf)
	static const uint64_t repkeys[4] = {
		0x03ff01ff00002300ull,
		//0xffffffff07fffffeull,
		0x000003ff07fffffeull,
		//0xfc000000000000ffull,
		0xfc00000000000000ull,
		0x00000000f8000001ull,
	};
	return repkeys[i];
}

enum class analog_function_t
{
	linear,
	step,
	exp,
	log,
};

struct trigger_config_t
{
	uint8_t deadzone;
	uint8_t saturation;
	int16_t minval;
	int16_t maxval;
	analog_function_t type;
	// TODO: exp, log のパラメータ
	float c; // c * a ** deadzone == minval, c * a ** saturation == maxval
	float a; // c * log_a(deadzone) == minval, c * log_a(saturation) == maxval
};

struct stick_config_t
{

};

struct keystate_t
{
	uint64_t keys[4] = {};

	void press(uint8_t key) {
		keys[key >> 6] |= (1ull << (key & 0x3f));
	}
	bool is_pressed(uint8_t key) const {
		return (keys[key >> 6] & (1ull << (key & 0x3f))) != 0;
	}
	bool empty() const {
		for (auto k: keys)
			if (k != 0)
				return false;
		return true;
	}
	bool oneshot() const {
		// virtual key code 0 は何にも割り当てられていないので、リピート禁止フラグとして使う
		return (keys[0] & 1) == 1;
	}
};



struct key_binding_t
{
	std::regex* executable = 0;
	uint16_t priority = USHRT_MAX;
	uint16_t buttons = 0;
	uint8_t flags = 0;
	uint8_t modifiers = 0;
	uint8_t keys[4] = {}; // とりあえず4つ押しまで

	enum {
		CONTROL = 1 << 0,
		ALT     = 1 << 1,
		SHIFT   = 1 << 2,
		WINDOWS = 1 << 3,
		COMMAND = 1 << 4,
		META    = 1 << 5,
		COPILOT = 1 << 6,
	};
	enum {
		ONESHOT				= 1 << 0,
		FOREGROUND_WINDOW	= 1 << 1,
	};

	bool has_alt() const {
		return modifiers & ALT;
	}
	bool has_ctrl() const {
		return modifiers & CONTROL;
	}
	bool has_shift() const {
		return modifiers & SHIFT;
	}
	bool has_win() const {
		return modifiers & WINDOWS;
	}

	void add_modifier(const std::string& ms) {
		auto s = boost::to_upper_copy(ms);
		if (s == "CONTROL" || s == "CTRL")
			modifiers |= CONTROL;
		if (s == "ALT")
			modifiers |= ALT;
		if (s == "SHIFT")
			modifiers |= SHIFT;
		if (s == "WIN" || s == "WINDOWS")
			modifiers |= WINDOWS;
	}

	void on(uint8_t flag) {
		flags |= flag;
	}
	void off(uint8_t flag) {
		flags &= ~flag;
	}
	void set_flag(uint8_t flag, bool val) {
		val ? on(flag) : off(flag);
	}
	bool oneshot() const {
		return flags & ONESHOT;
	}
	void oneshot(bool val) {
		set_flag(ONESHOT, val);
	}
	bool foreground_window() const {
		return flags & FOREGROUND_WINDOW;
	}
	void foreground_window(bool val) {
		set_flag(FOREGROUND_WINDOW, val);
	}

	void fill(keystate_t& ks) {
		if (has_alt())
			ks.press(VK_MENU);
		if (has_ctrl())
			ks.press(VK_CONTROL);
		if (has_shift())
			ks.press(VK_SHIFT);
		if (has_win())
			ks.press(VK_LWIN);
		if (oneshot())
			ks.press(0);

		for (auto vk: keys) {
			if (vk == 0)
				break;
			ks.press(vk);
		}
	}
};
inline bool operator<(const key_binding_t& a, const key_binding_t& b)
{
	return a.buttons < b.buttons;
}


enum class accel_type_t: uint16_t
{
	linear = 0,
	exponential,
};
enum class trigger_function_t: uint16_t
{
	nop = 0,
	acceleration,
	deacceleration,
};

struct stick_t
{
	int16_t cx = 0;
	int16_t cy = 0;
	uint16_t deadzone = 2500;
	accel_type_t accel_type = accel_type_t::exponential;
	float base_speed = 0.8;
	float accel_max = 8;
	float deaccel_max = 4;
	trigger_function_t left_trigger = trigger_function_t::deacceleration;
	trigger_function_t right_trigger = trigger_function_t::acceleration;
};

struct stick_params_t
{
	bool initialized = false;
	stick_t cursor;
	stick_t scroll;
	float min_touch_dist = 50;
};

// g_key_binding は (buttons asc, process desc) でソートしておく
extern std::vector<key_binding_t> g_key_bindings;
extern key_binding_t g_single_button[16];
extern stick_params_t g_stick_params[XUSER_MAX_COUNT];


void configure();
std::shared_ptr<spdlog::logger> get_logger();

const char* vk_name(uint8_t vk);

} // namespace gpmouse

#endif // ndef GPMOUSE_CONFIG_H
