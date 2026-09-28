#include "rdpch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Rod/Core/Application.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "ImGuizmo.h"

namespace Rod{

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{
		RD_PROFILE_FUNCTION();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		io.Fonts->AddFontFromFileTTF("assets/fonts/openSans/OpenSans-Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/openSans/OpenSans-Regular.ttf", 18.0f);

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach()
	{
		RD_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnImGuiRender()
	{
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if(m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin()
	{
		RD_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		RD_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

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

	void ImGuiLayer::SetDarkThemeColors()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(12.0f, 9.0f);
		style.FramePadding = ImVec2(8.0f, 5.0f);
		style.CellPadding = ImVec2(8.0f, 5.0f);
		style.ItemSpacing = ImVec2(9.0f, 7.0f);
		style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
		style.ScrollbarSize = 12.0f;
		style.GrabMinSize = 10.0f;
		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.FrameRounding = 3.0f;
		style.PopupRounding = 3.0f;
		style.ScrollbarRounding = 4.0f;
		style.GrabRounding = 3.0f;
		style.TabRounding = 3.0f;

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4{ 0.88f, 0.90f, 0.93f, 1.0f };
		colors[ImGuiCol_TextDisabled] = ImVec4{ 0.46f, 0.50f, 0.56f, 1.0f };
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.075f, 0.085f, 0.098f, 1.0f };
		colors[ImGuiCol_ChildBg] = ImVec4{ 0.075f, 0.085f, 0.098f, 1.0f };
		colors[ImGuiCol_PopupBg] = ImVec4{ 0.095f, 0.105f, 0.12f, 1.0f };
		colors[ImGuiCol_Border] = ImVec4{ 0.20f, 0.23f, 0.28f, 1.0f };
		colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };

		colors[ImGuiCol_Header] = ImVec4{ 0.16f, 0.18f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.23f, 0.26f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.30f, 0.34f, 0.40f, 1.0f };

		colors[ImGuiCol_Button] = ImVec4{ 0.15f, 0.17f, 0.20f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.23f, 0.26f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.12f, 0.14f, 0.17f, 1.0f };

		colors[ImGuiCol_FrameBg] = ImVec4{ 0.12f, 0.14f, 0.17f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.18f, 0.21f, 0.25f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.22f, 0.26f, 0.31f, 1.0f };
		colors[ImGuiCol_CheckMark] = ImVec4{ 0.82f, 0.20f, 0.27f, 1.0f };
		colors[ImGuiCol_SliderGrab] = ImVec4{ 0.60f, 0.64f, 0.70f, 1.0f };
		colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.82f, 0.20f, 0.27f, 1.0f };

		colors[ImGuiCol_Tab] = ImVec4{ 0.12f, 0.14f, 0.17f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.23f, 0.26f, 0.31f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.17f, 0.20f, 0.24f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.10f, 0.11f, 0.13f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.14f, 0.16f, 0.19f, 1.0f };

		colors[ImGuiCol_TitleBg] = ImVec4{ 0.10f, 0.12f, 0.15f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.13f, 0.15f, 0.18f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.10f, 0.12f, 0.15f, 1.0f };
		colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.10f, 0.12f, 0.15f, 1.0f };

		colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.075f, 0.085f, 0.098f, 1.0f };
		colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.22f, 0.25f, 0.30f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.30f, 0.34f, 0.40f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.38f, 0.43f, 0.50f, 1.0f };
		colors[ImGuiCol_Separator] = ImVec4{ 0.20f, 0.23f, 0.28f, 1.0f };
		colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.38f, 0.43f, 0.50f, 1.0f };
		colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.82f, 0.20f, 0.27f, 1.0f };
		colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.30f, 0.34f, 0.40f, 0.25f };
		colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.38f, 0.43f, 0.50f, 0.55f };
		colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.82f, 0.20f, 0.27f, 0.85f };
		colors[ImGuiCol_DockingPreview] = ImVec4{ 0.82f, 0.20f, 0.27f, 0.35f };
		colors[ImGuiCol_DockingEmptyBg] = ImVec4{ 0.075f, 0.085f, 0.098f, 1.0f };
	}
}
