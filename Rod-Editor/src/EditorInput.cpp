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

	EditorInput::EditorInput(EditorCommands& commands, EditorInteractionState& interaction, const Input& input) noexcept
	    : m_Commands(commands), m_Interaction(interaction), m_Input(input)
	{}

	void EditorInput::OnEvent(Event& event) const
	{
		HandleEvent<KeyPressedEvent>(event, [this](const KeyPressedEvent& keyEvent) { return OnKeyPressed(keyEvent); });
		HandleEvent<MouseButtonPressedEvent>(event, [this](const MouseButtonPressedEvent& mouseEvent) {
			return OnMouseButtonPressed(mouseEvent);
		});
	}

	auto EditorInput::OnKeyPressed(const KeyPressedEvent& e) const -> bool
	{
		// Shortcuts
		if (e.IsRepeat())
			return false;

		const bool controlPressed = m_Input.IsKeyPressed(Key::LeftControl) || m_Input.IsKeyPressed(Key::RightControl);
		const bool shiftPressed = m_Input.IsKeyPressed(Key::LeftShift) || m_Input.IsKeyPressed(Key::RightShift);

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
			default:
				break;
		}

		return false;
	}

	auto EditorInput::OnMouseButtonPressed(const MouseButtonPressedEvent& e) const -> bool
	{
		if (e.GetMouseButton() == Mouse::Button0 && m_Interaction.ViewportHovered && !ImGuizmo::IsOver() &&
		    !m_Input.IsKeyPressed(Key::LeftAlt))
		{
			m_Interaction.SelectedEntity = m_Interaction.HoveredEntity;
		}
		return false;
	}

}
