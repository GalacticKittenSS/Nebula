#pragma once

#include "Event.h"


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
		KeyPressedEvent(int32_t keycode, int32_t repeatCount): KeyEvent(keycode), m_RepeatCount(repeatCount) { }

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_TYPE(KeyPressed)
	private:
		int32_t m_RepeatCount;
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