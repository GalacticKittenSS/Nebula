#pragma once

#include "Event.h"

namespace Nebula {
	class NB_API WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, uint32_t height): m_Width(width), m_Height(height) { }

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_TYPE(WindowResize)
		EVENT_CATEGORY(ApplicationCat)
	private:
		uint32_t m_Width, m_Height;
	};

	class NB_API WindowCloseEvent: public Event{
	public:
		WindowCloseEvent() { }

		EVENT_TYPE(WindowClose)
		EVENT_CATEGORY(ApplicationCat)
	};
}