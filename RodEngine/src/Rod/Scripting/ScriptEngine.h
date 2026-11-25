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
	class InputState;
	class Project;
	class RenderDevice;
	class ScriptGlue;
	class Scene;

	class ScriptEngine
	{
	public:
		enum class Mode
		{
			Editor,
			Runtime
		};

		static void Init(const Project& project, const InputState& input, const RenderDevice& device, Mode mode);
		static void Shutdown();

		static void SetScriptProject(const std::filesystem::path& projectPath);
		static auto BuildScripts() -> bool;
		static auto ReloadAssembly() -> bool;
		static void OnUpdate();

		static void OnRuntimeStart(Scene& scene);
		static void OnRuntimeStop();
		static void OnRuntimeUpdate(Timestep ts);

		static void OnCreateEntity(Entity entity);
		static void OnDestroyEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);
		static void SetRuntimeFieldValue(Entity entity, const std::string& fieldName);

		[[nodiscard]] static auto IsInitialized() noexcept -> bool;
		[[nodiscard]] static auto IsRuntimeRunning() noexcept -> bool;
		[[nodiscard]] static auto GetScriptClasses() -> const std::unordered_map<std::string, ScriptClass>&;
		[[nodiscard]] static auto GetScriptClass(const std::string& className) -> std::optional<ScriptClass&>;

	private:
		friend class ScriptGlue;
		[[nodiscard]] static auto GetRenderDevice() noexcept -> std::optional<const RenderDevice&>;

		struct Configuration
		{
			const Project& ProjectData;
			const InputState& Input;
			const RenderDevice& Device;
			Mode StartupMode;
		};
		struct ScriptPaths;
		struct ScriptEngineData;
		static std::optional<Configuration> s_Configuration;
		static std::unique_ptr<ScriptEngineData> s_Data;

		[[nodiscard]] static auto CreateAssemblyCachePath(const std::filesystem::path& assemblyPath)
		    -> std::filesystem::path;
		[[nodiscard]] static auto ResolveScriptPaths(const Project& project) -> ScriptPaths;
		static auto EnsureInitialized() -> bool;

		static void RefreshSourceWriteTime();
		[[nodiscard]] static auto GetLatestSourceWriteTime() -> std::filesystem::file_time_type;
		static void ApplyScriptFields(Entity entity);
	};
}
