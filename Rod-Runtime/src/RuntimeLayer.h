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
		RuntimeLayer& operator=(const RuntimeLayer&) = delete;
		RuntimeLayer(RuntimeLayer&&) = delete;
		RuntimeLayer& operator=(RuntimeLayer&&) = delete;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		void LoadScene(const Project& project, const Window& window);
		bool OnWindowResize(WindowResizeEvent& event);
		bool OnKeyPressed(KeyPressedEvent& event);

	private:
		static constexpr glm::vec4 s_ClearColor{0.15f, 0.15f, 0.15f, 1.0f};
		std::function<void()> m_RequestClose;
		Scope<Scene> m_Scene;
		bool m_RunningScene = false;
	};

}
