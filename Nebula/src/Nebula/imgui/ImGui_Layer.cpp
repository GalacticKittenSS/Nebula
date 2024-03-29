#include "nbpch.h"
#include "ImGui_Layer.h"

#include "Nebula/Core/Application.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <platform/imgui_impl_glfw.h>
#include <platform/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

namespace Nebula {
	ImGuiLayer::ImGuiLayer(): Layer("ImGuiLayer") { }

	ImGuiLayer::~ImGuiLayer() { }

	void ImGuiLayer::Attach() {
		NB_PROFILE_FUNCTION();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		io.Fonts->AddFontFromFileTTF("Resources/fonts/OpenSans/Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Resources/fonts/OpenSans/Regular.ttf", 18.0f);

		SetDarkThemeColour();
		
		ImGui::GetStyle().FrameRounding = 4.0f;

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::Detach() {
		NB_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e) {
		if (m_BlockEvents) {
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(MouseCat) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(KeyboardCat) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin() {
		NB_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End() {
		NB_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::SetColours(ImVec4 primary, ImVec4 text, ImVec4 regular, ImVec4 hovered, ImVec4 active) {
		auto& colours = ImGui::GetStyle().Colors;
		colours[ImGuiCol_WindowBg] = primary;
		colours[ImGuiCol_MenuBarBg] = active;
		colours[ImGuiCol_PopupBg] = primary;
		colours[ImGuiCol_Text] = text;

		// Headers
		colours[ImGuiCol_Header] = active;
		colours[ImGuiCol_HeaderHovered] = hovered;
		colours[ImGuiCol_HeaderActive] = regular;

		// Buttons
		colours[ImGuiCol_Button] = regular;
		colours[ImGuiCol_ButtonHovered] = hovered;
		colours[ImGuiCol_ButtonActive] = active;

		// Frame BG
		colours[ImGuiCol_FrameBg] = regular;
		colours[ImGuiCol_FrameBgHovered] = hovered;
		colours[ImGuiCol_FrameBgActive] = active;

		// Tabs
		colours[ImGuiCol_Tab] = regular;
		colours[ImGuiCol_TabHovered] = hovered; 
		colours[ImGuiCol_TabActive] = hovered;
		colours[ImGuiCol_TabUnfocused] = regular;
		colours[ImGuiCol_TabUnfocusedActive] = primary;

		// Title
		colours[ImGuiCol_TitleBg] = active;
		colours[ImGuiCol_TitleBgActive] = active;
		colours[ImGuiCol_TitleBgCollapsed] = active;
	}

	void ImGuiLayer::SetDarkThemeColour() {
		ImVec4 primary = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		ImVec4 text = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		ImVec4 regular = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };
		ImVec4 hovered = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		ImVec4 active  = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		SetColours(primary, text, regular, hovered, active);
	}

	void ImGuiLayer::SetColdThemeColour() {
		ImVec4 primary = ImVec4{ 0.175f, 0.2f, 0.26f, 1.0f };
		ImVec4 text = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		ImVec4 regular = ImVec4{ 0.075f, 0.105f, 0.16f, 1.0f };
		ImVec4 hovered = ImVec4{ 0.275f, 0.305f, 0.36f, 1.0f };
		ImVec4 active = ImVec4{ 0.125f, 0.1505f, 0.201f, 1.0f };

		SetColours(primary, text, regular, hovered, active);
	}

	void ImGuiLayer::SetLightThemeColour() {
		ImVec4 primary = ImVec4{ 0.8f, 0.805f, 0.81f, 1.0f };
		ImVec4 text = ImVec4{ 0.115f, 0.12f, 0.125f, 1.0f };
		ImVec4 regular = ImVec4{ 0.5f, 0.505f, 0.51f, 1.0f };
		ImVec4 hovered = ImVec4{ 0.9f, 0.905f, 0.91f, 1.0f };
		ImVec4 active = ImVec4{ 0.55f, 0.5505f, 0.551f, 1.0f };

		SetColours(primary, text, regular, hovered, active);
	}

	void ImGuiLayer::SetWarmThemeColour() {
		ImVec4 primary = ImVec4{ 0.9f, 0.755f, 0.61f, 1.0f };
		ImVec4 text = ImVec4{ 0.115f, 0.12f, 0.125f, 1.0f };
		ImVec4 regular = ImVec4{ 0.7f, 0.505f, 0.41f, 1.0f };
		ImVec4 hovered = ImVec4{ 1.0f, 0.905f, 0.61f, 1.0f };
		ImVec4 active = ImVec4{ 0.65f, 0.4505f, 0.351f, 1.0f };

		SetColours(primary, text, regular, hovered, active);
	}

	uint32_t ImGuiLayer::GetActiveWidgetID() const
	{
		return GImGui->ActiveId;
	}
}