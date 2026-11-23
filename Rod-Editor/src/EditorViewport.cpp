#include "rdpch.h"

#include "EditorViewport.h"

#include "EditorInteractionState.h"
#include "EditorSceneManager.h"
#include "Rod/Core/InputState.h"
#include "Rod/Core/KeyCodes.h"
#include "Rod/Core/MouseButtonCodes.h"
#include "Rod/Events/Event.h"
#include "Rod/Renderer/RenderCommand.h"
#include "Rod/Renderer/Renderer2D.h"

#include "imgui.h"

#include "ImGuizmo.h"

namespace Rod {

	namespace {
		[[nodiscard]] auto ToScreen(glm::vec2 pos) -> glm::vec2
		{
			const ImVec2 origin = ImGui::GetMainViewport()->Pos;
			return pos + glm::vec2{origin.x, origin.y};
		}
	}

	EditorViewport::EditorViewport(EditorInteractionState& interaction, const InputState& input)
	    : m_Interaction(interaction), m_Input(input),
	      m_EditorCamera(s_EditorFOV, s_EditorAspectRatio, s_EditorNearClip, s_EditorFarClip)
	{
		CreateFramebuffer();
	}

	void EditorViewport::HandleButton(const MouseButtonChanged& button, EditorSceneManager& scenes)
	{
		if (!button.Down || !m_Interaction.ViewportHovered || ImGuizmo::IsOver())
			return;

		const auto pos = ToScreen(button.Position);
		if (!Contains(pos))
			return;

		if (button.Mods.Alt && scenes.GetState() == SceneState::Edit)
			m_DragStart = button.Position;
		else if (button.Button == Mouse::ButtonLeft && !button.Mods.Alt)
		{
			m_Framebuffer->Bind();
			m_Interaction.SelectedEntity = Pick(pos, scenes.GetActiveScene());
			m_Framebuffer->Unbind();
		}
	}

	void EditorViewport::HandleScroll(const Scrolled& scroll, SceneState state)
	{
		if (state == SceneState::Edit && m_Interaction.ViewportHovered && Contains(ToScreen(scroll.Position)))
			m_EditorCamera.Zoom(scroll.Offset.y);
	}

	void EditorViewport::OnImGuiRender(
	    EditorSceneManager& scenes,
	    const std::filesystem::path& assetsPath,
	    const std::function<void(const std::filesystem::path&)>& openScene)
	{
		ViewportPanel::OnImGuiRender(
		    {.Framebuffer = *m_Framebuffer,
			 .ActiveScene = scenes.GetActiveScene(),
			 .SelectedEntity = m_Interaction.SelectedEntity,
			 .EditorCamera = m_EditorCamera,
			 .State = scenes.GetState(),
			 .GizmoType = m_Interaction.GizmoType,
			 .Size = m_ViewportSize,
			 .PendingSize = m_PendingViewportSize,
			 .Bounds = m_ViewportBounds,
			 .Focused = m_ViewportFocused,
			 .Hovered = m_Interaction.ViewportHovered,
			 .AssetsPath = assetsPath,
			 .OpenScene = openScene,
			 .WindowInput = m_Input});
	}

	void EditorViewport::CreateFramebuffer()
	{
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = {
		    FramebufferTextureSpecification{FramebufferTextureFormat::RGBA8},
		    FramebufferTextureSpecification{FramebufferTextureFormat::RedInteger},
		    FramebufferTextureSpecification{FramebufferTextureFormat::DEPTH24}};
		fbSpec.Width = static_cast<uint32_t>(s_DefaultViewportSize.x);
		fbSpec.Height = static_cast<uint32_t>(s_DefaultViewportSize.y);
		m_Framebuffer = Framebuffer::Create(fbSpec);
	}

	void EditorViewport::OnUpdate(Timestep ts, EditorSceneManager& scenes)
	{
		ResizeViewportIfNeeded(scenes);

		m_Framebuffer->Bind();
		Renderer2D::ResetStats();
		RenderCommand::SetClearColor(s_ClearColor);
		RenderCommand::Clear();
		m_Framebuffer->ClearColorAttachment(1, -1);

		RenderScene(ts, scenes);
		const auto mouse = ImGui::GetMousePos();
		m_Interaction.HoveredEntity = Pick({mouse.x, mouse.y}, scenes.GetActiveScene());

		m_Framebuffer->Unbind();
	}

	void EditorViewport::ResizeViewportIfNeeded(EditorSceneManager& scenes)
	{
		if (m_ViewportSize != m_PendingViewportSize && m_PendingViewportSize.x >= 1.0f &&
		    m_PendingViewportSize.y >= 1.0f)
		{
			m_ViewportSize = m_PendingViewportSize;
			m_Framebuffer->Resize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			scenes.ResizeViewport(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}
	}

	void EditorViewport::RenderScene(Timestep ts, EditorSceneManager& scenes)
	{
		switch (scenes.GetState())
		{
			case SceneState::Edit: {
				UpdateCamera();

				scenes.GetActiveScene().OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play: {
				m_Dragging = false;
				m_DragStart.reset();
				scenes.GetActiveScene().OnUpdateRuntime(ts);
				break;
			}
		}
	}

	void EditorViewport::UpdateCamera()
	{
		const bool pan = m_Input.IsMouseDown(Mouse::ButtonMiddle);
		const bool orbit = m_Input.IsMouseDown(Mouse::ButtonLeft);
		const bool dolly = m_Input.IsMouseDown(Mouse::ButtonRight);
		const bool active = m_Input.IsKeyDown(Key::LeftAlt) && (pan || orbit || dolly) &&
		                    (m_Dragging || m_DragStart.has_value() || m_Interaction.ViewportHovered);
		const auto delta =
		    m_DragStart ? m_Input.GetCursor() - *m_DragStart : (m_Dragging ? m_Input.GetCursorDelta() : glm::vec2{});
		m_DragStart.reset();
		m_Dragging = active;
		if (!active || delta == glm::vec2{})
			return;

		if (pan)
			m_EditorCamera.Pan(delta);
		else if (orbit)
			m_EditorCamera.Orbit(delta);
		else if (dolly)
			m_EditorCamera.Dolly(delta.y);
	}

	auto EditorViewport::Contains(glm::vec2 pos) const noexcept -> bool
	{
		return pos.x >= m_ViewportBounds[0].x && pos.y >= m_ViewportBounds[0].y && pos.x < m_ViewportBounds[1].x &&
		       pos.y < m_ViewportBounds[1].y;
	}

	auto EditorViewport::Pick(glm::vec2 pos, Scene& scene) const -> Entity
	{
		if (!Contains(pos))
			return {};

		const auto offset = pos - m_ViewportBounds[0];
		const auto& spec = m_Framebuffer->GetSpecification();
		const int x = static_cast<int>(offset.x);
		const int y = static_cast<int>(offset.y);
		if (std::cmp_greater_equal(x, spec.Width) || std::cmp_greater_equal(y, spec.Height))
			return {};

		const int id = m_Framebuffer->ReadPixel(1, x, static_cast<int>(spec.Height) - 1 - y);
		const auto handle = static_cast<entt::entity>(id);
		return id != -1 && scene.IsValidEntityHandle(handle) ? Entity{handle, scene} : Entity{};
	}

}
