#pragma once

#include "Rod/Core/Core.h"
#include "Rod/Core/LayerStack.h"
#include "Rod/Core/Timestep.h"
#include "Rod/Core/Window.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/Event.h"
#include "Rod/ImGui/ImGuiLayer.h"

namespace Rod {

	class Application
	{
	public:
		explicit Application(
		    const std::string& name = "Rod App",
		    const std::string& iconFilepath = "",
		    bool usesCustomUi = false);
		explicit Application(const WindowProps& props);
		virtual ~Application();
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		[[nodiscard]] Window& GetWindow() noexcept { return *m_Window; };

		void Close();

		void Minimize();
		[[nodiscard]] bool IsMinimized() const noexcept { return m_Minimized; }

		void Maximize();
		[[nodiscard]] bool IsMaximized() const noexcept { return m_Maximized; }

		void RestoreWindow();
		void BeginWindowDrag();

		[[nodiscard]] ImGuiLayer* GetImGuiLayer() noexcept { return m_ImGuiLayer; };

		[[nodiscard]] virtual bool IsEditor() const { return false; }

		[[nodiscard]] static Application& Get() noexcept { return *s_Instance; };

	private:
		Timestep CalculateTimestep();
		void UpdateLayers(Timestep timestep);
		void RenderImGui();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer = nullptr;
		bool m_Running = true, m_Minimized = false, m_Maximized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}
