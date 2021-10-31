#pragma once

#include "Event.h"

#include <sstream>

namespace Nebula {
	class NB_API KeyEvent : public Event {
	public:
		inline int32_t GetKeyCode() const { return m_KeyCode; }

		EVENT_CATEGORY(KeyboardCat | InputCat);
	protected:
		KeyEvent(int32_t keycode): m_KeyCode(keycode) { }

		int32_t m_KeyCode;
	};

	class NB_API KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(int32_t keycode): KeyEvent(keycode) { }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_TYPE(KeyPressed)
	};

	class NB_API KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(int32_t keycode): KeyEvent(keycode) { }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_TYPE(KeyReleased)
	};
}