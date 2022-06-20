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
		KeyPressedEvent(KeyCode keycode, bool isRepeat = false): KeyEvent(keycode), m_IsRepeat(isRepeat) { }

		inline bool IsRepeat() const { return m_IsRepeat; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (Repeat= " << m_IsRepeat << ")";
			return ss.str();
		}

		EVENT_TYPE(KeyPressed)
	private:
		bool m_IsRepeat;
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
