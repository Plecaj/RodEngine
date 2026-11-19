#pragma once

#include "EditorCommands.h"
#include "EditorInput.h"
#include "EditorInteractionState.h"
#include "EditorLayout.h"
#include "EditorSceneManager.h"
#include "EditorViewport.h"
#include "Rod/Core/Layer.h"

#include <functional>

namespace Rod {

	class ImGuiLayer;
	class Input;
	class Project;
	class Window;

	class EditorLayer : public Layer
	{
	public:
		EditorLayer(
		    const Project& project,
		    const Window& window,
		    const Input& input,
		    ImGuiLayer& imGuiLayer,
		    std::function<void()> requestClose);
		~EditorLayer() override;
		EditorLayer(const EditorLayer&) = delete;
		EditorLayer& operator=(const EditorLayer&) = delete;
		EditorLayer(EditorLayer&&) = delete;
		EditorLayer& operator=(EditorLayer&&) = delete;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		ImGuiLayer& m_ImGuiLayer;
		EditorSceneManager m_Scenes;
		EditorInteractionState m_Interaction;
		EditorViewport m_Viewport;
		EditorCommands m_Commands;
		EditorLayout m_Layout;
		EditorInput m_Input;
		float m_LastDeltaTime = 0.0f;
	};

}
