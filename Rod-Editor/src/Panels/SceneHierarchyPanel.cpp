#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <imgui_internal.h>

#include "Rod/Scene/Components.h"
#include "Rod/Scripting/ScriptEngine.h"

#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <sstream>

namespace Rod {

	extern const std::filesystem::path g_AssetsPath;

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		DrawSceneHierarchy();
		DrawPropertiesPanel();
	}

	void SceneHierarchyPanel::DrawSceneHierarchy()
	{
		ImGui::Begin("Scene Hierarchy");

		for (auto entityID : m_Context->m_Registry.storage<entt::entity>())
		{
			Entity entity{ entityID, m_Context.get() };
			DrawEntityNode(entity);
		}

		HandleHierarchyBlankSpace();

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawPropertiesPanel()
	{
		ImGui::Begin("Properties");

		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		if (!entity)
			return;

		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;

		bool entityDeleted = HandleEntityContextMenu();

		if (opened)
		{
			ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (ImGui::TreeNodeEx((void*)90714394, childFlags, tag.c_str()))
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	void SceneHierarchyPanel::HandleHierarchyBlankSpace()
	{
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity("Empty Entity");
			ImGui::EndPopup();
		}
	}

	bool SceneHierarchyPanel::HandleEntityContextMenu()
	{
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			ImGui::EndPopup();
		}
		return entityDeleted;
	}

	static bool DrawMoreOptionsButton(const char* id, const ImVec2& size)
	{
		ImGui::InvisibleButton(id, size);

		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImU32 background = active ? IM_COL32(40, 45, 53, 255) : hovered ? IM_COL32(50, 56, 66, 255) : IM_COL32(0, 0, 0, 0);
		ImU32 dotColor = IM_COL32(220, 225, 232, 255);

		drawList->AddRectFilled(min, max, background, 3.0f);

		ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
		float spacing = 4.0f;
		float radius = 1.6f;
		drawList->AddCircleFilled(ImVec2(center.x - spacing, center.y), radius, dotColor);
		drawList->AddCircleFilled(center, radius, dotColor);
		drawList->AddCircleFilled(ImVec2(center.x + spacing, center.y), radius, dotColor);

		return ImGui::IsItemClicked(ImGuiMouseButton_Left);
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, const float speed = 0.1f,
		const float minBound = 0.0f, const float maxBound = 0.0f, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, speed, minBound, maxBound, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, speed, minBound, maxBound, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, speed, minBound, maxBound, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
		treeNodeFlags |= ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4.0f, 4.0f });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			auto typeHash = typeid(T).hash_code();
			bool open = ImGui::TreeNodeEx((void*)typeHash, treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.6f);
			std::string popupId = "ComponentSettings" + std::to_string(typeHash);
			std::string buttonId = "##ComponentSettings" + std::to_string(typeid(T).hash_code());
			if (DrawMoreOptionsButton(buttonId.c_str(), ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup(popupId.c_str());
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup(popupId.c_str()))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeComponent = true;
				}
				ImGui::EndPopup();
			}

			if (open)
			{
				ImGui::Dummy(ImVec2(0.0f, 4.0f));
				uiFunction(component);
				ImGui::Dummy(ImVec2(0.0f, 4.0f));
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		DrawTag(entity);
		DrawAddComponentButton(entity);

		DrawTransformComponent(entity);
		DrawCameraComponent(entity);
		DrawSpriteRendererComponent(entity);
		DrawMeshComponent(entity);
		DrawDirectionalLightComponent(entity);
		DrawScriptComponent(entity);
	}

	void SceneHierarchyPanel::DrawTag(Entity entity)
	{
		if (!entity.HasComponent<TagComponent>())
			return;

		auto& tag = entity.GetComponent<TagComponent>().Tag;

		char buffer[256] = {};
		tag.copy(buffer, sizeof(buffer) - 1);

		float addComponentWidth = 132.0f;
		float availableWidth = ImGui::GetContentRegionAvail().x;
		ImGui::PushItemWidth(ImMax(100.0f, availableWidth - addComponentWidth - 10.0f));
		if (ImGui::InputText("##", buffer, sizeof(buffer)))
			tag = std::string(buffer);
		ImGui::PopItemWidth();
	}


	void SceneHierarchyPanel::DrawAddComponentButton(Entity entity)
	{
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button("Add Component", ImVec2(132.0f, 0.0f)))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (ImGui::MenuItem("Camera") && !entity.HasComponent<CameraComponent>())
			{
				entity.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sprite Renderer") && !entity.HasComponent<SpriteRendererComponent>())
			{
				entity.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Mesh") && !entity.HasComponent<MeshComponent>())
			{
				entity.AddComponent<MeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Directional Light") && !entity.HasComponent<DirectionalLightComponent>())
			{
				entity.AddComponent<DirectionalLightComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Script") && !entity.HasComponent<ScriptComponent>())
			{
				entity.AddComponent<ScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		ImGui::Dummy(ImVec2(0.0f, 6.0f));
	}

	void SceneHierarchyPanel::DrawTransformComponent(Entity entity)
	{
		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);

				glm::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);

				DrawVec3Control("Scale", component.Scale, 1.0f);
			});
	}

	void SceneHierarchyPanel::DrawCameraComponent(Entity entity)
	{
		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];

				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &fov))
						camera.SetPerspectiveVerticalFOV(glm::radians(fov));

					float nearClip = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &nearClip))
						camera.SetPerspectiveNearClip(nearClip);

					float farClip = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &farClip))
						camera.SetPerspectiveFarClip(farClip);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float size = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &size))
						camera.SetOrthographicSize(size);

					float nearClip = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near", &nearClip))
						camera.SetOrthographicNearClip(nearClip);

					float farClip = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far", &farClip))
						camera.SetOrthographicFarClip(farClip);

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
				}
			});
	}

	void SceneHierarchyPanel::DrawSpriteRendererComponent(Entity entity)
	{
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

				if (ImGui::Button("Texture", ImVec2(100.0f, 0.0f)))
				{
					component.Texture = nullptr;
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_TEXTURE_ITEM"))
					{
						const char* path = (const char*)payload->Data;
						std::filesystem::path texturePath = std::filesystem::path(g_AssetsPath) / path;
						component.Texture = Texture2D::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
			});
	}

	void SceneHierarchyPanel::DrawMeshComponent(Entity entity)
	{
		DrawComponent<MeshComponent>("Mesh", entity, [](auto& component)
			{
				std::string meshPath = component.Mesh ? component.Mesh->GetPath() : "None";
				ImGui::Text("Asset: %s", meshPath.c_str());

				if (ImGui::Button("Mesh", ImVec2(100.0f, 0.0f)))
				{
					component.Mesh = nullptr;
					return;
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_MESH_ITEM"))
					{
						const char* path = (const char*)payload->Data;
						std::filesystem::path meshAssetPath = std::filesystem::path(g_AssetsPath) / path;
						component.Mesh = Mesh::Create(meshAssetPath.string());
					}
					ImGui::EndDragDropTarget();
				}
				if (!component.Mesh) return;

				if (!component.Mesh->GetMaterial())
					component.Mesh->GetMaterial() = Material::Create();

				auto material = component.Mesh->GetMaterial();
				glm::vec4 albedo = material->GetAlbedo();
				glm::vec3 emissive = material->GetEmissive();
				float roughness = material->GetRoughness();
				float metallic = material->GetMetallic();

				ImGui::ColorEdit4("Albedo", glm::value_ptr(albedo));
				ImGui::ColorEdit3("Emissive", glm::value_ptr(emissive));
				ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Metallic", &metallic, 0.01f, 0.0f, 1.0f);

				material->SetAlbedo(albedo);
				material->SetEmissive(emissive);
				material->SetRoughness(roughness);
				material->SetMetallic(metallic);

			});
	}

	void SceneHierarchyPanel::DrawDirectionalLightComponent(Entity entity)
	{
		DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component)
			{
				DrawVec3Control("Direction", component.Direction, 0.01f, -1.0f, 1.0f);
				component.Direction = glm::normalize(component.Direction);
				ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
				ImGui::DragFloat("Intensity", &component.Intensity, 0.01f, 0.0f, 1.0f);
			});
	}

	static std::string VecToFieldValue(const glm::vec2& value)
	{
		return std::to_string(value.x) + "," + std::to_string(value.y);
	}

	static std::string VecToFieldValue(const glm::vec3& value)
	{
		return std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z);
	}

	static std::string VecToFieldValue(const glm::vec4& value)
	{
		return std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z) + "," + std::to_string(value.w);
	}

	static glm::vec2 FieldValueToVec2(const std::string& value)
	{
		glm::vec2 result(0.0f);
		char separator = '\0';
		std::istringstream stream(value);
		if (!(stream >> result.x >> separator >> result.y) || separator != ',')
			return glm::vec2(0.0f);
		return result;
	}

	static glm::vec3 FieldValueToVec3(const std::string& value)
	{
		glm::vec3 result(0.0f);
		char firstSeparator = '\0';
		char secondSeparator = '\0';
		std::istringstream stream(value);
		if (!(stream >> result.x >> firstSeparator >> result.y >> secondSeparator >> result.z) || firstSeparator != ',' || secondSeparator != ',')
			return glm::vec3(0.0f);
		return result;
	}

	static glm::vec4 FieldValueToVec4(const std::string& value)
	{
		glm::vec4 result(0.0f);
		char firstSeparator = '\0';
		char secondSeparator = '\0';
		char thirdSeparator = '\0';
		std::istringstream stream(value);
		if (!(stream >> result.x >> firstSeparator >> result.y >> secondSeparator >> result.z >> thirdSeparator >> result.w)
			|| firstSeparator != ',' || secondSeparator != ',' || thirdSeparator != ',')
		{
			return glm::vec4(0.0f);
		}
		return result;
	}

	static void SyncScriptFields(ScriptComponent& component)
	{
		ScriptClass* scriptClass = ScriptEngine::GetScriptClass(component.ClassName);
		if (!scriptClass)
			return;

		ScriptFieldMap currentFields = component.Fields;
		component.Fields.clear();

		for (const auto& [name, field] : scriptClass->GetFields())
		{
			auto existingField = currentFields.find(name);
			component.Fields[name] = field;
			if (existingField != currentFields.end() && !existingField->second.Value.empty())
				component.Fields[name].Value = existingField->second.Value;
		}
	}

	static void PushRuntimeScriptField(Entity entity, const std::string& fieldName)
	{
		if (ScriptEngine::IsRuntimeRunning())
			ScriptEngine::SetRuntimeFieldValue(entity, fieldName);
	}

	void SceneHierarchyPanel::DrawScriptComponent(Entity entity)
	{
		DrawComponent<ScriptComponent>("Script", entity, [entity](auto& component)
			{
				const auto& scriptClasses = ScriptEngine::GetScriptClasses();
				const char* currentClassName = component.ClassName.empty() ? "None" : component.ClassName.c_str();

				if (ImGui::BeginCombo("Class", currentClassName))
				{
					for (const auto& [className, scriptClass] : scriptClasses)
					{
						bool isSelected = component.ClassName == className;
						if (ImGui::Selectable(className.c_str(), isSelected))
						{
							component.ClassName = className;
							SyncScriptFields(component);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (!component.ClassName.empty())
					SyncScriptFields(component);

				for (auto& [name, field] : component.Fields)
				{
					switch (field.Field.Type)
					{
						case ScriptFieldType::Float:
						{
							float value = field.Value.empty() ? 0.0f : std::stof(field.Value);
							if (ImGui::DragFloat(name.c_str(), &value, 0.1f))
							{
								field.Value = std::to_string(value);
								PushRuntimeScriptField(entity, name);
							}
							break;
						}
						case ScriptFieldType::Double:
						{
							float value = field.Value.empty() ? 0.0f : (float)std::stod(field.Value);
							if (ImGui::DragFloat(name.c_str(), &value, 0.1f))
							{
								field.Value = std::to_string(value);
								PushRuntimeScriptField(entity, name);
							}
							break;
						}
						case ScriptFieldType::Bool:
						{
							bool value = field.Value == "true" || field.Value == "1";
							if (ImGui::Checkbox(name.c_str(), &value))
							{
								field.Value = value ? "true" : "false";
								PushRuntimeScriptField(entity, name);
							}
							break;
						}
						case ScriptFieldType::Char:
						case ScriptFieldType::Byte:
						case ScriptFieldType::Short:
						case ScriptFieldType::Int:
						case ScriptFieldType::Long:
						case ScriptFieldType::UByte:
						case ScriptFieldType::UShort:
						case ScriptFieldType::UInt:
						case ScriptFieldType::ULong:
						case ScriptFieldType::Entity:
						{
							int value = field.Value.empty() ? 0 : std::stoi(field.Value);
							if (ImGui::DragInt(name.c_str(), &value))
							{
								field.Value = std::to_string(value);
								PushRuntimeScriptField(entity, name);
							}
							break;
						}
						case ScriptFieldType::Vector2:
						{
							glm::vec2 value = FieldValueToVec2(field.Value);
							if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(value), 0.1f))
							{
								field.Value = VecToFieldValue(value);
								PushRuntimeScriptField(entity, name);
							}
							break;
						}
						case ScriptFieldType::Vector3:
						{
							glm::vec3 value = FieldValueToVec3(field.Value);
							if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(value), 0.1f))
							{
								field.Value = VecToFieldValue(value);
								PushRuntimeScriptField(entity, name);
							}
							break;
						}
						case ScriptFieldType::Vector4:
						{
							glm::vec4 value = FieldValueToVec4(field.Value);
							if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(value), 0.1f))
							{
								field.Value = VecToFieldValue(value);
								PushRuntimeScriptField(entity, name);
							}
							break;
						}
						case ScriptFieldType::String:
						{
							char buffer[256] = {};
							field.Value.copy(buffer, sizeof(buffer) - 1);
							if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
							{
								field.Value = buffer;
								PushRuntimeScriptField(entity, name);
							}
							break;
						}
						default:
							ImGui::Text("%s: unsupported type '%s'", name.c_str(), field.Field.TypeName.c_str());
							break;
					}
				}
			});
	}
}
