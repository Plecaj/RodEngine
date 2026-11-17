#include "rdpch.h"

#include "EditorInput.h"

#include "EditorCommands.h"
#include "EditorInteractionState.h"
#include "Rod/Core/Input.h"
#include "Rod/Core/KeyCodes.h"
#include "Rod/Core/MouseButtonCodes.h"

#include "imgui.h" // IWYU pragma: keep

#include "ImGuizmo.h"

namespace Rod {

	EditorInput::EditorInput(EditorCommands& commands, EditorInteractionState& interaction) noexcept
	    : m_Commands(commands), m_Interaction(interaction)
	{}

	void EditorInput::OnEvent(Event& event) const
	{
		HandleEvent<KeyPressedEvent>(event, [this](const KeyPressedEvent& keyEvent) { return OnKeyPressed(keyEvent); });
		HandleEvent<MouseButtonPressedEvent>(event, [this](const MouseButtonPressedEvent& mouseEvent) {
			return OnMouseButtonPressed(mouseEvent);
		});
	}

	bool EditorInput::OnKeyPressed(const KeyPressedEvent& e) const
	{
		// Shortcuts
		if (e.IsRepeat())
			return false;

		const bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		const bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
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
					m_Commands.SaveSceneAs();

				else if (controlPressed)
					m_Commands.SaveScene();

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
		}

		return false;
	}

	bool EditorInput::OnMouseButtonPressed(const MouseButtonPressedEvent& e) const
	{
		if (e.GetMouseButton() == Mouse::Button0 && m_Interaction.ViewportHovered && !ImGuizmo::IsOver() &&
		    !Input::IsKeyPressed(Key::LeftAlt))
		{
			m_Interaction.SelectedEntity = m_Interaction.HoveredEntity;
		}
		return false;
	}

}
