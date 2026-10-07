#include "rdpch.h"
#include "ScriptGlue.h"

#include <cstdlib>
#include <cstring>

#include "Rod/Core/Input.h"
#include "Rod/Renderer/Mesh.h"
#include "Rod/Renderer/Texture.h"
#include "Rod/Scene/Components.h"

#ifdef RD_PLATFORM_WINDOWS
	#include <objbase.h>
#endif

namespace Rod {

	struct ScriptVec2 { float X, Y; };
	struct ScriptVec3 { float X, Y, Z; };
	struct ScriptVec4 { float X, Y, Z, W; };

	enum class ScriptComponentType : int32_t
	{
		ID = 0,
		Tag,
		Transform,
		Camera,
		SpriteRenderer,
		Mesh,
		DirectionalLight,
		Script
	};

	Scene* ScriptGlue::s_SceneContext = nullptr;

	static Entity GetEntity(uint64_t entityID)
	{
		Scene* scene = ScriptGlue::GetSceneContext();
		if (!scene)
			return {};
		return scene->FindEntityByUUID(entityID);
	}

	static void* AllocateString(const std::string& value)
	{
#ifdef RD_PLATFORM_WINDOWS
		char* buffer = (char*)CoTaskMemAlloc(value.size() + 1);
#else
		char* buffer = (char*)std::malloc(value.size() + 1);
#endif
		if (buffer)
			std::memcpy(buffer, value.c_str(), value.size() + 1);
		return buffer;
	}

	static glm::vec3 ToGLM(ScriptVec3 value)
	{
		return { value.X, value.Y, value.Z };
	}

	static glm::vec4 ToGLM(ScriptVec4 value)
	{
		return { value.X, value.Y, value.Z, value.W };
	}

	static ScriptVec3 ToScript(const glm::vec3& value)
	{
		return { value.x, value.y, value.z };
	}

	static ScriptVec4 ToScript(const glm::vec4& value)
	{
		return { value.x, value.y, value.z, value.w };
	}

	extern "C" {

	static void RD_CDECL Log_Message(int32_t level, const char* message)
	{
		switch (level)
		{
			case 0: RD_TRACE("{}", message); break;
			case 1: RD_INFO("{}", message); break;
			case 2: RD_WARN("{}", message); break;
			case 3: RD_ERROR("{}", message); break;
			default: RD_CORE_INFO("{}", message); break;
		}
	}

	static bool RD_CDECL Input_IsKeyPressed(int32_t keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	static bool RD_CDECL Input_IsMouseButtonPressed(int32_t button)
	{
		return Input::IsMouseButtonPressed(button);
	}

	static ScriptVec2 RD_CDECL Input_GetMousePosition()
	{
		auto [x, y] = Input::GetMousePosition();
		return { x, y };
	}

	static uint64_t RD_CDECL Entity_Create(const char* name)
	{
		Scene* scene = ScriptGlue::GetSceneContext();
		if (!scene)
			return 0;
		Entity entity = scene->CreateEntity(name ? name : "");
		return entity.GetUUID();
	}

	static void RD_CDECL Entity_Destroy(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		if (entity)
			ScriptGlue::GetSceneContext()->DestroyEntity(entity);
	}

	static uint64_t RD_CDECL Entity_FindByName(const char* name)
	{
		Scene* scene = ScriptGlue::GetSceneContext();
		if (!scene || !name)
			return 0;
		Entity entity = scene->FindEntityByName(name);
		return entity ? (uint64_t)entity.GetUUID() : 0;
	}

	static void* RD_CDECL Entity_GetName(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return AllocateString(entity ? entity.GetName() : "");
	}

	static void RD_CDECL Entity_SetName(uint64_t entityID, const char* name)
	{
		Entity entity = GetEntity(entityID);
		if (entity && name)
			entity.GetComponent<TagComponent>().Tag = name;
	}

	static bool RD_CDECL Entity_HasComponent(uint64_t entityID, int32_t componentType)
	{
		Entity entity = GetEntity(entityID);
		if (!entity)
			return false;

		switch ((ScriptComponentType)componentType)
		{
			case ScriptComponentType::ID: return entity.HasComponent<IDComponent>();
			case ScriptComponentType::Tag: return entity.HasComponent<TagComponent>();
			case ScriptComponentType::Transform: return entity.HasComponent<TransformComponent>();
			case ScriptComponentType::Camera: return entity.HasComponent<CameraComponent>();
			case ScriptComponentType::SpriteRenderer: return entity.HasComponent<SpriteRendererComponent>();
			case ScriptComponentType::Mesh: return entity.HasComponent<MeshComponent>();
			case ScriptComponentType::DirectionalLight: return entity.HasComponent<DirectionalLightComponent>();
			case ScriptComponentType::Script: return entity.HasComponent<ScriptComponent>();
		}
		return false;
	}

	static void RD_CDECL Entity_AddComponent(uint64_t entityID, int32_t componentType)
	{
		Entity entity = GetEntity(entityID);
		if (!entity)
			return;

		switch ((ScriptComponentType)componentType)
		{
			case ScriptComponentType::Camera: if (!entity.HasComponent<CameraComponent>()) entity.AddComponent<CameraComponent>(); break;
			case ScriptComponentType::SpriteRenderer: if (!entity.HasComponent<SpriteRendererComponent>()) entity.AddComponent<SpriteRendererComponent>(); break;
			case ScriptComponentType::Mesh: if (!entity.HasComponent<MeshComponent>()) entity.AddComponent<MeshComponent>(); break;
			case ScriptComponentType::DirectionalLight: if (!entity.HasComponent<DirectionalLightComponent>()) entity.AddComponent<DirectionalLightComponent>(); break;
			case ScriptComponentType::Script: if (!entity.HasComponent<ScriptComponent>()) entity.AddComponent<ScriptComponent>(); break;
			default: break;
		}
	}

	static void RD_CDECL Entity_RemoveComponent(uint64_t entityID, int32_t componentType)
	{
		Entity entity = GetEntity(entityID);
		if (!entity)
			return;

		switch ((ScriptComponentType)componentType)
		{
			case ScriptComponentType::Camera: if (entity.HasComponent<CameraComponent>()) entity.RemoveComponent<CameraComponent>(); break;
			case ScriptComponentType::SpriteRenderer: if (entity.HasComponent<SpriteRendererComponent>()) entity.RemoveComponent<SpriteRendererComponent>(); break;
			case ScriptComponentType::Mesh: if (entity.HasComponent<MeshComponent>()) entity.RemoveComponent<MeshComponent>(); break;
			case ScriptComponentType::DirectionalLight: if (entity.HasComponent<DirectionalLightComponent>()) entity.RemoveComponent<DirectionalLightComponent>(); break;
			case ScriptComponentType::Script: if (entity.HasComponent<ScriptComponent>()) entity.RemoveComponent<ScriptComponent>(); break;
			default: break;
		}
	}

	static bool RD_CDECL Camera_GetPrimary(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().Primary : false;
	}

	static void RD_CDECL Camera_SetPrimary(uint64_t entityID, bool value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Primary = value;
	}

	static bool RD_CDECL Camera_GetFixedAspectRatio(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().FixedAspectRatio : false;
	}

	static void RD_CDECL Camera_SetFixedAspectRatio(uint64_t entityID, bool value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().FixedAspectRatio = value;
	}

	static int32_t RD_CDECL Camera_GetProjectionType(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? (int32_t)entity.GetComponent<CameraComponent>().Camera.GetProjectionType() : 0;
	}

	static void RD_CDECL Camera_SetProjectionType(uint64_t entityID, int32_t value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetProjectionType((SceneCamera::ProjectionType)value);
	}

	static float RD_CDECL Camera_GetPerspectiveVerticalFOV(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().Camera.GetPerspectiveVerticalFOV() : 0.0f;
	}

	static void RD_CDECL Camera_SetPerspectiveVerticalFOV(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetPerspectiveVerticalFOV(value);
	}

	static float RD_CDECL Camera_GetPerspectiveNearClip(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().Camera.GetPerspectiveNearClip() : 0.0f;
	}

	static void RD_CDECL Camera_SetPerspectiveNearClip(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetPerspectiveNearClip(value);
	}

	static float RD_CDECL Camera_GetPerspectiveFarClip(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().Camera.GetPerspectiveFarClip() : 0.0f;
	}

	static void RD_CDECL Camera_SetPerspectiveFarClip(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetPerspectiveFarClip(value);
	}

	static float RD_CDECL Camera_GetOrthographicSize(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().Camera.GetOrthographicSize() : 0.0f;
	}

	static void RD_CDECL Camera_SetOrthographicSize(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetOrthographicSize(value);
	}

	static float RD_CDECL Camera_GetOrthographicNearClip(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().Camera.GetOrthographicNearClip() : 0.0f;
	}

	static void RD_CDECL Camera_SetOrthographicNearClip(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetOrthographicNearClip(value);
	}

	static float RD_CDECL Camera_GetOrthographicFarClip(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().Camera.GetOrthographicFarClip() : 0.0f;
	}

	static void RD_CDECL Camera_SetOrthographicFarClip(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetOrthographicFarClip(value);
	}

	static ScriptVec3 RD_CDECL Transform_GetTranslation(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<TransformComponent>() ? ToScript(entity.GetComponent<TransformComponent>().Translation) : ScriptVec3{};
	}

	static void RD_CDECL Transform_SetTranslation(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<TransformComponent>())
			entity.GetComponent<TransformComponent>().Translation = ToGLM(value);
	}

	static ScriptVec3 RD_CDECL Transform_GetRotation(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<TransformComponent>() ? ToScript(entity.GetComponent<TransformComponent>().Rotation) : ScriptVec3{};
	}

	static void RD_CDECL Transform_SetRotation(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<TransformComponent>())
			entity.GetComponent<TransformComponent>().Rotation = ToGLM(value);
	}

	static ScriptVec3 RD_CDECL Transform_GetScale(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<TransformComponent>() ? ToScript(entity.GetComponent<TransformComponent>().Scale) : ScriptVec3{ 1.0f, 1.0f, 1.0f };
	}

	static void RD_CDECL Transform_SetScale(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<TransformComponent>())
			entity.GetComponent<TransformComponent>().Scale = ToGLM(value);
	}

	static ScriptVec4 RD_CDECL SpriteRenderer_GetColor(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<SpriteRendererComponent>() ? ToScript(entity.GetComponent<SpriteRendererComponent>().Color) : ScriptVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
	}

	static void RD_CDECL SpriteRenderer_SetColor(uint64_t entityID, ScriptVec4 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<SpriteRendererComponent>())
			entity.GetComponent<SpriteRendererComponent>().Color = ToGLM(value);
	}

	static float RD_CDECL SpriteRenderer_GetTilingFactor(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<SpriteRendererComponent>() ? entity.GetComponent<SpriteRendererComponent>().TilingFactor : 1.0f;
	}

	static void RD_CDECL SpriteRenderer_SetTilingFactor(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<SpriteRendererComponent>())
			entity.GetComponent<SpriteRendererComponent>().TilingFactor = value;
	}

	static void* RD_CDECL SpriteRenderer_GetTexturePath(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<SpriteRendererComponent>() || !entity.GetComponent<SpriteRendererComponent>().Texture)
			return AllocateString("");
		return AllocateString(entity.GetComponent<SpriteRendererComponent>().Texture->GetPath());
	}

	static void RD_CDECL SpriteRenderer_SetTexturePath(uint64_t entityID, const char* path)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<SpriteRendererComponent>())
			return;
		auto& sprite = entity.GetComponent<SpriteRendererComponent>();
		sprite.Texture = path && path[0] ? Texture2D::Create(path) : nullptr;
	}

	static void* RD_CDECL Mesh_GetPath(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<MeshComponent>() || !entity.GetComponent<MeshComponent>().Mesh)
			return AllocateString("");
		return AllocateString(entity.GetComponent<MeshComponent>().Mesh->GetPath());
	}

	static void RD_CDECL Mesh_SetPath(uint64_t entityID, const char* path)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<MeshComponent>())
			return;
		auto& mesh = entity.GetComponent<MeshComponent>();
		mesh.Mesh = path && path[0] ? Mesh::Create(path) : nullptr;
	}

	static ScriptVec4 RD_CDECL Mesh_GetAlbedo(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<MeshComponent>() || !entity.GetComponent<MeshComponent>().Mesh)
			return { 1.0f, 1.0f, 1.0f, 1.0f };
		return ToScript(entity.GetComponent<MeshComponent>().Mesh->GetMaterial()->GetAlbedo());
	}

	static void RD_CDECL Mesh_SetAlbedo(uint64_t entityID, ScriptVec4 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh)
			entity.GetComponent<MeshComponent>().Mesh->GetMaterial()->SetAlbedo(ToGLM(value));
	}

	static ScriptVec3 RD_CDECL Mesh_GetEmissive(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<MeshComponent>() || !entity.GetComponent<MeshComponent>().Mesh)
			return {};
		return ToScript(entity.GetComponent<MeshComponent>().Mesh->GetMaterial()->GetEmissive());
	}

	static void RD_CDECL Mesh_SetEmissive(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh)
			entity.GetComponent<MeshComponent>().Mesh->GetMaterial()->SetEmissive(ToGLM(value));
	}

	static float RD_CDECL Mesh_GetRoughness(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh ? entity.GetComponent<MeshComponent>().Mesh->GetMaterial()->GetRoughness() : 1.0f;
	}

	static void RD_CDECL Mesh_SetRoughness(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh)
			entity.GetComponent<MeshComponent>().Mesh->GetMaterial()->SetRoughness(value);
	}

	static float RD_CDECL Mesh_GetMetallic(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh ? entity.GetComponent<MeshComponent>().Mesh->GetMaterial()->GetMetallic() : 0.0f;
	}

	static void RD_CDECL Mesh_SetMetallic(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh)
			entity.GetComponent<MeshComponent>().Mesh->GetMaterial()->SetMetallic(value);
	}

	static ScriptVec3 RD_CDECL DirectionalLight_GetDirection(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<DirectionalLightComponent>() ? ToScript(entity.GetComponent<DirectionalLightComponent>().Direction) : ScriptVec3{};
	}

	static void RD_CDECL DirectionalLight_SetDirection(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<DirectionalLightComponent>())
			entity.GetComponent<DirectionalLightComponent>().Direction = glm::normalize(ToGLM(value));
	}

	static ScriptVec3 RD_CDECL DirectionalLight_GetColor(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<DirectionalLightComponent>() ? ToScript(entity.GetComponent<DirectionalLightComponent>().Color) : ScriptVec3{ 1.0f, 1.0f, 1.0f };
	}

	static void RD_CDECL DirectionalLight_SetColor(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<DirectionalLightComponent>())
			entity.GetComponent<DirectionalLightComponent>().Color = ToGLM(value);
	}

	static float RD_CDECL DirectionalLight_GetIntensity(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<DirectionalLightComponent>() ? entity.GetComponent<DirectionalLightComponent>().Intensity : 1.0f;
	}

	static void RD_CDECL DirectionalLight_SetIntensity(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<DirectionalLightComponent>())
			entity.GetComponent<DirectionalLightComponent>().Intensity = value;
	}

	static void* RD_CDECL Script_GetClassName(uint64_t entityID)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<ScriptComponent>())
			return AllocateString("");
		return AllocateString(entity.GetComponent<ScriptComponent>().ClassName);
	}

	static void RD_CDECL Script_SetClassName(uint64_t entityID, const char* className)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<ScriptComponent>())
			entity.GetComponent<ScriptComponent>().ClassName = className ? className : "";
	}

	}

	void ScriptGlue::SetSceneContext(Scene* scene)
	{
		s_SceneContext = scene;
	}

	std::vector<NativeCall> ScriptGlue::GetNativeCalls()
	{
		return {
			{ "Log.Message", (void*)Log_Message },
			{ "Input.IsKeyPressed", (void*)Input_IsKeyPressed },
			{ "Input.IsMouseButtonPressed", (void*)Input_IsMouseButtonPressed },
			{ "Input.GetMousePosition", (void*)Input_GetMousePosition },
			{ "Entity.Create", (void*)Entity_Create },
			{ "Entity.Destroy", (void*)Entity_Destroy },
			{ "Entity.FindByName", (void*)Entity_FindByName },
			{ "Entity.GetName", (void*)Entity_GetName },
			{ "Entity.SetName", (void*)Entity_SetName },
			{ "Entity.HasComponent", (void*)Entity_HasComponent },
			{ "Entity.AddComponent", (void*)Entity_AddComponent },
			{ "Entity.RemoveComponent", (void*)Entity_RemoveComponent },
			{ "Camera.GetPrimary", (void*)Camera_GetPrimary },
			{ "Camera.SetPrimary", (void*)Camera_SetPrimary },
			{ "Camera.GetFixedAspectRatio", (void*)Camera_GetFixedAspectRatio },
			{ "Camera.SetFixedAspectRatio", (void*)Camera_SetFixedAspectRatio },
			{ "Camera.GetProjectionType", (void*)Camera_GetProjectionType },
			{ "Camera.SetProjectionType", (void*)Camera_SetProjectionType },
			{ "Camera.GetPerspectiveVerticalFOV", (void*)Camera_GetPerspectiveVerticalFOV },
			{ "Camera.SetPerspectiveVerticalFOV", (void*)Camera_SetPerspectiveVerticalFOV },
			{ "Camera.GetPerspectiveNearClip", (void*)Camera_GetPerspectiveNearClip },
			{ "Camera.SetPerspectiveNearClip", (void*)Camera_SetPerspectiveNearClip },
			{ "Camera.GetPerspectiveFarClip", (void*)Camera_GetPerspectiveFarClip },
			{ "Camera.SetPerspectiveFarClip", (void*)Camera_SetPerspectiveFarClip },
			{ "Camera.GetOrthographicSize", (void*)Camera_GetOrthographicSize },
			{ "Camera.SetOrthographicSize", (void*)Camera_SetOrthographicSize },
			{ "Camera.GetOrthographicNearClip", (void*)Camera_GetOrthographicNearClip },
			{ "Camera.SetOrthographicNearClip", (void*)Camera_SetOrthographicNearClip },
			{ "Camera.GetOrthographicFarClip", (void*)Camera_GetOrthographicFarClip },
			{ "Camera.SetOrthographicFarClip", (void*)Camera_SetOrthographicFarClip },
			{ "Transform.GetTranslation", (void*)Transform_GetTranslation },
			{ "Transform.SetTranslation", (void*)Transform_SetTranslation },
			{ "Transform.GetRotation", (void*)Transform_GetRotation },
			{ "Transform.SetRotation", (void*)Transform_SetRotation },
			{ "Transform.GetScale", (void*)Transform_GetScale },
			{ "Transform.SetScale", (void*)Transform_SetScale },
			{ "SpriteRenderer.GetColor", (void*)SpriteRenderer_GetColor },
			{ "SpriteRenderer.SetColor", (void*)SpriteRenderer_SetColor },
			{ "SpriteRenderer.GetTilingFactor", (void*)SpriteRenderer_GetTilingFactor },
			{ "SpriteRenderer.SetTilingFactor", (void*)SpriteRenderer_SetTilingFactor },
			{ "SpriteRenderer.GetTexturePath", (void*)SpriteRenderer_GetTexturePath },
			{ "SpriteRenderer.SetTexturePath", (void*)SpriteRenderer_SetTexturePath },
			{ "Mesh.GetPath", (void*)Mesh_GetPath },
			{ "Mesh.SetPath", (void*)Mesh_SetPath },
			{ "Mesh.GetAlbedo", (void*)Mesh_GetAlbedo },
			{ "Mesh.SetAlbedo", (void*)Mesh_SetAlbedo },
			{ "Mesh.GetEmissive", (void*)Mesh_GetEmissive },
			{ "Mesh.SetEmissive", (void*)Mesh_SetEmissive },
			{ "Mesh.GetRoughness", (void*)Mesh_GetRoughness },
			{ "Mesh.SetRoughness", (void*)Mesh_SetRoughness },
			{ "Mesh.GetMetallic", (void*)Mesh_GetMetallic },
			{ "Mesh.SetMetallic", (void*)Mesh_SetMetallic },
			{ "DirectionalLight.GetDirection", (void*)DirectionalLight_GetDirection },
			{ "DirectionalLight.SetDirection", (void*)DirectionalLight_SetDirection },
			{ "DirectionalLight.GetColor", (void*)DirectionalLight_GetColor },
			{ "DirectionalLight.SetColor", (void*)DirectionalLight_SetColor },
			{ "DirectionalLight.GetIntensity", (void*)DirectionalLight_GetIntensity },
			{ "DirectionalLight.SetIntensity", (void*)DirectionalLight_SetIntensity },
			{ "Script.GetClassName", (void*)Script_GetClassName },
			{ "Script.SetClassName", (void*)Script_SetClassName },
		};
	}
}
