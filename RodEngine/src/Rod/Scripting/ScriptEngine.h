#pragma once

#include "Rod/Core/Timestep.h"
#include "ScriptAssembly.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

		[[nodiscard]] static bool IsInitialized() noexcept;
		[[nodiscard]] static bool IsRuntimeRunning() noexcept;
		[[nodiscard]] static const std::unordered_map<std::string, ScriptClass>& GetScriptClasses();
		[[nodiscard]] static ScriptClass* GetScriptClass(const std::string& className);

	private:
		struct ScriptEngineData;
		static std::unique_ptr<ScriptEngineData> s_Data;

		static std::filesystem::path GetEditorRoot();
		static std::filesystem::path GetSourceRoot();
		static std::filesystem::path
		GetManagedAssemblyPath(const std::filesystem::path& projectPath, const std::string& requiredSidecar = "");
		static std::filesystem::path SelectPackagedAssembly(
		    const std::vector<std::filesystem::path>& candidates,
		    const std::string& requiredSidecar = "");
		static std::filesystem::path CreateAssemblyCachePath(const std::filesystem::path& assemblyPath);
		static std::string CopyManagedString(void* value);
		static bool BindHostFunctions();
		static void InitializeScriptPaths();
		static void ClearFailedInit();

		static void RefreshSourceWriteTime();
		static std::filesystem::file_time_type GetLatestSourceWriteTime();
		static void ApplyScriptFields(Entity entity);
	};
}
