#pragma once

#include "Event.h"
#include "Nebula/Core/Input.h"

namespace Nebula {
	class NB_API MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(float x, float y): m_MouseX(x), m_MouseY(y) { }

		inline float GetX() const { return m_MouseX; }
		inline float GetY() const { return m_MouseY; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_TYPE(MouseMoved)
		EVENT_CATEGORY(MouseCat | InputCat)
	private:
		float m_MouseX, m_MouseY;
	};

	
	class NB_API MouseScrolledEvent : public Event {
	public:
		MouseScrolledEvent(float x, float y) : m_XOffset(x), m_YOffset(y) { }

		inline float GetOffsetX() const { return m_XOffset; }
		inline float GetOffsetY() const { return m_YOffset; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		EVENT_TYPE(MouseScrolled)
		EVENT_CATEGORY(MouseCat | InputCat)
	private:
		float m_XOffset, m_YOffset;
	};

	class NB_API MouseButtonEvent : public Event {
	public:
		inline MouseCode GetMouseButton() const { return m_Button; }

		EVENT_CATEGORY(MouseCat | InputCat)
	protected:
		MouseButtonEvent(MouseCode button): m_Button(button) { }

		MouseCode m_Button;
	};	
	
	class NB_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode button): MouseButtonEvent(button) { }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_TYPE(MouseButtonPressed)
	};

	class NB_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseCode button): MouseButtonEvent(button) { }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_TYPE(MouseButtonReleased)
	};
}