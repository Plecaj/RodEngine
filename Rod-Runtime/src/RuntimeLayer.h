#pragma once

#include "Rod.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/KeyEvent.h"

namespace Rod {

	class RuntimeLayer : public Layer
	{
	public:
		explicit RuntimeLayer(const Project& project);
		~RuntimeLayer() override;
		RuntimeLayer(const RuntimeLayer&) = delete;
		RuntimeLayer& operator=(const RuntimeLayer&) = delete;
		RuntimeLayer(RuntimeLayer&&) = delete;
		RuntimeLayer& operator=(RuntimeLayer&&) = delete;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		void LoadScene();
		bool OnWindowResize(WindowResizeEvent& event);
		static bool OnKeyPressed(KeyPressedEvent& event) noexcept;

	private:
		const Project& m_Project;
		static constexpr glm::vec4 CLEAR_COLOR{0.15f, 0.15f, 0.15f, 1.0f};
		Scope<Scene> m_Scene;
		bool m_RunningScene = false;
	};

}
