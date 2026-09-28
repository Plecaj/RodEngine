#pragma once

#include "Rod.h"
#include "Rod/Events/ApplicationEvent.h"
#include "Rod/Events/KeyEvent.h"

namespace Rod {

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer() = default;
		~RuntimeLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& event) override;

	private:
		bool OnWindowResize(WindowResizeEvent& event);
		bool OnKeyPressed(KeyPressedEvent& event);
		std::filesystem::path ResolveStartupScenePath() const;

	private:
		Ref<Scene> m_Scene;
		bool m_RunningScene = false;
	};

}

