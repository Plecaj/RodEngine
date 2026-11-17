#pragma once

#include "EditorCommands.h"
#include "EditorInput.h"
#include "EditorInteractionState.h"
#include "EditorLayout.h"
#include "EditorSceneManager.h"
#include "EditorViewport.h"
#include "Rod/Core/Layer.h"

namespace Rod {

	class EditorLayer : public Layer
	{
	public:
		explicit EditorLayer(const Project& project);
		~EditorLayer() override;
		EditorLayer(const EditorLayer&) = delete;
		EditorLayer& operator=(const EditorLayer&) = delete;
		EditorLayer(EditorLayer&&) = delete;
		EditorLayer& operator=(EditorLayer&&) = delete;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		EditorSceneManager m_Scenes;
		EditorInteractionState m_Interaction;
		EditorViewport m_Viewport;
		EditorCommands m_Commands;
		EditorLayout m_Layout;
		EditorInput m_Input;
		float m_LastDeltaTime = 0.0f;
	};

}
