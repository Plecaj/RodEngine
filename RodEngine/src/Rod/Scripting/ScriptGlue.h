#pragma once

#include "Rod/Core/Core.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Rod {
	class Scene;
	class Entity;
	class Input;

	struct NativeCall
	{
		const char* Name;
		void* Function;
	};

	class ScriptGlue
	{
	public:
		static void SetSceneContext(std::optional<Scene&> scene) noexcept;
		static void SetInputContext(std::optional<const Input&> input) noexcept;
		[[nodiscard]] static auto GetSceneContext() noexcept -> std::optional<Scene&> { return s_SceneContext; }
		[[nodiscard]] static auto GetNativeCalls() -> std::vector<NativeCall>;

	private:
		struct ScriptVec2;
		struct ScriptVec3;
		struct ScriptVec4;
		enum class ScriptComponentType : int32_t;
		static constexpr int32_t s_LogLevelTrace = 0;
		static constexpr int32_t s_LogLevelInfo = 1;
		static constexpr int32_t s_LogLevelWarning = 2;
		static constexpr int32_t s_LogLevelError = 3;

		[[nodiscard]] static auto ToGLM(ScriptVec3 value) -> glm::vec3;
		[[nodiscard]] static auto ToGLM(ScriptVec4 value) -> glm::vec4;
		[[nodiscard]] static auto ToScript(const glm::vec3& value) noexcept -> ScriptVec3;
		[[nodiscard]] static auto ToScript(const glm::vec4& value) noexcept -> ScriptVec4;
		static void RD_CDECL LogMessage(int32_t level, const char* message);
		static auto RD_CDECL InputIsKeyPressed(int32_t keycode) -> bool;
		static auto RD_CDECL InputIsMouseButtonPressed(int32_t button) -> bool;
		static auto RD_CDECL InputGetMousePosition() -> ScriptVec2;
		static auto RD_CDECL EntityCreate(const char* name) -> uint64_t;
		static void RD_CDECL EntityDestroy(uint64_t entityID);
		static auto RD_CDECL EntityFindByName(const char* name) -> uint64_t;
		static auto RD_CDECL EntityGetName(uint64_t entityID) -> void*;
		static void RD_CDECL EntitySetName(uint64_t entityID, const char* name);
		static auto RD_CDECL EntityHasComponent(uint64_t entityID, int32_t componentType) -> bool;
		static void RD_CDECL EntityAddComponent(uint64_t entityID, int32_t componentType);
		static void RD_CDECL EntityRemoveComponent(uint64_t entityID, int32_t componentType);
		static auto RD_CDECL CameraGetPrimary(uint64_t entityID) -> bool;
		static void RD_CDECL CameraSetPrimary(uint64_t entityID, bool value);
		static auto RD_CDECL CameraGetFixedAspectRatio(uint64_t entityID) -> bool;
		static void RD_CDECL CameraSetFixedAspectRatio(uint64_t entityID, bool value);
		static auto RD_CDECL CameraGetProjectionType(uint64_t entityID) -> int32_t;
		static void RD_CDECL CameraSetProjectionType(uint64_t entityID, int32_t value);
		static auto RD_CDECL CameraGetPerspectiveVerticalFOV(uint64_t entityID) -> float;
		static void RD_CDECL CameraSetPerspectiveVerticalFOV(uint64_t entityID, float value);
		static auto RD_CDECL CameraGetPerspectiveNearClip(uint64_t entityID) -> float;
		static void RD_CDECL CameraSetPerspectiveNearClip(uint64_t entityID, float value);
		static auto RD_CDECL CameraGetPerspectiveFarClip(uint64_t entityID) -> float;
		static void RD_CDECL CameraSetPerspectiveFarClip(uint64_t entityID, float value);
		static auto RD_CDECL CameraGetOrthographicSize(uint64_t entityID) -> float;
		static void RD_CDECL CameraSetOrthographicSize(uint64_t entityID, float value);
		static auto RD_CDECL CameraGetOrthographicNearClip(uint64_t entityID) -> float;
		static void RD_CDECL CameraSetOrthographicNearClip(uint64_t entityID, float value);
		static auto RD_CDECL CameraGetOrthographicFarClip(uint64_t entityID) -> float;
		static void RD_CDECL CameraSetOrthographicFarClip(uint64_t entityID, float value);
		static auto RD_CDECL TransformGetTranslation(uint64_t entityID) -> ScriptVec3;
		static void RD_CDECL TransformSetTranslation(uint64_t entityID, ScriptVec3 value);
		static auto RD_CDECL TransformGetRotation(uint64_t entityID) -> ScriptVec3;
		static void RD_CDECL TransformSetRotation(uint64_t entityID, ScriptVec3 value);
		static auto RD_CDECL TransformGetScale(uint64_t entityID) -> ScriptVec3;
		static void RD_CDECL TransformSetScale(uint64_t entityID, ScriptVec3 value);
		static auto RD_CDECL SpriteRendererGetColor(uint64_t entityID) -> ScriptVec4;
		static void RD_CDECL SpriteRendererSetColor(uint64_t entityID, ScriptVec4 value);
		static auto RD_CDECL SpriteRendererGetTilingFactor(uint64_t entityID) -> float;
		static void RD_CDECL SpriteRendererSetTilingFactor(uint64_t entityID, float value);
		static auto RD_CDECL SpriteRendererGetTexturePath(uint64_t entityID) -> void*;
		static void RD_CDECL SpriteRendererSetTexturePath(uint64_t entityID, const char* path);
		static auto RD_CDECL MeshGetPath(uint64_t entityID) -> void*;
		static void RD_CDECL MeshSetPath(uint64_t entityID, const char* path);
		static auto RD_CDECL MeshGetAlbedo(uint64_t entityID) -> ScriptVec4;
		static void RD_CDECL MeshSetAlbedo(uint64_t entityID, ScriptVec4 value);
		static auto RD_CDECL MeshGetEmissive(uint64_t entityID) -> ScriptVec3;
		static void RD_CDECL MeshSetEmissive(uint64_t entityID, ScriptVec3 value);
		static auto RD_CDECL MeshGetRoughness(uint64_t entityID) -> float;
		static void RD_CDECL MeshSetRoughness(uint64_t entityID, float value);
		static auto RD_CDECL MeshGetMetallic(uint64_t entityID) -> float;
		static void RD_CDECL MeshSetMetallic(uint64_t entityID, float value);
		static auto RD_CDECL DirectionalLightGetDirection(uint64_t entityID) -> ScriptVec3;
		static void RD_CDECL DirectionalLightSetDirection(uint64_t entityID, ScriptVec3 value);
		static auto RD_CDECL DirectionalLightGetColor(uint64_t entityID) -> ScriptVec3;
		static void RD_CDECL DirectionalLightSetColor(uint64_t entityID, ScriptVec3 value);
		static auto RD_CDECL DirectionalLightGetIntensity(uint64_t entityID) -> float;
		static void RD_CDECL DirectionalLightSetIntensity(uint64_t entityID, float value);
		static auto RD_CDECL ScriptGetClassName(uint64_t entityID) -> void*;
		static void RD_CDECL ScriptSetClassName(uint64_t entityID, const char* className);

		static std::optional<Scene&> s_SceneContext;
		static std::optional<const Input&> s_InputContext;
	};
}
