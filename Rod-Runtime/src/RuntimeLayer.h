#pragma once

#include "Rod/Core/Core.h"
#include "Rod/Core/Layer.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/KeyEvent.h"

#include <glm/vec4.hpp>

#include <functional>

namespace Rod {

	class Input;
	class Project;
	class Scene;
	class Window;

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer(
		    const Project& project,
		    const Window& window,
		    const Input& input,
		    std::function<void()> requestClose);
		~RuntimeLayer() override;
		RuntimeLayer(const RuntimeLayer&) = delete;
		auto operator=(const RuntimeLayer&) -> RuntimeLayer& = delete;
		RuntimeLayer(RuntimeLayer&&) = delete;
		auto operator=(RuntimeLayer&&) -> RuntimeLayer& = delete;

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& event) override;

	private:
		void LoadScene(const Project& project, const Window& window);
		auto OnWindowResize(WindowResizeEvent& event) -> bool;
		auto OnKeyPressed(KeyPressedEvent& event) -> bool;

	private:
		static constexpr glm::vec4 s_ClearColor{0.15f, 0.15f, 0.15f, 1.0f};
		std::function<void()> m_RequestClose;
		Scope<Scene> m_Scene;
		bool m_RunningScene = false;
	};

}
