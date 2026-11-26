#include "rdpch.h"

#include "ScriptGlue.h"

#include "Rod/Core/InputState.h"
#include "Rod/Renderer/Mesh.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Utils/FunctionPointerCast.h"
#include "ScriptEngine.h"

#include <cstdlib> // IWYU pragma: keep
#include <cstring>
#include <stdexcept>

#ifdef RD_PLATFORM_WINDOWS
	#include <objbase.h> // IWYU pragma: keep
#endif

namespace Rod {

	namespace {

		template <typename Function>
		[[nodiscard]] auto MakeNativeCall(const char* name, Function function) noexcept -> NativeCall
		{
			return {name, FunctionPointerCast<void*>(function)};
		}

		[[nodiscard]] auto GetEntity(uint64_t entityID) -> Entity
		{
			const auto scene = ScriptGlue::GetSceneContext();
			if (!scene)
				return {};
			return scene->FindEntityByUUID(UUID{entityID});
		}

		[[nodiscard]] auto AllocateString(const std::string& value) noexcept -> void*
		{
#ifdef RD_PLATFORM_WINDOWS
			auto* buffer = static_cast<char*>(CoTaskMemAlloc(value.size() + 1));
#else
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
			auto* buffer = static_cast<char*>(std::malloc(value.size() + 1));
#endif
			if (buffer)
				std::memcpy(buffer, value.c_str(), value.size() + 1);
			return buffer;
		}
	}

	struct ScriptGlue::ScriptVec2
	{
		float X, Y;
	};
	struct ScriptGlue::ScriptVec3
	{
		float X, Y, Z;
	};
	struct ScriptGlue::ScriptVec4
	{
		float X, Y, Z, W;
	};

	// Component tags are shared with the managed API as 32-bit integers.
	// NOLINTNEXTLINE(performance-enum-size)
	enum class ScriptGlue::ScriptComponentType : int32_t
	{
		ID = 0,
		Tag,
		Transform,
		Camera,
		SpriteRenderer,
		Mesh,
		DirectionalLight,
		Script,
	};

	std::optional<Scene&> ScriptGlue::s_SceneContext;
	std::optional<const InputState&> ScriptGlue::s_InputContext;

	auto ScriptGlue::ToGLM(ScriptVec3 value) -> glm::vec3
	{
		return {value.X, value.Y, value.Z};
	}

	auto ScriptGlue::ToGLM(ScriptVec4 value) -> glm::vec4
	{
		return {value.X, value.Y, value.Z, value.W};
	}

	auto ScriptGlue::ToScript(const glm::vec3& value) noexcept -> ScriptGlue::ScriptVec3
	{
		return {.X = value.x, .Y = value.y, .Z = value.z};
	}

	auto ScriptGlue::ToScript(const glm::vec4& value) noexcept -> ScriptGlue::ScriptVec4
	{
		return {.X = value.x, .Y = value.y, .Z = value.z, .W = value.w};
	}

	void RD_CDECL ScriptGlue::LogMessage(int32_t level, const char* message)
	{
		switch (level)
		{
			case s_LogLevelTrace:
				Log::Client().trace("{}", message);
				break;
			case s_LogLevelInfo:
				Log::Client().info("{}", message);
				break;
			case s_LogLevelWarning:
				Log::Client().warn("{}", message);
				break;
			case s_LogLevelError:
				Log::Client().error("{}", message);
				break;
			default:
				Log::Core().info("{}", message);
				break;
		}
	}

	auto RD_CDECL ScriptGlue::InputIsKeyPressed(int32_t keycode) -> bool
	{
		return s_InputContext && s_InputContext->IsKeyDown(keycode);
	}

	auto RD_CDECL ScriptGlue::InputIsMouseButtonPressed(int32_t button) -> bool
	{
		return s_InputContext && s_InputContext->IsMouseDown(button);
	}

	auto RD_CDECL ScriptGlue::InputGetMousePosition() -> ScriptGlue::ScriptVec2
	{
		if (!s_InputContext)
			return {};

		const auto cursor = s_InputContext->GetCursor();
		return {.X = cursor.x, .Y = cursor.y};
	}

	auto RD_CDECL ScriptGlue::EntityCreate(const char* name) -> uint64_t
	{
		if (!s_SceneContext)
			return 0;
		const Entity entity = s_SceneContext->CreateEntity(name ? name : "");
		return static_cast<uint64_t>(entity.GetUUID());
	}

	void RD_CDECL ScriptGlue::EntityDestroy(uint64_t entityID)
	{
		if (!s_SceneContext)
			return;
		auto& scene = *s_SceneContext;
		if (const Entity entity = scene.FindEntityByUUID(UUID{entityID}))
			scene.DestroyEntity(entity);
	}

	auto RD_CDECL ScriptGlue::EntityFindByName(const char* name) -> uint64_t
	{
		if (!s_SceneContext || !name)
			return 0;
		const Entity entity = s_SceneContext->FindEntityByName(name);
		return entity ? static_cast<uint64_t>(entity.GetUUID()) : 0;
	}

	auto RD_CDECL ScriptGlue::EntityGetName(uint64_t entityID) -> void*
	{
		const Entity entity = GetEntity(entityID);
		return AllocateString(entity ? entity.GetName() : "");
	}

	void RD_CDECL ScriptGlue::EntitySetName(uint64_t entityID, const char* name)
	{
		Entity entity = GetEntity(entityID);
		if (entity && name)
			entity.GetComponent<TagComponent>().Tag = name;
	}

	auto RD_CDECL ScriptGlue::EntityHasComponent(uint64_t entityID, int32_t componentType) -> bool
	{
		const Entity entity = GetEntity(entityID);
		if (!entity)
			return false;

		switch (static_cast<ScriptComponentType>(componentType))
		{
			case ScriptComponentType::ID:
				return entity.HasComponent<IDComponent>();
			case ScriptComponentType::Tag:
				return entity.HasComponent<TagComponent>();
			case ScriptComponentType::Transform:
				return entity.HasComponent<TransformComponent>();
			case ScriptComponentType::Camera:
				return entity.HasComponent<CameraComponent>();
			case ScriptComponentType::SpriteRenderer:
				return entity.HasComponent<SpriteRendererComponent>();
			case ScriptComponentType::Mesh:
				return entity.HasComponent<MeshComponent>();
			case ScriptComponentType::DirectionalLight:
				return entity.HasComponent<DirectionalLightComponent>();
			case ScriptComponentType::Script:
				return entity.HasComponent<ScriptComponent>();
		}
		return false;
	}

	void RD_CDECL ScriptGlue::EntityAddComponent(uint64_t entityID, int32_t componentType)
	{
		Entity entity = GetEntity(entityID);
		if (!entity)
			return;

		switch (static_cast<ScriptComponentType>(componentType))
		{
			case ScriptComponentType::Camera:
				if (!entity.HasComponent<CameraComponent>())
					entity.AddComponent<CameraComponent>();
				break;
			case ScriptComponentType::SpriteRenderer:
				if (!entity.HasComponent<SpriteRendererComponent>())
					entity.AddComponent<SpriteRendererComponent>();
				break;
			case ScriptComponentType::Mesh:
				if (!entity.HasComponent<MeshComponent>())
					entity.AddComponent<MeshComponent>();
				break;
			case ScriptComponentType::DirectionalLight:
				if (!entity.HasComponent<DirectionalLightComponent>())
					entity.AddComponent<DirectionalLightComponent>();
				break;
			case ScriptComponentType::Script:
				if (!entity.HasComponent<ScriptComponent>())
					entity.AddComponent<ScriptComponent>();
				break;
			case ScriptComponentType::ID:
			case ScriptComponentType::Tag:
			case ScriptComponentType::Transform:
			default:
				break;
		}
	}

	void RD_CDECL ScriptGlue::EntityRemoveComponent(uint64_t entityID, int32_t componentType)
	{
		Entity entity = GetEntity(entityID);
		if (!entity)
			return;

		switch (static_cast<ScriptComponentType>(componentType))
		{
			case ScriptComponentType::Camera:
				if (entity.HasComponent<CameraComponent>())
					entity.RemoveComponent<CameraComponent>();
				break;
			case ScriptComponentType::SpriteRenderer:
				if (entity.HasComponent<SpriteRendererComponent>())
					entity.RemoveComponent<SpriteRendererComponent>();
				break;
			case ScriptComponentType::Mesh:
				if (entity.HasComponent<MeshComponent>())
					entity.RemoveComponent<MeshComponent>();
				break;
			case ScriptComponentType::DirectionalLight:
				if (entity.HasComponent<DirectionalLightComponent>())
					entity.RemoveComponent<DirectionalLightComponent>();
				break;
			case ScriptComponentType::Script:
				if (entity.HasComponent<ScriptComponent>())
					entity.RemoveComponent<ScriptComponent>();
				break;
			case ScriptComponentType::ID:
			case ScriptComponentType::Tag:
			case ScriptComponentType::Transform:
			default:
				break;
		}
	}

	auto RD_CDECL ScriptGlue::CameraGetPrimary(uint64_t entityID) -> bool
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>() ? entity.GetComponent<CameraComponent>().Primary
		                                                        : false;
	}

	void RD_CDECL ScriptGlue::CameraSetPrimary(uint64_t entityID, bool value)
	{
		if (s_SceneContext)
			s_SceneContext->SetCameraPrimary(s_SceneContext->FindEntityByUUID(UUID{entityID}), value);
	}

	auto RD_CDECL ScriptGlue::CameraGetFixedAspectRatio(uint64_t entityID) -> bool
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>()
		           ? entity.GetComponent<CameraComponent>().FixedAspectRatio
				   : false;
	}

	void RD_CDECL ScriptGlue::CameraSetFixedAspectRatio(uint64_t entityID, bool value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().FixedAspectRatio = value;
	}

	auto RD_CDECL ScriptGlue::CameraGetProjectionType(uint64_t entityID) -> int32_t
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>()
		           ? static_cast<int32_t>(entity.GetComponent<CameraComponent>().Camera.GetProjectionType())
				   : 0;
	}

	void RD_CDECL ScriptGlue::CameraSetProjectionType(uint64_t entityID, int32_t value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
		{
			entity.GetComponent<CameraComponent>().Camera.SetProjectionType(
			    static_cast<SceneCamera::ProjectionType>(value));
		}
	}

	auto RD_CDECL ScriptGlue::CameraGetPerspectiveVerticalFOV(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>()
		           ? entity.GetComponent<CameraComponent>().Camera.GetPerspectiveVerticalFOV()
				   : 0.0f;
	}

	void RD_CDECL ScriptGlue::CameraSetPerspectiveVerticalFOV(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetPerspectiveVerticalFOV(value);
	}

	auto RD_CDECL ScriptGlue::CameraGetPerspectiveNearClip(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>()
		           ? entity.GetComponent<CameraComponent>().Camera.GetPerspectiveNearClip()
				   : 0.0f;
	}

	void RD_CDECL ScriptGlue::CameraSetPerspectiveNearClip(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetPerspectiveNearClip(value);
	}

	auto RD_CDECL ScriptGlue::CameraGetPerspectiveFarClip(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>()
		           ? entity.GetComponent<CameraComponent>().Camera.GetPerspectiveFarClip()
				   : 0.0f;
	}

	void RD_CDECL ScriptGlue::CameraSetPerspectiveFarClip(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetPerspectiveFarClip(value);
	}

	auto RD_CDECL ScriptGlue::CameraGetOrthographicSize(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>()
		           ? entity.GetComponent<CameraComponent>().Camera.GetOrthographicSize()
				   : 0.0f;
	}

	void RD_CDECL ScriptGlue::CameraSetOrthographicSize(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetOrthographicSize(value);
	}

	auto RD_CDECL ScriptGlue::CameraGetOrthographicNearClip(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>()
		           ? entity.GetComponent<CameraComponent>().Camera.GetOrthographicNearClip()
				   : 0.0f;
	}

	void RD_CDECL ScriptGlue::CameraSetOrthographicNearClip(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetOrthographicNearClip(value);
	}

	auto RD_CDECL ScriptGlue::CameraGetOrthographicFarClip(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<CameraComponent>()
		           ? entity.GetComponent<CameraComponent>().Camera.GetOrthographicFarClip()
				   : 0.0f;
	}

	void RD_CDECL ScriptGlue::CameraSetOrthographicFarClip(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Camera.SetOrthographicFarClip(value);
	}

	auto RD_CDECL ScriptGlue::TransformGetTranslation(uint64_t entityID) -> ScriptGlue::ScriptVec3
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<TransformComponent>()
		           ? ToScript(entity.GetComponent<TransformComponent>().Translation)
				   : ScriptVec3{};
	}

	void RD_CDECL ScriptGlue::TransformSetTranslation(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<TransformComponent>())
			entity.GetComponent<TransformComponent>().Translation = ToGLM(value);
	}

	auto RD_CDECL ScriptGlue::TransformGetRotation(uint64_t entityID) -> ScriptGlue::ScriptVec3
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<TransformComponent>()
		           ? ToScript(entity.GetComponent<TransformComponent>().Rotation)
				   : ScriptVec3{};
	}

	void RD_CDECL ScriptGlue::TransformSetRotation(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<TransformComponent>())
			entity.GetComponent<TransformComponent>().Rotation = ToGLM(value);
	}

	auto RD_CDECL ScriptGlue::TransformGetScale(uint64_t entityID) -> ScriptGlue::ScriptVec3
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<TransformComponent>()
		           ? ToScript(entity.GetComponent<TransformComponent>().Scale)
				   : ScriptVec3{.X = 1.0f, .Y = 1.0f, .Z = 1.0f};
	}

	void RD_CDECL ScriptGlue::TransformSetScale(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<TransformComponent>())
			entity.GetComponent<TransformComponent>().Scale = ToGLM(value);
	}

	auto RD_CDECL ScriptGlue::SpriteRendererGetColor(uint64_t entityID) -> ScriptGlue::ScriptVec4
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<SpriteRendererComponent>()
		           ? ToScript(entity.GetComponent<SpriteRendererComponent>().Color)
				   : ScriptVec4{.X = 1.0f, .Y = 1.0f, .Z = 1.0f, .W = 1.0f};
	}

	void RD_CDECL ScriptGlue::SpriteRendererSetColor(uint64_t entityID, ScriptVec4 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<SpriteRendererComponent>())
			entity.GetComponent<SpriteRendererComponent>().Color = ToGLM(value);
	}

	auto RD_CDECL ScriptGlue::SpriteRendererGetTilingFactor(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<SpriteRendererComponent>()
		           ? entity.GetComponent<SpriteRendererComponent>().TilingFactor
				   : 1.0f;
	}

	void RD_CDECL ScriptGlue::SpriteRendererSetTilingFactor(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<SpriteRendererComponent>())
			entity.GetComponent<SpriteRendererComponent>().TilingFactor = value;
	}

	auto RD_CDECL ScriptGlue::SpriteRendererGetTexturePath(uint64_t entityID) -> void*
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<SpriteRendererComponent>() ||
		    !entity.GetComponent<SpriteRendererComponent>().Texture)
			return AllocateString("");
		return AllocateString(entity.GetComponent<SpriteRendererComponent>().Texture->GetPath());
	}

	void RD_CDECL ScriptGlue::SpriteRendererSetTexturePath(uint64_t entityID, const char* path)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<SpriteRendererComponent>())
			return;
		auto& sprite = entity.GetComponent<SpriteRendererComponent>();
		if (!path || *path == '\0')
		{
			sprite.Texture.reset();
			return;
		}

		if (const auto device = ScriptEngine::GetRenderDevice())
		{
			try
			{
				sprite.Texture = device->LoadTexture2D(path);
			}
			catch (const std::runtime_error& exception)
			{
				Log::Core().error("{}", exception.what());
			}
		}
	}

	auto RD_CDECL ScriptGlue::MeshGetPath(uint64_t entityID) -> void*
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<MeshComponent>() || !entity.GetComponent<MeshComponent>().Mesh)
			return AllocateString("");
		return AllocateString(entity.GetComponent<MeshComponent>().Mesh->GetPath());
	}

	void RD_CDECL ScriptGlue::MeshSetPath(uint64_t entityID, const char* path)
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<MeshComponent>())
			return;
		auto& mesh = entity.GetComponent<MeshComponent>();
		if (!path || *path == '\0')
		{
			mesh.SetMesh(nullptr);
			return;
		}
		try
		{
			if (const auto device = ScriptEngine::GetRenderDevice())
				mesh.SetMesh(CreateRef<Mesh>(*device, path));
		}
		catch (const std::runtime_error& exception)
		{
			Log::Core().error("{}", exception.what());
		}
	}

	auto RD_CDECL ScriptGlue::MeshGetAlbedo(uint64_t entityID) -> ScriptGlue::ScriptVec4
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<MeshComponent>() || !entity.GetComponent<MeshComponent>().Mesh)
			return {.X = 1.0f, .Y = 1.0f, .Z = 1.0f, .W = 1.0f};
		return ToScript(entity.GetComponent<MeshComponent>().Material.GetAlbedo());
	}

	void RD_CDECL ScriptGlue::MeshSetAlbedo(uint64_t entityID, ScriptVec4 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh)
			entity.GetComponent<MeshComponent>().Material.SetAlbedo(ToGLM(value));
	}

	auto RD_CDECL ScriptGlue::MeshGetEmissive(uint64_t entityID) -> ScriptGlue::ScriptVec3
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<MeshComponent>() || !entity.GetComponent<MeshComponent>().Mesh)
			return {};
		return ToScript(entity.GetComponent<MeshComponent>().Material.GetEmissive());
	}

	void RD_CDECL ScriptGlue::MeshSetEmissive(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh)
			entity.GetComponent<MeshComponent>().Material.SetEmissive(ToGLM(value));
	}

	auto RD_CDECL ScriptGlue::MeshGetRoughness(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh
		           ? entity.GetComponent<MeshComponent>().Material.GetRoughness()
				   : 1.0f;
	}

	void RD_CDECL ScriptGlue::MeshSetRoughness(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh)
			entity.GetComponent<MeshComponent>().Material.SetRoughness(value);
	}

	auto RD_CDECL ScriptGlue::MeshGetMetallic(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh
		           ? entity.GetComponent<MeshComponent>().Material.GetMetallic()
				   : 0.0f;
	}

	void RD_CDECL ScriptGlue::MeshSetMetallic(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<MeshComponent>() && entity.GetComponent<MeshComponent>().Mesh)
			entity.GetComponent<MeshComponent>().Material.SetMetallic(value);
	}

	auto RD_CDECL ScriptGlue::DirectionalLightGetDirection(uint64_t entityID) -> ScriptGlue::ScriptVec3
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<DirectionalLightComponent>()
		           ? ToScript(entity.GetComponent<DirectionalLightComponent>().Direction)
				   : ScriptVec3{};
	}

	void RD_CDECL ScriptGlue::DirectionalLightSetDirection(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<DirectionalLightComponent>())
			entity.GetComponent<DirectionalLightComponent>().Direction = glm::normalize(ToGLM(value));
	}

	auto RD_CDECL ScriptGlue::DirectionalLightGetColor(uint64_t entityID) -> ScriptGlue::ScriptVec3
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<DirectionalLightComponent>()
		           ? ToScript(entity.GetComponent<DirectionalLightComponent>().Color)
				   : ScriptVec3{.X = 1.0f, .Y = 1.0f, .Z = 1.0f};
	}

	void RD_CDECL ScriptGlue::DirectionalLightSetColor(uint64_t entityID, ScriptVec3 value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<DirectionalLightComponent>())
			entity.GetComponent<DirectionalLightComponent>().Color = ToGLM(value);
	}

	auto RD_CDECL ScriptGlue::DirectionalLightGetIntensity(uint64_t entityID) -> float
	{
		Entity entity = GetEntity(entityID);
		return entity && entity.HasComponent<DirectionalLightComponent>()
		           ? entity.GetComponent<DirectionalLightComponent>().Intensity
				   : 1.0f;
	}

	void RD_CDECL ScriptGlue::DirectionalLightSetIntensity(uint64_t entityID, float value)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<DirectionalLightComponent>())
			entity.GetComponent<DirectionalLightComponent>().Intensity = value;
	}

	auto RD_CDECL ScriptGlue::ScriptGetClassName(uint64_t entityID) -> void*
	{
		Entity entity = GetEntity(entityID);
		if (!entity || !entity.HasComponent<ScriptComponent>())
			return AllocateString("");
		return AllocateString(entity.GetComponent<ScriptComponent>().ClassName);
	}

	void RD_CDECL ScriptGlue::ScriptSetClassName(uint64_t entityID, const char* className)
	{
		Entity entity = GetEntity(entityID);
		if (entity && entity.HasComponent<ScriptComponent>())
			entity.GetComponent<ScriptComponent>().ClassName = className ? className : "";
	}

	void ScriptGlue::SetSceneContext(std::optional<Scene&> scene) noexcept
	{
		s_SceneContext = scene;
	}

	void ScriptGlue::SetInputContext(std::optional<const InputState&> input) noexcept
	{
		s_InputContext = input;
	}

	auto ScriptGlue::GetNativeCalls() -> std::vector<NativeCall>
	{
		return {
		    MakeNativeCall("Log.Message", LogMessage),
		    MakeNativeCall("Input.IsKeyPressed", InputIsKeyPressed),
		    MakeNativeCall("Input.IsMouseButtonPressed", InputIsMouseButtonPressed),
		    MakeNativeCall("Input.GetMousePosition", InputGetMousePosition),
		    MakeNativeCall("Entity.Create", EntityCreate),
		    MakeNativeCall("Entity.Destroy", EntityDestroy),
		    MakeNativeCall("Entity.FindByName", EntityFindByName),
		    MakeNativeCall("Entity.GetName", EntityGetName),
		    MakeNativeCall("Entity.SetName", EntitySetName),
		    MakeNativeCall("Entity.HasComponent", EntityHasComponent),
		    MakeNativeCall("Entity.AddComponent", EntityAddComponent),
		    MakeNativeCall("Entity.RemoveComponent", EntityRemoveComponent),
		    MakeNativeCall("Camera.GetPrimary", CameraGetPrimary),
		    MakeNativeCall("Camera.SetPrimary", CameraSetPrimary),
		    MakeNativeCall("Camera.GetFixedAspectRatio", CameraGetFixedAspectRatio),
		    MakeNativeCall("Camera.SetFixedAspectRatio", CameraSetFixedAspectRatio),
		    MakeNativeCall("Camera.GetProjectionType", CameraGetProjectionType),
		    MakeNativeCall("Camera.SetProjectionType", CameraSetProjectionType),
		    MakeNativeCall("Camera.GetPerspectiveVerticalFOV", CameraGetPerspectiveVerticalFOV),
		    MakeNativeCall("Camera.SetPerspectiveVerticalFOV", CameraSetPerspectiveVerticalFOV),
		    MakeNativeCall("Camera.GetPerspectiveNearClip", CameraGetPerspectiveNearClip),
		    MakeNativeCall("Camera.SetPerspectiveNearClip", CameraSetPerspectiveNearClip),
		    MakeNativeCall("Camera.GetPerspectiveFarClip", CameraGetPerspectiveFarClip),
		    MakeNativeCall("Camera.SetPerspectiveFarClip", CameraSetPerspectiveFarClip),
		    MakeNativeCall("Camera.GetOrthographicSize", CameraGetOrthographicSize),
		    MakeNativeCall("Camera.SetOrthographicSize", CameraSetOrthographicSize),
		    MakeNativeCall("Camera.GetOrthographicNearClip", CameraGetOrthographicNearClip),
		    MakeNativeCall("Camera.SetOrthographicNearClip", CameraSetOrthographicNearClip),
		    MakeNativeCall("Camera.GetOrthographicFarClip", CameraGetOrthographicFarClip),
		    MakeNativeCall("Camera.SetOrthographicFarClip", CameraSetOrthographicFarClip),
		    MakeNativeCall("Transform.GetTranslation", TransformGetTranslation),
		    MakeNativeCall("Transform.SetTranslation", TransformSetTranslation),
		    MakeNativeCall("Transform.GetRotation", TransformGetRotation),
		    MakeNativeCall("Transform.SetRotation", TransformSetRotation),
		    MakeNativeCall("Transform.GetScale", TransformGetScale),
		    MakeNativeCall("Transform.SetScale", TransformSetScale),
		    MakeNativeCall("SpriteRenderer.GetColor", SpriteRendererGetColor),
		    MakeNativeCall("SpriteRenderer.SetColor", SpriteRendererSetColor),
		    MakeNativeCall("SpriteRenderer.GetTilingFactor", SpriteRendererGetTilingFactor),
		    MakeNativeCall("SpriteRenderer.SetTilingFactor", SpriteRendererSetTilingFactor),
		    MakeNativeCall("SpriteRenderer.GetTexturePath", SpriteRendererGetTexturePath),
		    MakeNativeCall("SpriteRenderer.SetTexturePath", SpriteRendererSetTexturePath),
		    MakeNativeCall("Mesh.GetPath", MeshGetPath),
		    MakeNativeCall("Mesh.SetPath", MeshSetPath),
		    MakeNativeCall("Mesh.GetAlbedo", MeshGetAlbedo),
		    MakeNativeCall("Mesh.SetAlbedo", MeshSetAlbedo),
		    MakeNativeCall("Mesh.GetEmissive", MeshGetEmissive),
		    MakeNativeCall("Mesh.SetEmissive", MeshSetEmissive),
		    MakeNativeCall("Mesh.GetRoughness", MeshGetRoughness),
		    MakeNativeCall("Mesh.SetRoughness", MeshSetRoughness),
		    MakeNativeCall("Mesh.GetMetallic", MeshGetMetallic),
		    MakeNativeCall("Mesh.SetMetallic", MeshSetMetallic),
		    MakeNativeCall("DirectionalLight.GetDirection", DirectionalLightGetDirection),
		    MakeNativeCall("DirectionalLight.SetDirection", DirectionalLightSetDirection),
		    MakeNativeCall("DirectionalLight.GetColor", DirectionalLightGetColor),
		    MakeNativeCall("DirectionalLight.SetColor", DirectionalLightSetColor),
		    MakeNativeCall("DirectionalLight.GetIntensity", DirectionalLightGetIntensity),
		    MakeNativeCall("DirectionalLight.SetIntensity", DirectionalLightSetIntensity),
		    MakeNativeCall("Script.GetClassName", ScriptGetClassName),
		    MakeNativeCall("Script.SetClassName", ScriptSetClassName),
		};
	}
}
