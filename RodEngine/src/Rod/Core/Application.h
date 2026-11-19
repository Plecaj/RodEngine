#pragma once

#include "Rod/Core/Core.h"
#include "Rod/Core/Input.h"
#include "Rod/Core/LayerStack.h"
#include "Rod/Core/Timestep.h"
#include "Rod/Core/Window.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/Event.h"
#include "Rod/Project/Project.h"

#include <optional>

namespace Rod {

	class ImGuiLayer;

	class Application
	{
	public:
		explicit Application(const WindowProps& props, std::optional<Project> project);
		virtual ~Application();
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Scope<Layer> layer);
		void PushOverlay(Scope<Layer> overlay);

		[[nodiscard]] Window& GetWindow() noexcept { return *m_Window; }
		[[nodiscard]] const Input& GetInput() const noexcept { return m_Input; }
		[[nodiscard]] ObserverPtr<Project> GetProject() noexcept { return m_Project ? &*m_Project : nullptr; }
		[[nodiscard]] ObserverPtr<const Project> GetProject() const noexcept
		{
			return m_Project ? &*m_Project : nullptr;
		}

		void Close() noexcept;

		[[nodiscard]] ObserverPtr<ImGuiLayer> GetImGuiLayer() noexcept { return m_ImGuiLayer; }

	private:
		[[nodiscard]] Timestep CalculateTimestep();
		void UpdateLayers(Timestep timestep);
		void RenderImGui();

		bool OnWindowClose(WindowCloseEvent& e) noexcept;
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		std::optional<Project> m_Project;
		Scope<Window> m_Window;
		Input m_Input;
		ObserverPtr<ImGuiLayer> m_ImGuiLayer = nullptr;
		bool m_Running = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
	};

	// To be defined in CLIENT
	[[nodiscard]] Scope<Application> CreateApplication(const std::optional<Project>& project);
}
