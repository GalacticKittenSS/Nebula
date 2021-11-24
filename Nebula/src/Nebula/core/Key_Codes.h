namespace Nebula
{
	typedef enum class KeyCode : uint16_t {
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode key) {
		return os << static_cast<uint32_t>(key);
	}
}

#define NB_SPACE ::Nebula::Key::Space  
#define NB_APOSTROPHE ::Nebula::Key::Apostrophe/* ' */  
#define NB_COMMA ::Nebula::Key::Comma /* , */  
#define NB_MINUS ::Nebula::Key::Minus /* - */  
#define NB_PERIOD ::Nebula::Key::Period/* . */  
#define NB_SLASH ::Nebula::Key::Slash /* / */  
#define NB_0 ::Nebula::Key::D0  
#define NB_1 ::Nebula::Key::D1  
#define NB_2 ::Nebula::Key::D2  
#define NB_3 ::Nebula::Key::D3  
#define NB_4 ::Nebula::Key::D4  
#define NB_5 ::Nebula::Key::D5  
#define NB_6 ::Nebula::Key::D6  
#define NB_7 ::Nebula::Key::D7  
#define NB_8 ::Nebula::Key::D8  
#define NB_9 ::Nebula::Key::D9  
#define NB_SEMICOLON ::Nebula::Key::Semicolon /* ; */  
#define NB_EQUAL ::Nebula::Key::Equal /* = */  
#define NB_A ::Nebula::Key::A  
#define NB_B ::Nebula::Key::B  
#define NB_C ::Nebula::Key::C  
#define NB_D ::Nebula::Key::D  
#define NB_E ::Nebula::Key::E  
#define NB_F ::Nebula::Key::F  
#define NB_G ::Nebula::Key::G  
#define NB_H ::Nebula::Key::H  
#define NB_I ::Nebula::Key::I  
#define NB_J ::Nebula::Key::J  
#define NB_K ::Nebula::Key::K  
#define NB_L ::Nebula::Key::L  
#define NB_M ::Nebula::Key::M  
#define NB_N ::Nebula::Key::N  
#define NB_O ::Nebula::Key::O  
#define NB_P ::Nebula::Key::P  
#define NB_Q ::Nebula::Key::Q  
#define NB_R ::Nebula::Key::R  
#define NB_S ::Nebula::Key::S  
#define NB_T ::Nebula::Key::T  
#define NB_U ::Nebula::Key::U  
#define NB_V ::Nebula::Key::V  
#define NB_W ::Nebula::Key::W  
#define NB_X ::Nebula::Key::X  
#define NB_Y ::Nebula::Key::Y  
#define NB_Z ::Nebula::Key::Z  
#define NB_LEFT_BRACKET ::Nebula::Key::LeftBracket /* [ */  
#define NB_BACKSLASH ::Nebula::Key::Backslash /* \ */  
#define NB_RIGHT_BRACKET ::Nebula::Key::RightBracket/* ] */  
#define NB_GRAVE_ACCENT ::Nebula::Key::GraveAccent /* ` */  
#define NB_WORLD_1 ::Nebula::Key::World1/* non-US #1 */  
#define NB_WORLD_2 ::Nebula::Key::World2/* non-US #2 */  
/* Function keys */
#define NB_ESCAPE ::Nebula::Key::Escape  
#define NB_ENTER ::Nebula::Key::Enter  
#define NB_TAB ::Nebula::Key::Tab  
#define NB_BACKSPACE ::Nebula::Key::Backspace  
#define NB_INSERT ::Nebula::Key::Insert  
#define NB_DELETE ::Nebula::Key::Delete  
#define NB_RIGHT ::Nebula::Key::Right  
#define NB_LEFT ::Nebula::Key::Left  
#define NB_DOWN ::Nebula::Key::Down  
#define NB_UP ::Nebula::Key::Up  
#define NB_PAGE_UP ::Nebula::Key::PageUp  
#define NB_PAGE_DOWN ::Nebula::Key::PageDown  
#define NB_HOME ::Nebula::Key::Home  
#define NB_END ::Nebula::Key::End  
#define NB_CAPS_LOCK ::Nebula::Key::CapsLock  
#define NB_SCROLL_LOCK ::Nebula::Key::ScrollLock  
#define NB_NUM_LOCK ::Nebula::Key::NumLock  
#define NB_PRINT_SCREEN ::Nebula::Key::PrintScreen  
#define NB_PAUSE ::Nebula::Key::Pause  
#define NB_F1 ::Nebula::Key::F1  
#define NB_F2 ::Nebula::Key::F2  
#define NB_F3 ::Nebula::Key::F3  
#define NB_F4 ::Nebula::Key::F4  
#define NB_F5 ::Nebula::Key::F5  
#define NB_F6 ::Nebula::Key::F6  
#define NB_F7 ::Nebula::Key::F7  
#define NB_F8 ::Nebula::Key::F8  
#define NB_F9 ::Nebula::Key::F9  
#define NB_F10 ::Nebula::Key::F10  
#define NB_F11 ::Nebula::Key::F11  
#define NB_F12 ::Nebula::Key::F12  
#define NB_F13 ::Nebula::Key::F13  
#define NB_F14 ::Nebula::Key::F14  
#define NB_F15 ::Nebula::Key::F15  
#define NB_F16 ::Nebula::Key::F16  
#define NB_F17 ::Nebula::Key::F17  
#define NB_F18 ::Nebula::Key::F18  
#define NB_F19 ::Nebula::Key::F19  
#define NB_F20 ::Nebula::Key::F20  
#define NB_F21 ::Nebula::Key::F21  
#define NB_F22 ::Nebula::Key::F22  
#define NB_F23 ::Nebula::Key::F23  
#define NB_F24 ::Nebula::Key::F24  
#define NB_F25 ::Nebula::Key::F25  
/* Keypad */
#define NB_KP_0 ::Nebula::Key::KP0  
#define NB_KP_1 ::Nebula::Key::KP1  
#define NB_KP_2 ::Nebula::Key::KP2  
#define NB_KP_3 ::Nebula::Key::KP3  
#define NB_KP_4 ::Nebula::Key::KP4  
#define NB_KP_5 ::Nebula::Key::KP5  
#define NB_KP_6 ::Nebula::Key::KP6  
#define NB_KP_7 ::Nebula::Key::KP7  
#define NB_KP_8 ::Nebula::Key::KP8  
#define NB_KP_9 ::Nebula::Key::KP9  
#define NB_KP_DECIMAL ::Nebula::Key::KPDecimal  
#define NB_KP_DIVIDE ::Nebula::Key::KPDivide  
#define NB_KP_MULTIPLY ::Nebula::Key::KPMultiply  
#define NB_KP_SUBTRACT ::Nebula::Key::KPSubtract  
#define NB_KP_ADD ::Nebula::Key::KPAdd  
#define NB_KP_ENTER ::Nebula::Key::KPEnter  
#define NB_KP_EQUAL ::Nebula::Key::KPEqual  
#define NB_LEFT_SHIFT ::Nebula::Key::LeftShift  
#define NB_LEFT_CONTROL ::Nebula::Key::LeftControl  
#define NB_LEFT_ALT ::Nebula::Key::LeftAlt  
#define NB_LEFT_SUPER ::Nebula::Key::LeftSuper  
#define NB_RIGHT_SHIFT ::Nebula::Key::RightShift  
#define NB_RIGHT_CONTROL ::Nebula::Key::RightControl  
#define NB_RIGHT_ALT ::Nebula::Key::RightAlt  
#define NB_RIGHT_SUPER ::Nebula::Key::RightSuper  
#define NB_MENU ::Nebula::Key::Menu  