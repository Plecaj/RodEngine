#pragma once

#include "Rod/Core/Core.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace Rod {
	class Scene;
	class Entity;

	struct NativeCall
	{
		const char* Name;
		void* Function;
	};

	class ScriptGlue
	{
	public:
		static void SetSceneContext(Scene* scene);
		static Scene* GetSceneContext() { return s_SceneContext; }
		static std::vector<NativeCall> GetNativeCalls();

	private:
		struct ScriptVec2;
		struct ScriptVec3;
		struct ScriptVec4;
		enum class ScriptComponentType : int32_t;
		static constexpr int32_t LOG_LEVEL_TRACE = 0;
		static constexpr int32_t LOG_LEVEL_INFO = 1;
		static constexpr int32_t LOG_LEVEL_WARNING = 2;
		static constexpr int32_t LOG_LEVEL_ERROR = 3;

		template <typename Function>
		static NativeCall MakeNativeCall(const char* name, Function function);
		static Entity GetEntity(uint64_t entityID);
		static void* AllocateString(const std::string& value);
		static glm::vec3 ToGLM(ScriptVec3 value);
		static glm::vec4 ToGLM(ScriptVec4 value);
		static ScriptVec3 ToScript(const glm::vec3& value);
		static ScriptVec4 ToScript(const glm::vec4& value);
		static void RD_CDECL LogMessage(int32_t level, const char* message);
		static bool RD_CDECL InputIsKeyPressed(int32_t keycode);
		static bool RD_CDECL InputIsMouseButtonPressed(int32_t button);
		static ScriptVec2 RD_CDECL InputGetMousePosition();
		static uint64_t RD_CDECL EntityCreate(const char* name);
		static void RD_CDECL EntityDestroy(uint64_t entityID);
		static uint64_t RD_CDECL EntityFindByName(const char* name);
		static void* RD_CDECL EntityGetName(uint64_t entityID);
		static void RD_CDECL EntitySetName(uint64_t entityID, const char* name);
		static bool RD_CDECL EntityHasComponent(uint64_t entityID, int32_t componentType);
		static void RD_CDECL EntityAddComponent(uint64_t entityID, int32_t componentType);
		static void RD_CDECL EntityRemoveComponent(uint64_t entityID, int32_t componentType);
		static bool RD_CDECL CameraGetPrimary(uint64_t entityID);
		static void RD_CDECL CameraSetPrimary(uint64_t entityID, bool value);
		static bool RD_CDECL CameraGetFixedAspectRatio(uint64_t entityID);
		static void RD_CDECL CameraSetFixedAspectRatio(uint64_t entityID, bool value);
		static int32_t RD_CDECL CameraGetProjectionType(uint64_t entityID);
		static void RD_CDECL CameraSetProjectionType(uint64_t entityID, int32_t value);
		static float RD_CDECL CameraGetPerspectiveVerticalFOV(uint64_t entityID);
		static void RD_CDECL CameraSetPerspectiveVerticalFOV(uint64_t entityID, float value);
		static float RD_CDECL CameraGetPerspectiveNearClip(uint64_t entityID);
		static void RD_CDECL CameraSetPerspectiveNearClip(uint64_t entityID, float value);
		static float RD_CDECL CameraGetPerspectiveFarClip(uint64_t entityID);
		static void RD_CDECL CameraSetPerspectiveFarClip(uint64_t entityID, float value);
		static float RD_CDECL CameraGetOrthographicSize(uint64_t entityID);
		static void RD_CDECL CameraSetOrthographicSize(uint64_t entityID, float value);
		static float RD_CDECL CameraGetOrthographicNearClip(uint64_t entityID);
		static void RD_CDECL CameraSetOrthographicNearClip(uint64_t entityID, float value);
		static float RD_CDECL CameraGetOrthographicFarClip(uint64_t entityID);
		static void RD_CDECL CameraSetOrthographicFarClip(uint64_t entityID, float value);
		static ScriptVec3 RD_CDECL TransformGetTranslation(uint64_t entityID);
		static void RD_CDECL TransformSetTranslation(uint64_t entityID, ScriptVec3 value);
		static ScriptVec3 RD_CDECL TransformGetRotation(uint64_t entityID);
		static void RD_CDECL TransformSetRotation(uint64_t entityID, ScriptVec3 value);
		static ScriptVec3 RD_CDECL TransformGetScale(uint64_t entityID);
		static void RD_CDECL TransformSetScale(uint64_t entityID, ScriptVec3 value);
		static ScriptVec4 RD_CDECL SpriteRendererGetColor(uint64_t entityID);
		static void RD_CDECL SpriteRendererSetColor(uint64_t entityID, ScriptVec4 value);
		static float RD_CDECL SpriteRendererGetTilingFactor(uint64_t entityID);
		static void RD_CDECL SpriteRendererSetTilingFactor(uint64_t entityID, float value);
		static void* RD_CDECL SpriteRendererGetTexturePath(uint64_t entityID);
		static void RD_CDECL SpriteRendererSetTexturePath(uint64_t entityID, const char* path);
		static void* RD_CDECL MeshGetPath(uint64_t entityID);
		static void RD_CDECL MeshSetPath(uint64_t entityID, const char* path);
		static ScriptVec4 RD_CDECL MeshGetAlbedo(uint64_t entityID);
		static void RD_CDECL MeshSetAlbedo(uint64_t entityID, ScriptVec4 value);
		static ScriptVec3 RD_CDECL MeshGetEmissive(uint64_t entityID);
		static void RD_CDECL MeshSetEmissive(uint64_t entityID, ScriptVec3 value);
		static float RD_CDECL MeshGetRoughness(uint64_t entityID);
		static void RD_CDECL MeshSetRoughness(uint64_t entityID, float value);
		static float RD_CDECL MeshGetMetallic(uint64_t entityID);
		static void RD_CDECL MeshSetMetallic(uint64_t entityID, float value);
		static ScriptVec3 RD_CDECL DirectionalLightGetDirection(uint64_t entityID);
		static void RD_CDECL DirectionalLightSetDirection(uint64_t entityID, ScriptVec3 value);
		static ScriptVec3 RD_CDECL DirectionalLightGetColor(uint64_t entityID);
		static void RD_CDECL DirectionalLightSetColor(uint64_t entityID, ScriptVec3 value);
		static float RD_CDECL DirectionalLightGetIntensity(uint64_t entityID);
		static void RD_CDECL DirectionalLightSetIntensity(uint64_t entityID, float value);
		static void* RD_CDECL ScriptGetClassName(uint64_t entityID);
		static void RD_CDECL ScriptSetClassName(uint64_t entityID, const char* className);

		static Scene* s_SceneContext;
	};
}
