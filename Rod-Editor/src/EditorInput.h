#pragma once

#include "Rod/Events/Event.h"
#include "Rod/Events/KeyEvent.h"
#include "Rod/Events/MouseEvent.h"

namespace Rod {

	class EditorCommands;
	struct EditorInteractionState;

	class EditorInput
	{
	public:
		EditorInput(EditorCommands& commands, EditorInteractionState& interaction) noexcept;
		void OnEvent(Event& event) const;

	private:
		[[nodiscard]] bool OnKeyPressed(const KeyPressedEvent& event) const;
		[[nodiscard]] bool OnMouseButtonPressed(const MouseButtonPressedEvent& event) const;

		EditorCommands& m_Commands;
		EditorInteractionState& m_Interaction;
	};

}
