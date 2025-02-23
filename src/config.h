#ifndef GPMOUSE_CONFIG_H
#define GPMOUSE_CONFIG_H
#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <spdlog/spdlog.h>


namespace gpmouse
{

struct modifiers_t
{
	union {
		uint16_t value;
		struct {
			uint16_t ctrl : 1;
			uint16_t alt : 1;
			uint16_t shift : 1;
			uint16_t win : 1;
			uint16_t cmd : 1;
			uint16_t meta : 1;
			uint16_t copilot : 1;
			uint16_t repeatble : 1;
			uint16_t mod0 : 1;
			uint16_t mod1 : 1;
			uint16_t mod2 : 1;
			uint16_t mod3 : 1;
			uint16_t mod4 : 1;
			uint16_t mod5 : 1;
			uint16_t mod6 : 1;
			uint16_t mod7 : 1;
		};
	};
};

static const uint8_t VK_MOUSE_EVENTS = VK_LBUTTON|VK_RBUTTON|VK_MBUTTON|VK_XBUTTON1|VK_XBUTTON2;
static const uint8_t VK_MODIFIERS    = VK_SHIFT|VK_CONTROL|VK_MENU|VK_LSHIFT|VK_RSHIFT|VK_LCONTROL|VK_RCONTROL|VK_LMENU|VK_RMENU|VK_LWIN|VK_RWIN;

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

static constexpr uint64_t MOUSE_EVENTS_MASK[4] = {
	mouse_events_mask(0),
	mouse_events_mask(1),
	mouse_events_mask(2),
	mouse_events_mask(3),
};
static const uint64_t MODIFIERS_MASK[4] = {
	modifiers_mask(0),
	modifiers_mask(1),
	modifiers_mask(2),
	modifiers_mask(3),
};

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
	// TODO: exp, log �̃p�����[�^
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
		// virtual key code 0 �͉��ɂ����蓖�Ă��Ă��Ȃ��̂ŁA���s�[�g�֎~�t���O�Ƃ��Ďg��
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
	uint8_t keys[4] = {}; // �Ƃ肠����4�����܂�

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
		ONESHOT       = 1 << 0,
		ACTIVE_WINDOW = 1 << 1,
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
	bool active_window() const {
		return flags & ACTIVE_WINDOW;
	}
	void active_window(bool val) {
		set_flag(ACTIVE_WINDOW, val);
	}

	void fill(keystate_t& ks) {
		memset(&ks, 0, sizeof(ks));

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


// g_key_binding �� (buttons asc, process desc) �Ń\�[�g���Ă���
extern std::vector<key_binding_t> g_key_bindings;
extern key_binding_t g_single_button[16];

void configure();
std::shared_ptr<spdlog::logger> get_logger();

const char* vk_name(uint8_t vk);

} // namespace gpmouse

#endif // ndef GPMOUSE_CONFIG_H
