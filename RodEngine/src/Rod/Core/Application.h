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
		auto operator=(const Application&) -> Application& = delete;
		Application(Application&&) = delete;
		auto operator=(Application&&) -> Application& = delete;

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Scope<Layer> layer);
		void PushOverlay(Scope<Layer> overlay);

		[[nodiscard]] auto GetWindow() noexcept -> Window& { return *m_Window; }
		[[nodiscard]] auto GetInput() const noexcept -> const Input& { return m_Input; }
		[[nodiscard]] auto GetProject() noexcept -> std::optional<Project&> { return m_Project; }
		[[nodiscard]] auto GetProject() const noexcept -> std::optional<const Project&> { return m_Project; }

		void Close() noexcept;

		[[nodiscard]] auto GetImGuiLayer() noexcept -> std::optional<ImGuiLayer&> { return m_ImGuiLayer; }

	private:
		[[nodiscard]] auto CalculateTimestep() -> Timestep;
		void UpdateLayers(Timestep timestep);
		void RenderImGui();

		auto OnWindowClose(WindowCloseEvent& e) noexcept -> bool;
		auto OnWindowResize(WindowResizeEvent& e) -> bool;

	private:
		std::optional<Project> m_Project;
		Scope<Window> m_Window;
		Input m_Input;
		std::optional<ImGuiLayer&> m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
	};

	// To be defined in CLIENT
	[[nodiscard]] auto CreateApplication(const std::optional<Project>& project) -> Scope<Application>;
}
