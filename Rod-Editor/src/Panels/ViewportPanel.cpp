#include "rdpch.h"

#include "ViewportPanel.h"

#include "Rod/Core/Application.h"
#include "Rod/Core/Input.h"
#include "Rod/Math/Math.h"
#include "Rod/Scene/Components.h"

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

#include "ImGuizmo.h"

namespace Rod {

	void ViewportPanel::OnImGuiRender(const RenderContext& context)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar);
		const auto viewportOffset = ImGui::GetCursorPos();
		context.Focused = ImGui::IsWindowFocused();
		context.Hovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!context.Focused && !context.Hovered);

		const auto panelSize = ImGui::GetContentRegionAvail();
		context.PendingSize = {panelSize.x, panelSize.y};
		ImGui::Image(
		    static_cast<ImTextureID>(context.Framebuffer.GetColorAttachmentRendererID()),
		    {context.Size.x, context.Size.y},
		    {0.0f, 1.0f},
		    {1.0f, 0.0f});
		const auto sceneToOpen = AcceptSceneDrop(context.AssetsPath);
		const ImVec2 windowPosition = ImGui::GetWindowPos();
		context.Bounds[0] = {windowPosition.x + viewportOffset.x, windowPosition.y + viewportOffset.y};
		context.Bounds[1] = context.Bounds[0] + context.Size;
		DrawGizmo(context);

		ImGui::End();
		ImGui::PopStyleVar();
		if (!sceneToOpen.empty() && context.OpenScene)
			context.OpenScene(sceneToOpen);
	}

	std::filesystem::path ViewportPanel::AcceptSceneDrop(const std::filesystem::path& assetsPath)
	{
		std::filesystem::path sceneToOpen;
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE_ITEM"))
				sceneToOpen = assetsPath / static_cast<const char*>(payload->Data);
			ImGui::EndDragDropTarget();
		}
		return sceneToOpen;
	}

	bool ViewportPanel::GetCameraMatrices(const RenderContext& context, glm::mat4& view, glm::mat4& projection)
	{
		if (context.State == SceneState::Edit)
		{
			projection = context.EditorCamera.GetProjection();
			view = context.EditorCamera.GetViewMatrix();
			return true;
		}
		const auto cameraEntity = context.ActiveScene.GetPrimaryCameraEntity();
		if (!cameraEntity)
			return false;
		projection = cameraEntity.GetComponent<CameraComponent>().Camera.GetProjection();
		view = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());
		return true;
	}

	void ViewportPanel::DrawGizmo(const RenderContext& context)
	{
		Entity selectedEntity = context.SelectedEntity;
		if (!selectedEntity || context.GizmoType == -1)
			return;
		glm::mat4 cameraView, cameraProjection;
		if (!GetCameraMatrices(context, cameraView, cameraProjection))
			return;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(
		    context.Bounds[0].x,
		    context.Bounds[0].y,
		    context.Bounds[1].x - context.Bounds[0].x,
		    context.Bounds[1].y - context.Bounds[0].y);
		auto& component = selectedEntity.GetComponent<TransformComponent>();
		glm::mat4 transform = component.GetTransform();
		const bool snap = Input::IsKeyPressed(Key::LeftControl);
		const float snapValue = context.GizmoType == ImGuizmo::OPERATION::ROTATE ? ROTATION_SNAP : TRANSLATION_SNAP;
		const std::array<float, VECTOR_COMPONENT_COUNT> snapValues{snapValue, snapValue, snapValue};
		ImGuizmo::Manipulate(
		    glm::value_ptr(cameraView),
		    glm::value_ptr(cameraProjection),
		    static_cast<ImGuizmo::OPERATION>(context.GizmoType),
		    ImGuizmo::MODE::LOCAL,
		    glm::value_ptr(transform),
		    nullptr,
		    snap ? snapValues.data() : nullptr);
		if (ImGuizmo::IsUsing())
		{
			glm::vec3 translation, rotation, scale;
			if (Math::DecomposeTransform(transform, translation, rotation, scale))
			{
				component.Translation = translation;
				component.Rotation = rotation;
				component.Scale = scale;
			}
		}
	}
}
