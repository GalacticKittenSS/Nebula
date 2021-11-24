#pragma once

namespace Nebula {
	typedef enum class MouseCode : uint16_t {
		// From glfw3.h
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Button6 = 6,
		Button7 = 7,

		ButtonLast = Button7,
		ButtonLeft = Button0,
		ButtonRight = Button1,
		ButtonMiddle = Button2
	} Mouse;

	inline std::ostream& operator<<(std::ostream& os, MouseCode button) {
		return os << static_cast<int32_t>(button);
	}
}

#define NB_MOUSE_0 ::Nebula::Mouse::Button0  
#define NB_MOUSE_1 ::Nebula::Mouse::Button1  
#define NB_MOUSE_2 ::Nebula::Mouse::Button2  
#define NB_MOUSE_3 ::Nebula::Mouse::Button3  
#define NB_MOUSE_4 ::Nebula::Mouse::Button4  
#define NB_MOUSE_5 ::Nebula::Mouse::Button5  
#define NB_MOUSE_6 ::Nebula::Mouse::Button6  
#define NB_MOUSE_7 ::Nebula::Mouse::Button7  
#define NB_MOUSE_LAST ::Nebula::Mouse::ButtonLast  
#define NB_MOUSE_LEFT ::Nebula::Mouse::ButtonLeft  
#define NB_MOUSE_RIGHT ::Nebula::Mouse::ButtonRight  
#define NB_MOUSE_MIDDLE ::Nebula::Mouse::ButtonMiddle 