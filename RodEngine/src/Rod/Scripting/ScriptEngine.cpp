#include "rdpch.h"

#include "ScriptEngine.h"

#include "Rod/Core/Application.h"
#include "Rod/Core/UUID.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "ScriptBuilder.h"
#include "ScriptGlue.h"
#include "ScriptRuntime.h"

#include <bit>
#include <cstdlib> // IWYU pragma: keep
#include <optional>
#include <stdexcept>
#include <system_error>

#ifdef RD_PLATFORM_WINDOWS
	#include <objbase.h> // IWYU pragma: keep
#endif

namespace Rod {

	struct ScriptEngine::ScriptPaths
	{
		std::filesystem::path CoreAssemblyPath;
		std::filesystem::path RuntimeConfigPath;
		std::filesystem::path ScriptProjectPath;
		std::filesystem::path ScriptAssemblyPath;
		std::filesystem::path ScriptSourceDirectory;
		std::filesystem::path ScriptCacheDirectory;
	};

	struct ScriptEngine::ScriptEngineData
	{
		struct HostFunctions
		{
			static constexpr auto TYPE_NAME = "Rod.Internal.ScriptHost, Rod.ScriptCore";
			using InitializeFn = int32_t(RD_CDECL*)(NativeCall*, int32_t);
			using ShutdownFn = void(RD_CDECL*)();
			using LoadAssemblyFn = void*(RD_CDECL*)(const char*);
			using CreateScriptFn = int32_t(RD_CDECL*)(uint64_t, const char*);
			using StartScriptFn = void(RD_CDECL*)(uint64_t);
			using DestroyScriptFn = void(RD_CDECL*)(uint64_t);
			using UpdateScriptFn = void(RD_CDECL*)(uint64_t, float);
			using SetFieldValueFn = void(RD_CDECL*)(uint64_t, const char*, int32_t, const char*);

			template <typename Function>
			static Function Require(const ScriptRuntime& runtime, const char* name)
			{
				auto function = std::bit_cast<Function>(runtime.GetFunction(TYPE_NAME, name));
				if (!function)
					throw std::runtime_error("Script host function missing '" + std::string(name) + "'");
				return function;
			}

			explicit HostFunctions(const ScriptRuntime& runtime)
			    : Initialize(Require<InitializeFn>(runtime, "Initialize")),
			      Shutdown(Require<ShutdownFn>(runtime, "Shutdown")),
			      LoadAssembly(Require<LoadAssemblyFn>(runtime, "LoadAssembly")),
			      CreateScript(Require<CreateScriptFn>(runtime, "CreateScript")),
			      StartScript(Require<StartScriptFn>(runtime, "StartScript")),
			      DestroyScript(Require<DestroyScriptFn>(runtime, "DestroyScript")),
			      UpdateScript(Require<UpdateScriptFn>(runtime, "UpdateScript")),
			      SetFieldValue(Require<SetFieldValueFn>(runtime, "SetFieldValue"))
			{}

			InitializeFn Initialize;
			ShutdownFn Shutdown;
			LoadAssemblyFn LoadAssembly;
			CreateScriptFn CreateScript;
			StartScriptFn StartScript;
			DestroyScriptFn DestroyScript;
			UpdateScriptFn UpdateScript;
			SetFieldValueFn SetFieldValue;
		};

		class HostLifetime
		{
		public:
			explicit HostLifetime(HostFunctions::ShutdownFn shutdown) noexcept : m_Shutdown(shutdown) {}
			~HostLifetime()
			{
				if (m_Shutdown)
					m_Shutdown();
			}
			HostLifetime(const HostLifetime&) = delete;
			HostLifetime& operator=(const HostLifetime&) = delete;
			HostLifetime(HostLifetime&& other) noexcept : m_Shutdown(std::exchange(other.m_Shutdown, nullptr)) {}
			HostLifetime& operator=(HostLifetime&& other) noexcept
			{
				if (this != &other)
				{
					if (m_Shutdown)
						m_Shutdown();
					m_Shutdown = std::exchange(other.m_Shutdown, nullptr);
				}
				return *this;
			}

		private:
			HostFunctions::ShutdownFn m_Shutdown;
		};

		explicit ScriptEngineData(ScriptPaths paths)
		    : Paths(std::move(paths)), Runtime(Paths.RuntimeConfigPath, Paths.CoreAssemblyPath),
		      Functions(Runtime), Host(Functions.Shutdown)
		{
			auto calls = ScriptGlue::GetNativeCalls();
			if (Functions.Initialize(calls.data(), static_cast<int32_t>(calls.size())) != 0)
				throw std::runtime_error("Script host initialization failed");
		}
		~ScriptEngineData() = default;
		ScriptEngineData(const ScriptEngineData&) = delete;
		ScriptEngineData& operator=(const ScriptEngineData&) = delete;
		ScriptEngineData(ScriptEngineData&&) = delete;
		ScriptEngineData& operator=(ScriptEngineData&&) = delete;

		ScriptPaths Paths;
		ScriptRuntime Runtime;
		HostFunctions Functions;
		HostLifetime Host;
		std::optional<ScriptAssembly> AppAssembly;
		Scene* SceneContext = nullptr;
		bool RuntimeRunning = false;
		std::filesystem::file_time_type LastSourceWriteTime = std::filesystem::file_time_type::min();
	};

	std::unique_ptr<ScriptEngine::ScriptEngineData> ScriptEngine::s_Data;

	std::filesystem::path ScriptEngine::GetEditorRoot()
	{
		return std::filesystem::current_path();
	}

	std::filesystem::path ScriptEngine::GetSourceRoot()
	{
#ifdef ROD_SOURCE_DIR
		return ROD_SOURCE_DIR;
#else
		std::filesystem::path current = std::filesystem::current_path();
		if (std::filesystem::exists(current / "Rod-ScriptCore"))
			return current;
		if (std::filesystem::exists(current.parent_path().parent_path().parent_path() / "Rod-ScriptCore"))
			return current.parent_path().parent_path().parent_path();
		return current;
#endif
	}

	std::filesystem::path
	ScriptEngine::GetManagedAssemblyPath(const std::filesystem::path& projectPath, const std::string& requiredSidecar)
	{
		const std::string assemblyName = projectPath.stem().string() + ".dll";
		const auto candidates = std::to_array<std::filesystem::path>(
		    {projectPath.parent_path() / "bin" / "Debug" / "net9.0" / assemblyName,
		     projectPath.parent_path() / "bin" / "x64" / "Debug" / "net9.0" / assemblyName});

		std::filesystem::path newestCandidate;
		std::filesystem::file_time_type newestWriteTime = std::filesystem::file_time_type::min();

		auto trySelectCandidate = [&](const std::filesystem::path& candidate) {
			if (!std::filesystem::exists(candidate))
				return;

			if (!requiredSidecar.empty() && !std::filesystem::exists(candidate.parent_path() / requiredSidecar))
				return;

			auto writeTime = std::filesystem::last_write_time(candidate);
			if (newestCandidate.empty() || writeTime > newestWriteTime)
			{
				newestCandidate = candidate;
				newestWriteTime = writeTime;
			}
		};

		for (const auto& candidate : candidates)
			trySelectCandidate(candidate);

		if (!newestCandidate.empty())
			return newestCandidate;

		return candidates[0];
	}

	std::filesystem::path ScriptEngine::SelectPackagedAssembly(
	    const std::vector<std::filesystem::path>& candidates,
	    const std::string& requiredSidecar)
	{
		for (const auto& candidate : candidates)
		{
			if (!std::filesystem::exists(candidate))
				continue;

			if (!requiredSidecar.empty() && !std::filesystem::exists(candidate.parent_path() / requiredSidecar))
				continue;

			return candidate;
		}

		return candidates.empty() ? std::filesystem::path() : candidates[0];
	}

	std::filesystem::path ScriptEngine::CreateAssemblyCachePath(const std::filesystem::path& assemblyPath)
	{
		std::filesystem::create_directories(s_Data->Paths.ScriptCacheDirectory);

		std::filesystem::path cachedAssembly =
		    s_Data->Paths.ScriptCacheDirectory /
		    (assemblyPath.stem().string() + "-" + std::to_string(static_cast<uint64_t>(UUID{})) +
		     assemblyPath.extension().string());
		auto cachedPdb = cachedAssembly;
		cachedPdb.replace_extension(".pdb");
		try
		{
			std::filesystem::copy_file(assemblyPath, cachedAssembly);
			auto pdbPath = assemblyPath;
			pdbPath.replace_extension(".pdb");
			if (std::filesystem::exists(pdbPath))
				std::filesystem::copy_file(pdbPath, cachedPdb);
		}
		catch (...)
		{
			std::error_code error;
			std::filesystem::remove(cachedAssembly, error);
			std::filesystem::remove(cachedPdb, error);
			throw;
		}

		return cachedAssembly;
	}

	std::string ScriptEngine::CopyManagedString(void* value)
	{
		if (!value)
			return {};

		auto release = [](void* text) {
#ifdef RD_PLATFORM_WINDOWS
			CoTaskMemFree(text);
#else
			std::free(text);
#endif
		};
		std::unique_ptr<void, decltype(release)> text(value, release);
		return std::string(static_cast<const char*>(text.get()));
	}

	ScriptEngine::ScriptPaths ScriptEngine::ResolveScriptPaths()
	{
		const std::filesystem::path root = GetSourceRoot();
		const std::filesystem::path editorRoot = GetEditorRoot();
		const bool editorMode = Application::Get().IsEditor();

		const std::filesystem::path sourceCoreAssemblyPath = GetManagedAssemblyPath(
		    root / "Rod-ScriptCore" / "Rod.ScriptCore.csproj",
		    "Rod.ScriptCore.runtimeconfig.json");
		const std::filesystem::path packagedCoreAssemblyPath = SelectPackagedAssembly(
		    {editorRoot / "assets" / "Scripts" / "Core" / "Rod.ScriptCore.dll",
		     editorRoot / "assets" / "Scripts" / "bin" / "Debug" / "net9.0" / "Rod.ScriptCore.dll"},
		    "Rod.ScriptCore.runtimeconfig.json");

		const auto coreAssembly = editorMode || !std::filesystem::exists(packagedCoreAssemblyPath)
		                              ? sourceCoreAssemblyPath
		                              : packagedCoreAssemblyPath;
		const auto scriptProject = root / "Rod-Editor" / "assets" / "Scripts" / "RodGame.csproj";
		auto scriptAssembly = GetManagedAssemblyPath(scriptProject);
		if (!editorMode && !std::filesystem::exists(scriptAssembly))
		{
			scriptAssembly = SelectPackagedAssembly(
			    {editorRoot / "assets" / "Scripts" / "App" / "RodGame.dll",
			     editorRoot / "assets" / "Scripts" / "bin" / "Debug" / "net9.0" / "RodGame.dll"});
		}
		return {
		    coreAssembly,
		    coreAssembly.parent_path() / "Rod.ScriptCore.runtimeconfig.json",
		    scriptProject,
		    scriptAssembly,
		    scriptProject.parent_path() / "Source",
		    editorRoot / "assets" / "Scripts" / "Cache"};
	}

	void ScriptEngine::Init()
	{
		if (s_Data)
			return;

		try
		{
			auto data = std::make_unique<ScriptEngineData>(ResolveScriptPaths());
			s_Data = std::move(data);
		}
		catch (const std::runtime_error& exception)
		{
			RD_CORE_ERROR("Script startup failed: {}", exception.what());
			return;
		}

		if (BuildScripts())
			ReloadAssembly();
		RefreshSourceWriteTime();
	}

	void ScriptEngine::Shutdown()
	{
		if (!s_Data)
			return;

		OnRuntimeStop();

		s_Data.reset();
	}

	void ScriptEngine::SetScriptProject(const std::filesystem::path& projectPath)
	{
		if (!s_Data)
			Init();
		if (!s_Data)
			return;

		s_Data->Paths.ScriptProjectPath = projectPath;
		s_Data->Paths.ScriptAssemblyPath = GetManagedAssemblyPath(projectPath);
		s_Data->Paths.ScriptSourceDirectory = projectPath.parent_path() / "Source";
	}

	bool ScriptEngine::BuildScripts()
	{
		if (!s_Data)
			Init();
		if (!s_Data)
			return false;

		if (!std::filesystem::exists(s_Data->Paths.ScriptProjectPath))
		{
			if (std::filesystem::exists(s_Data->Paths.ScriptAssemblyPath))
			{
				RD_CORE_TRACE(
				    "Script project is not available. Using packaged script assembly: {}",
				    s_Data->Paths.ScriptAssemblyPath.string());
				return true;
			}
		}

		ScriptBuildResult result = ScriptBuilder::BuildProject(s_Data->Paths.ScriptProjectPath);
		if (!result.Success)
		{
			RD_CORE_ERROR("Script build failed:\n{}", result.Output);
			return false;
		}

		RD_CORE_TRACE("Script build succeeded:\n{}", result.Output);
		s_Data->Paths.ScriptAssemblyPath = GetManagedAssemblyPath(s_Data->Paths.ScriptProjectPath);
		return true;
	}

	bool ScriptEngine::ReloadAssembly()
	{
		if (!s_Data)
			return false;
		try
		{
			if (!std::filesystem::exists(s_Data->Paths.ScriptAssemblyPath))
			{
				RD_CORE_ERROR("Assembly missing '{}'", s_Data->Paths.ScriptAssemblyPath.string());
				return false;
			}
			const auto cachedAssembly = CreateAssemblyCachePath(s_Data->Paths.ScriptAssemblyPath);
			const auto metadataJson =
			    CopyManagedString(s_Data->Functions.LoadAssembly(cachedAssembly.string().c_str()));
			if (metadataJson.empty())
			{
				RD_CORE_ERROR("Assembly load failed '{}'", cachedAssembly.string());
				return false;
			}
			ScriptAssembly assembly(cachedAssembly, metadataJson);
			s_Data->AppAssembly = std::move(assembly);
			return true;
		}
		catch (const std::runtime_error& exception)
		{
			RD_CORE_ERROR("Assembly reload failed: {}", exception.what());
			return false;
		}
	}

	void ScriptEngine::OnUpdate()
	{
		if (!s_Data)
			return;

		const std::filesystem::file_time_type latestWriteTime = GetLatestSourceWriteTime();
		if (latestWriteTime == std::filesystem::file_time_type::min() || latestWriteTime <= s_Data->LastSourceWriteTime)
			return;

		RefreshSourceWriteTime();
		if (!BuildScripts())
			return;

		const bool wasRunning = s_Data->RuntimeRunning;
		Scene* scene = s_Data->SceneContext;
		if (wasRunning)
			OnRuntimeStop();

		ReloadAssembly();
		if (wasRunning)
			OnRuntimeStart(scene);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		if (!s_Data)
			Init();
		if (!s_Data)
			return;

		s_Data->SceneContext = scene;
		s_Data->RuntimeRunning = true;
		ScriptGlue::SetSceneContext(scene);

		auto view = scene->m_Registry.view<IDComponent, ScriptComponent>();
		for (auto entityID : view)
			OnCreateEntity(Entity{entityID, scene});
	}

	void ScriptEngine::OnRuntimeStop()
	{
		if (!s_Data || !s_Data->RuntimeRunning)
			return;

		if (s_Data->SceneContext)
		{
			auto view = s_Data->SceneContext->m_Registry.view<IDComponent, ScriptComponent>();
			for (auto entityID : view)
				OnDestroyEntity(Entity{entityID, s_Data->SceneContext});
		}

		s_Data->RuntimeRunning = false;
		s_Data->SceneContext = nullptr;
		ScriptGlue::SetSceneContext(nullptr);
	}

	void ScriptEngine::OnRuntimeUpdate(Timestep ts)
	{
		if (!s_Data || !s_Data->RuntimeRunning || !s_Data->SceneContext)
			return;

		auto view = s_Data->SceneContext->m_Registry.view<IDComponent, ScriptComponent>();
		for (auto entityID : view)
			OnUpdateEntity(Entity{entityID, s_Data->SceneContext}, ts);
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		if (!s_Data || !s_Data->AppAssembly || !entity.HasComponent<ScriptComponent>())
			return;

		auto& script = entity.GetComponent<ScriptComponent>();
		if (script.ClassName.empty() || !s_Data->AppAssembly->HasClass(script.ClassName))
			return;

		const auto entityID = static_cast<uint64_t>(entity.GetUUID());
		if (s_Data->Functions.CreateScript(entityID, script.ClassName.c_str()) != 0)
		{
			ApplyScriptFields(entity);
			s_Data->Functions.StartScript(entityID);
		}
	}

	void ScriptEngine::OnDestroyEntity(Entity entity)
	{
		if (!s_Data)
			return;
		s_Data->Functions.DestroyScript(static_cast<uint64_t>(entity.GetUUID()));
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		if (!s_Data)
			return;
		s_Data->Functions.UpdateScript(static_cast<uint64_t>(entity.GetUUID()), ts.GetSeconds());
	}

	void ScriptEngine::SetRuntimeFieldValue(Entity entity, const std::string& fieldName)
	{
		if (!s_Data || !s_Data->RuntimeRunning || !entity ||
		    !entity.HasComponent<ScriptComponent>())
			return;

		auto& script = entity.GetComponent<ScriptComponent>();
		auto fieldIt = script.Fields.find(fieldName);
		if (fieldIt == script.Fields.end())
			return;

		const auto& field = fieldIt->second;
		s_Data->Functions.SetFieldValue(
		    static_cast<uint64_t>(entity.GetUUID()),
		    fieldName.c_str(),
		    static_cast<int32_t>(field.Field.Type),
		    field.Value.c_str());
	}

	bool ScriptEngine::IsInitialized() noexcept
	{
		return s_Data != nullptr;
	}

	bool ScriptEngine::IsRuntimeRunning() noexcept
	{
		return s_Data && s_Data->RuntimeRunning;
	}

	const std::unordered_map<std::string, ScriptClass>& ScriptEngine::GetScriptClasses()
	{
		static const std::unordered_map<std::string, ScriptClass> EMPTY_CLASSES;
		return s_Data && s_Data->AppAssembly ? s_Data->AppAssembly->GetClasses() : EMPTY_CLASSES;
	}

	ScriptClass* ScriptEngine::GetScriptClass(const std::string& className)
	{
		return s_Data && s_Data->AppAssembly ? s_Data->AppAssembly->GetClass(className) : nullptr;
	}

	void ScriptEngine::RefreshSourceWriteTime()
	{
		if (s_Data)
			s_Data->LastSourceWriteTime = GetLatestSourceWriteTime();
	}

	std::filesystem::file_time_type ScriptEngine::GetLatestSourceWriteTime()
	{
		if (!s_Data || !std::filesystem::exists(s_Data->Paths.ScriptSourceDirectory))
			return std::filesystem::file_time_type::min();

		std::filesystem::file_time_type latestWriteTime = std::filesystem::file_time_type::min();
		for (const auto& entry : std::filesystem::recursive_directory_iterator(s_Data->Paths.ScriptSourceDirectory))
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".cs")
				continue;

			auto writeTime = std::filesystem::last_write_time(entry.path());
			if (writeTime > latestWriteTime)
				latestWriteTime = writeTime;
		}
		return latestWriteTime;
	}

	void ScriptEngine::ApplyScriptFields(Entity entity)
	{
		if (!s_Data || !entity.HasComponent<ScriptComponent>())
			return;

		auto& script = entity.GetComponent<ScriptComponent>();
		for (const auto& [name, field] : script.Fields)
		{
			if (!field.Value.empty())
			s_Data->Functions.SetFieldValue(
				    static_cast<uint64_t>(entity.GetUUID()),
				    name.c_str(),
				    static_cast<int32_t>(field.Field.Type),
				    field.Value.c_str());
		}
	}
}
