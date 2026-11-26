#include "rdpch.h"

#include "EditorInput.h"

#include "EditorCommands.h"
#include "EditorInteractionState.h"
#include "Rod/Core/KeyCodes.h"
#include "Rod/Events/Event.h"

#include "imgui.h" // IWYU pragma: keep

#include "ImGuizmo.h"

namespace Rod {

	EditorInput::EditorInput(EditorCommands& commands, EditorInteractionState& interaction) noexcept
	    : m_Commands(commands), m_Interaction(interaction)
	{}

	void EditorInput::HandleKey(const KeyChanged& key) const
	{
		const bool controlPressed = key.Mods.Control;
		const bool shiftPressed = key.Mods.Shift;

		switch (key.Key)
		{
			case Key::N: {
				if (controlPressed)
					m_Commands.NewScene();

				break;
			}

			case Key::O: {
				if (controlPressed)
					m_Commands.OpenScene();

				break;
			}

			case Key::S: {
				if (controlPressed && shiftPressed)
				{
					m_Commands.SaveSceneAs();
				}
				else if (controlPressed)
				{
					m_Commands.SaveScene();
				}

				break;
			}

			case Key::Q:
				if (!ImGuizmo::IsUsing())
					m_Interaction.GizmoType = -1;
				break;
			case Key::W:
				if (!ImGuizmo::IsUsing())
					m_Interaction.GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Key::E:
				if (!ImGuizmo::IsUsing())
					m_Interaction.GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case Key::R:
				if (!ImGuizmo::IsUsing())
					m_Interaction.GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			default:
				break;
		}
	}

}
