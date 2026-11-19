#pragma once

#include "Rod/Core/Timestep.h"
#include "ScriptAssembly.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Rod {

	class Entity;
	class Input;
	class Project;
	class Scene;

	class ScriptEngine
	{
	public:
		enum class Mode
		{
			Editor,
			Runtime
		};

		static void Init(const Project& project, const Input& input, Mode mode);
		static void Shutdown();

		static void SetScriptProject(const std::filesystem::path& projectPath);
		static bool BuildScripts();
		static bool ReloadAssembly();
		static void OnUpdate();

		static void OnRuntimeStart(Scene& scene);
		static void OnRuntimeStop();
		static void OnRuntimeUpdate(Timestep ts);

		static void OnCreateEntity(Entity entity);
		static void OnDestroyEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);
		static void SetRuntimeFieldValue(Entity entity, const std::string& fieldName);

		[[nodiscard]] static bool IsInitialized() noexcept;
		[[nodiscard]] static bool IsRuntimeRunning() noexcept;
		[[nodiscard]] static const std::unordered_map<std::string, ScriptClass>& GetScriptClasses();
		[[nodiscard]] static std::optional<ScriptClass&> GetScriptClass(const std::string& className);

	private:
		struct Configuration
		{
			const Project& ProjectData;
			const Input& InputDevice;
			Mode StartupMode;
		};
		struct ScriptPaths;
		struct ScriptEngineData;
		static std::optional<Configuration> s_Configuration;
		static std::unique_ptr<ScriptEngineData> s_Data;

		[[nodiscard]] static std::filesystem::path CreateAssemblyCachePath(const std::filesystem::path& assemblyPath);
		[[nodiscard]] static ScriptPaths ResolveScriptPaths(const Project& project);
		static bool EnsureInitialized();

		static void RefreshSourceWriteTime();
		[[nodiscard]] static std::filesystem::file_time_type GetLatestSourceWriteTime();
		static void ApplyScriptFields(Entity entity);
	};
}
