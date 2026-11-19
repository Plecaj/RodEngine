#pragma once

#include "Rod/Events/Event.h"
#include "Rod/Events/KeyEvent.h"
#include "Rod/Events/MouseEvent.h"

namespace Rod {

	class EditorCommands;
	class Input;
	struct EditorInteractionState;

	class EditorInput
	{
	public:
		EditorInput(EditorCommands& commands, EditorInteractionState& interaction, const Input& input) noexcept;
		void OnEvent(Event& event) const;

	private:
		[[nodiscard]] auto OnKeyPressed(const KeyPressedEvent& event) const -> bool;
		[[nodiscard]] auto OnMouseButtonPressed(const MouseButtonPressedEvent& event) const -> bool;

		EditorCommands& m_Commands;
		EditorInteractionState& m_Interaction;
		const Input& m_Input;
	};

}
