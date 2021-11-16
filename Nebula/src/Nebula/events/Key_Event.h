#pragma once

#include "Event.h"
#include "Nebula/Core/Input.h"

namespace Nebula {
	class NB_API KeyEvent : public Event {
	public:
		inline KeyCode GetKeyCode() const { return m_KeyCode; }

		EVENT_CATEGORY(KeyboardCat | InputCat);
	protected:
		KeyEvent(KeyCode keycode): m_KeyCode(keycode) { }

		KeyCode m_KeyCode;
	};

	class NB_API KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(KeyCode keycode, int32_t repeatCount): KeyEvent(keycode), m_RepeatCount(repeatCount) { }

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
		KeyReleasedEvent(KeyCode keycode): KeyEvent(keycode) { }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_TYPE(KeyReleased)
	};

	class NB_API KeyTypedEvent : public KeyEvent {
	public:
		KeyTypedEvent(KeyCode keycode) : KeyEvent(keycode) { }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_TYPE(KeyTyped)
	};
}