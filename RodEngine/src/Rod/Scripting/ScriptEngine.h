#pragma once

#include "Rod/Core/Timestep.h"
#include "ScriptAssembly.h"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Rod {

	class Entity;
	class Scene;

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void SetScriptProject(const std::filesystem::path& projectPath);
		static bool BuildScripts();
		static bool ReloadAssembly();
		static void OnUpdate();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();
		static void OnRuntimeUpdate(Timestep ts);

		static void OnCreateEntity(Entity entity);
		static void OnDestroyEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);
		static void SetRuntimeFieldValue(Entity entity, const std::string& fieldName);

		static bool IsInitialized();
		static bool IsRuntimeRunning();
		static const std::unordered_map<std::string, ScriptClass>& GetScriptClasses();
		static ScriptClass* GetScriptClass(const std::string& className);
	private:
		static void RefreshSourceWriteTime();
		static std::filesystem::file_time_type GetLatestSourceWriteTime();
		static void ApplyScriptFields(Entity entity);
	};
}
