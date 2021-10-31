#pragma once

#include "Nebula/API.h"
#include "Event_Manager.h"

#include <functional>
#include <string>

//TODO: Add Event Listener/Manager

namespace Nebula {
	// Events are currently BLOCKING. When an event occurs
	// it has to be dealt with then and there. No Event
	// cue has been setup.

	//TODO: Buffer Events into Cue

	enum class EventType {
		None = 0,
		//Window Events
		WindowClose, WindowResize, WindowFocus, WindowNoFocus, WindowMoved,
		//Key Updates
		KeyPressed, KeyReleased,
		//Mouse Updates
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory {
		None = 0,
		ApplicationCat  = BIT(0),
		InputCat		= BIT(1),
		KeyboardCat		= BIT(2),
		MouseCat		= BIT(3)
	};

#define EVENT_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
						 virtual EventType GetEventType() const override { return GetStaticType(); }\
						 virtual const char* GetName() const override { return #type; }

#define EVENT_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class NB_API Event
	{
		friend class Dispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) {
			return GetCategoryFlags() & category;
		}
	protected:
		bool m_Handled = false;
	};

	class NB_API Dispatcher {
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		Dispatcher(Event& event): m_Event(event) { }

		template<typename T>
		bool Dispatch(EventFn<T> func) {
			if (m_Event.GetEventType() == T::GetStaticType()) {
				//TODO Call Event Listener from event Manager
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.ToString();
	}
}