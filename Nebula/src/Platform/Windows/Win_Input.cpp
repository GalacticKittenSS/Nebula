#include "nbpch.h"
#include "Win_Input.h"

#include <GLFW/glfw3.h>
#include "Nebula/Core/Application.h"

namespace Nebula {
	bool Win_Input::IsKeyPressedImpl(KeyCode key) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, static_cast<int32_t>(key));

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Win_Input::IsMouseButtonPressedImpl(MouseCode button) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));

		return state == GLFW_PRESS;
	}

	std::pair<float, float> Win_Input::GetMousePosImpl() {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return  { (float)xpos, (float)ypos };
	}

	float Win_Input::GetMouseXImpl() {
		auto [x, y] = GetMousePosImpl();
		return x;
	}

	float Win_Input::GetMouseYImpl() {
		auto [x, y] = GetMousePosImpl();
		return y;
	}
}