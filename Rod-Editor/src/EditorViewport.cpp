#include "rdpch.h"

#include "EditorViewport.h"

#include "EditorInteractionState.h"
#include "EditorSceneManager.h"
#include "Rod/Renderer/RenderCommand.h"
#include "Rod/Renderer/Renderer2D.h"

#include "imgui.h"

namespace Rod {

	EditorViewport::EditorViewport(EditorInteractionState& interaction, const Input& input)
	    : m_Interaction(interaction), m_Input(input),
	      m_EditorCamera(s_EditorFOV, s_EditorAspectRatio, s_EditorNearClip, s_EditorFarClip)
	{
		CreateFramebuffer();
	}

	void EditorViewport::OnEvent(Event& event, SceneState state)
	{
		m_EditorCamera.SetControlsEnabled(state == SceneState::Edit);
		m_EditorCamera.OnEvent(event);
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

	auto EditorViewport::ShouldBlockEvents() const noexcept -> bool
	{
		return !m_ViewportFocused && !m_Interaction.ViewportHovered;
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
		m_EditorCamera.SetControlsEnabled(scenes.GetState() == SceneState::Edit);
		ResizeViewportIfNeeded(scenes);

		m_Framebuffer->Bind();
		Renderer2D::ResetStats();
		RenderCommand::SetClearColor(s_ClearColor);
		RenderCommand::Clear();
		m_Framebuffer->ClearColorAttachment(1, -1);

		RenderScene(ts, scenes);
		UpdateHoveredEntity(scenes);

		m_Framebuffer->Unbind();
	}

	void EditorViewport::ResizeViewportIfNeeded(EditorSceneManager& scenes)
	{
		if (m_ViewportSize != m_PendingViewportSize)
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
				m_EditorCamera.OnUpdate(ts, m_Input);

				scenes.GetActiveScene().OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play: {
				scenes.GetActiveScene().OnUpdateRuntime(ts);
				break;
			}
		}
	}

	void EditorViewport::UpdateHoveredEntity(EditorSceneManager& scenes) const
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		const glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		const int mouseX = static_cast<int>(mx);
		const int mouseY = static_cast<int>(my);
		if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(viewportSize.x) &&
		    mouseY < static_cast<int>(viewportSize.y))
		{
			const int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_Interaction.HoveredEntity =
			    pixelData == -1 ? Entity() : Entity(static_cast<entt::entity>(pixelData), scenes.GetActiveScene());
		}
	}

}
