#include "rdpch.h"

#include "ImGuiSession.h"

#include "Rod/Core/Window.h"
#include "Rod/ImGui/ImGuiFlags.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "imgui.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "ImGuizmo.h"
#include <span>
#include <stdexcept>

namespace Rod {

	namespace {
		constexpr float g_DefaultFontSize = 18.0f;
		constexpr ImVec2 g_WindowPadding{12.0f, 9.0f};
		constexpr ImVec2 g_FramePadding{8.0f, 5.0f};
		constexpr ImVec2 g_CellPadding{8.0f, 5.0f};
		constexpr ImVec2 g_ItemSpacing{9.0f, 7.0f};
		constexpr ImVec2 g_ItemInnerSpacing{6.0f, 5.0f};
		constexpr float g_ScrollbarSize = 12.0f;
		constexpr float g_GrabMinSize = 10.0f;
		constexpr float g_ControlRounding = 3.0f;
		constexpr float g_ScrollbarRounding = 4.0f;

		constexpr auto g_DarkColors = std::to_array<std::pair<ImGuiCol, ImVec4>>({
		    {ImGuiCol_Text, {0.88f, 0.90f, 0.93f, 1.0f}},
		    {ImGuiCol_TextDisabled, {0.46f, 0.50f, 0.56f, 1.0f}},
		    {ImGuiCol_WindowBg, {0.075f, 0.085f, 0.098f, 1.0f}},
		    {ImGuiCol_ChildBg, {0.075f, 0.085f, 0.098f, 1.0f}},
		    {ImGuiCol_PopupBg, {0.095f, 0.105f, 0.12f, 1.0f}},
		    {ImGuiCol_Border, {0.20f, 0.23f, 0.28f, 1.0f}},
		    {ImGuiCol_BorderShadow, {0.0f, 0.0f, 0.0f, 0.0f}},
		    {ImGuiCol_Header, {0.16f, 0.18f, 0.21f, 1.0f}},
		    {ImGuiCol_HeaderHovered, {0.23f, 0.26f, 0.31f, 1.0f}},
		    {ImGuiCol_HeaderActive, {0.30f, 0.34f, 0.40f, 1.0f}},
		    {ImGuiCol_Button, {0.15f, 0.17f, 0.20f, 1.0f}},
		    {ImGuiCol_ButtonHovered, {0.23f, 0.26f, 0.31f, 1.0f}},
		    {ImGuiCol_ButtonActive, {0.12f, 0.14f, 0.17f, 1.0f}},
		    {ImGuiCol_FrameBg, {0.12f, 0.14f, 0.17f, 1.0f}},
		    {ImGuiCol_FrameBgHovered, {0.18f, 0.21f, 0.25f, 1.0f}},
		    {ImGuiCol_FrameBgActive, {0.22f, 0.26f, 0.31f, 1.0f}},
		    {ImGuiCol_CheckMark, {0.82f, 0.20f, 0.27f, 1.0f}},
		    {ImGuiCol_SliderGrab, {0.60f, 0.64f, 0.70f, 1.0f}},
		    {ImGuiCol_SliderGrabActive, {0.82f, 0.20f, 0.27f, 1.0f}},
		    {ImGuiCol_Tab, {0.12f, 0.14f, 0.17f, 1.0f}},
		    {ImGuiCol_TabHovered, {0.23f, 0.26f, 0.31f, 1.0f}},
		    {ImGuiCol_TabActive, {0.17f, 0.20f, 0.24f, 1.0f}},
		    {ImGuiCol_TabUnfocused, {0.10f, 0.11f, 0.13f, 1.0f}},
		    {ImGuiCol_TabUnfocusedActive, {0.14f, 0.16f, 0.19f, 1.0f}},
		    {ImGuiCol_TitleBg, {0.10f, 0.12f, 0.15f, 1.0f}},
		    {ImGuiCol_TitleBgActive, {0.13f, 0.15f, 0.18f, 1.0f}},
		    {ImGuiCol_TitleBgCollapsed, {0.10f, 0.12f, 0.15f, 1.0f}},
		    {ImGuiCol_MenuBarBg, {0.10f, 0.12f, 0.15f, 1.0f}},
		    {ImGuiCol_ScrollbarBg, {0.075f, 0.085f, 0.098f, 1.0f}},
		    {ImGuiCol_ScrollbarGrab, {0.22f, 0.25f, 0.30f, 1.0f}},
		    {ImGuiCol_ScrollbarGrabHovered, {0.30f, 0.34f, 0.40f, 1.0f}},
		    {ImGuiCol_ScrollbarGrabActive, {0.38f, 0.43f, 0.50f, 1.0f}},
		    {ImGuiCol_Separator, {0.20f, 0.23f, 0.28f, 1.0f}},
		    {ImGuiCol_SeparatorHovered, {0.38f, 0.43f, 0.50f, 1.0f}},
		    {ImGuiCol_SeparatorActive, {0.82f, 0.20f, 0.27f, 1.0f}},
		    {ImGuiCol_ResizeGrip, {0.30f, 0.34f, 0.40f, 0.25f}},
		    {ImGuiCol_ResizeGripHovered, {0.38f, 0.43f, 0.50f, 0.55f}},
		    {ImGuiCol_ResizeGripActive, {0.82f, 0.20f, 0.27f, 0.85f}},
		    {ImGuiCol_DockingPreview, {0.82f, 0.20f, 0.27f, 0.35f}},
		    {ImGuiCol_DockingEmptyBg, {0.075f, 0.085f, 0.098f, 1.0f}},
		});
	}

	ImGuiSession::ImGuiSession(const Window& window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		try
		{
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = CombineImGuiFlags(
			    io.ConfigFlags,
			    ImGuiConfigFlags_NavEnableKeyboard,
			    ImGuiConfigFlags_DockingEnable,
			    ImGuiConfigFlags_ViewportsEnable);

			io.Fonts->AddFontFromFileTTF("RodAssets/Fonts/OpenSans/OpenSans-Bold.ttf", g_DefaultFontSize);
			io.FontDefault =
			    io.Fonts->AddFontFromFileTTF("RodAssets/Fonts/OpenSans/OpenSans-Regular.ttf", g_DefaultFontSize);

			ImGui::StyleColorsDark();

			ImGuiStyle& style = ImGui::GetStyle();
			if (HasAnyImGuiFlag(io.ConfigFlags, ImGuiConfigFlags_ViewportsEnable))
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			SetDarkThemeColors();

			auto* nativeWindow = static_cast<GLFWwindow*>(window.GetNativeWindow());

			if (!ImGui_ImplGlfw_InitForOpenGL(nativeWindow, true))
				throw std::runtime_error("Failed to initialize the ImGui GLFW backend");
			if (!ImGui_ImplOpenGL3_Init("#version 410"))
				throw std::runtime_error("Failed to initialize the ImGui OpenGL backend");
		}
		catch (...)
		{
			const ImGuiIO& io = ImGui::GetIO();
			if (io.BackendRendererUserData)
				ImGui_ImplOpenGL3_Shutdown();
			if (io.BackendPlatformUserData)
				ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			throw;
		}
	}

	ImGuiSession::~ImGuiSession()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiSession::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiSession::End(bool drawable)
	{
		ImGui::Render();
		if (drawable)
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		const ImGuiIO& io = ImGui::GetIO();

		if (HasAnyImGuiFlag(io.ConfigFlags, ImGuiConfigFlags_ViewportsEnable))
		{
			GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupCurrentContext);
		}
	}

	void ImGuiSession::SetDarkThemeColors()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = g_WindowPadding;
		style.FramePadding = g_FramePadding;
		style.CellPadding = g_CellPadding;
		style.ItemSpacing = g_ItemSpacing;
		style.ItemInnerSpacing = g_ItemInnerSpacing;
		style.ScrollbarSize = g_ScrollbarSize;
		style.GrabMinSize = g_GrabMinSize;
		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.FrameRounding = g_ControlRounding;
		style.PopupRounding = g_ControlRounding;
		style.ScrollbarRounding = g_ScrollbarRounding;
		style.GrabRounding = g_ControlRounding;
		style.TabRounding = g_ControlRounding;

		std::span colors(style.Colors);
		for (const auto& [slot, color] : g_DarkColors)
			colors.at(static_cast<size_t>(slot)) = color;
	}
}
