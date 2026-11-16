#pragma once

#include "Rod.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/KeyEvent.h"

namespace Rod {

	class RuntimeLayer : public Layer
	{
	public:
		explicit RuntimeLayer(const Project& project);

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		bool OnWindowResize(WindowResizeEvent& event);
		bool OnKeyPressed(KeyPressedEvent& event);

	private:
		const Project& m_Project;
		static constexpr glm::vec4 CLEAR_COLOR{0.15f, 0.15f, 0.15f, 1.0f};
		Scope<Scene> m_Scene;
		bool m_RunningScene = false;
	};

}
