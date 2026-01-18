#include "ViewportPanel.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Rod/Core/Application.h"
#include "Rod/Scene/Components.h"
#include "Rod/Math/Math.h"
#include "Rod/Core/Input.h"
#include "Rod/Events/KeyEvent.h"
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

#include "../SceneState.h"

namespace Rod {

	void ViewportPanel::OnImGuiRender(
		Ref<Framebuffer> framebuffer,
		Ref<Scene> activeScene,
		Entity selectedEntity,
		EditorCamera& editorCamera,
		SceneState sceneState,
		int& guizmoType,
		glm::vec2& viewportSize,
		glm::vec2& pendingViewportSize,
		glm::vec2 viewportBounds[2],
		bool& viewportFocused,
		bool& viewportHovered,
		const std::filesystem::path& assetsPath,
		std::function<void(const std::filesystem::path&)> onOpenScene)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		auto viewportOffset = ImGui::GetCursorPos();

		viewportFocused = ImGui::IsWindowFocused();
		viewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!viewportFocused && !viewportHovered);

		uint32_t textureID = framebuffer->GetColorAttachmentRendererID();
		ImVec2 viewPortPanelSize = ImGui::GetContentRegionAvail();
		pendingViewportSize = { viewPortPanelSize.x, viewPortPanelSize.y };

		ImGui::Image((uint64_t)textureID, ImVec2{ viewportSize.x, viewportSize.y }, { 0.0f, 1.0f }, { 1.0f, 0.0f });

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				if (onOpenScene)
					onOpenScene(std::filesystem::path(assetsPath) / path);
			}
			ImGui::EndDragDropTarget();
		}

		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;
		ImVec2 maxBound = { minBound.x + viewportSize.x, minBound.y + viewportSize.y };
		viewportBounds[0] = { minBound.x, minBound.y };
		viewportBounds[1] = { maxBound.x, maxBound.y };

		if (selectedEntity && guizmoType != -1)
		{
			glm::mat4 cameraView, cameraProjection;

			if (sceneState == SceneState::Edit)
			{
				cameraProjection = editorCamera.GetProjection();
				cameraView = editorCamera.GetViewMatrix();
			}
			else if (sceneState == SceneState::Play)
			{
				auto cameraEntity = activeScene->GetPrimaryCameraEntity();
				if (cameraEntity)
				{
					auto& cameraComp = cameraEntity.GetComponent<CameraComponent>().Camera;
					cameraProjection = cameraComp.GetProjection();

					const auto& transform = cameraEntity.GetComponent<TransformComponent>().GetTransform();
					cameraView = glm::inverse(transform);
				}
			}

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(
				viewportBounds[0].x,
				viewportBounds[0].y,
				viewportBounds[1].x - viewportBounds[0].x,
				viewportBounds[1].y - viewportBounds[0].y
			);

			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = (guizmoType == ImGuizmo::OPERATION::ROTATE) ? 45.0f : 0.5f;
			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(
				glm::value_ptr(cameraView),
				glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)guizmoType,
				ImGuizmo::MODE::LOCAL,
				glm::value_ptr(transform),
				nullptr,
				snap ? snapValues : nullptr
			);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				tc.Translation = translation;
				tc.Rotation = rotation;
				tc.Scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

}
