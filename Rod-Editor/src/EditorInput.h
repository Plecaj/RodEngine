#pragma once

namespace Rod {

	class EditorCommands;
	struct EditorInteractionState;
	struct KeyChanged;

	class EditorInput
	{
	public:
		EditorInput(EditorCommands& commands, EditorInteractionState& interaction) noexcept;
		void HandleKey(const KeyChanged& key) const;

	private:
		EditorCommands& m_Commands;
		EditorInteractionState& m_Interaction;
	};

}
