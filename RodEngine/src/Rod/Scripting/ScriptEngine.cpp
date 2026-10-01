#include "rdpch.h"
#include "ScriptEngine.h"

#include "ScriptBuilder.h"
#include "ScriptGlue.h"
#include "ScriptRuntime.h"
#include "Rod/Core/Application.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"

#include <memory>

#ifdef RD_PLATFORM_WINDOWS
	#include <objbase.h>
#endif

namespace Rod {

	struct ScriptEngineData
	{
		ScriptRuntime Runtime;
		ScriptAssembly AppAssembly;

		std::filesystem::path CoreAssemblyPath;
		std::filesystem::path RuntimeConfigPath;
		std::filesystem::path ScriptProjectPath;
		std::filesystem::path ScriptAssemblyPath;
		std::filesystem::path ScriptSourceDirectory;
		std::filesystem::path ScriptCacheDirectory;

		Scene* SceneContext = nullptr;
		bool RuntimeRunning = false;
		bool Initialized = false;
		std::filesystem::file_time_type LastSourceWriteTime = std::filesystem::file_time_type::min();

		using InitializeFn = int32_t(__cdecl*)(NativeCall*, int32_t);
		using ShutdownFn = void(__cdecl*)();
		using LoadAssemblyFn = void*(__cdecl*)(const char*);
		using UnloadAssemblyFn = void(__cdecl*)();
		using CreateScriptFn = int32_t(__cdecl*)(uint64_t, const char*);
		using StartScriptFn = void(__cdecl*)(uint64_t);
		using DestroyScriptFn = void(__cdecl*)(uint64_t);
		using UpdateScriptFn = void(__cdecl*)(uint64_t, float);
		using SetFieldValueFn = void(__cdecl*)(uint64_t, const char*, int32_t, const char*);

		InitializeFn Initialize = nullptr;
		ShutdownFn Shutdown = nullptr;
		LoadAssemblyFn LoadAssembly = nullptr;
		UnloadAssemblyFn UnloadAssembly = nullptr;
		CreateScriptFn CreateScript = nullptr;
		StartScriptFn StartScript = nullptr;
		DestroyScriptFn DestroyScript = nullptr;
		UpdateScriptFn UpdateScript = nullptr;
		SetFieldValueFn SetFieldValue = nullptr;
	};

	static std::unique_ptr<ScriptEngineData> s_Data;

	static std::filesystem::path GetEditorRoot()
	{
		return std::filesystem::current_path();
	}

	static std::filesystem::path GetSourceRoot()
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

	static std::filesystem::path GetManagedAssemblyPath(const std::filesystem::path& projectPath, const std::string& requiredSidecar = "")
	{
		std::string assemblyName = projectPath.stem().string() + ".dll";
		std::array<std::filesystem::path, 2> candidates = {
			projectPath.parent_path() / "bin" / "Debug" / "net9.0" / assemblyName,
			projectPath.parent_path() / "bin" / "x64" / "Debug" / "net9.0" / assemblyName
		};

		std::filesystem::path newestCandidate;
		std::filesystem::file_time_type newestWriteTime = std::filesystem::file_time_type::min();

		auto trySelectCandidate = [&](const std::filesystem::path& candidate)
		{
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

	static std::filesystem::path SelectPackagedAssembly(const std::vector<std::filesystem::path>& candidates, const std::string& requiredSidecar = "")
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

	static std::filesystem::path CreateAssemblyCachePath(const std::filesystem::path& assemblyPath)
	{
		std::filesystem::create_directories(s_Data->ScriptCacheDirectory);
		auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();

		std::filesystem::path cachedAssembly = s_Data->ScriptCacheDirectory / (assemblyPath.stem().string() + "-" + std::to_string(timestamp) + assemblyPath.extension().string());
		std::filesystem::copy_file(assemblyPath, cachedAssembly, std::filesystem::copy_options::overwrite_existing);

		std::filesystem::path pdbPath = assemblyPath;
		pdbPath.replace_extension(".pdb");
		if (std::filesystem::exists(pdbPath))
		{
			std::filesystem::path cachedPdb = cachedAssembly;
			cachedPdb.replace_extension(".pdb");
			std::filesystem::copy_file(pdbPath, cachedPdb, std::filesystem::copy_options::overwrite_existing);
		}

		return cachedAssembly;
	}

	static std::string CopyManagedString(void* value)
	{
		if (!value)
			return {};

		const char* text = (const char*)value;
		std::string result = text;
#ifdef RD_PLATFORM_WINDOWS
		CoTaskMemFree(value);
#endif
		return result;
	}

	static bool BindHostFunctions()
	{
		const std::wstring scriptHostType = L"Rod.Internal.ScriptHost, Rod.ScriptCore";
		s_Data->Initialize = (ScriptEngineData::InitializeFn)s_Data->Runtime.GetFunction(scriptHostType, L"Initialize");
		s_Data->Shutdown = (ScriptEngineData::ShutdownFn)s_Data->Runtime.GetFunction(scriptHostType, L"Shutdown");
		s_Data->LoadAssembly = (ScriptEngineData::LoadAssemblyFn)s_Data->Runtime.GetFunction(scriptHostType, L"LoadAssembly");
		s_Data->UnloadAssembly = (ScriptEngineData::UnloadAssemblyFn)s_Data->Runtime.GetFunction(scriptHostType, L"UnloadAssembly");
		s_Data->CreateScript = (ScriptEngineData::CreateScriptFn)s_Data->Runtime.GetFunction(scriptHostType, L"CreateScript");
		s_Data->StartScript = (ScriptEngineData::StartScriptFn)s_Data->Runtime.GetFunction(scriptHostType, L"StartScript");
		s_Data->DestroyScript = (ScriptEngineData::DestroyScriptFn)s_Data->Runtime.GetFunction(scriptHostType, L"DestroyScript");
		s_Data->UpdateScript = (ScriptEngineData::UpdateScriptFn)s_Data->Runtime.GetFunction(scriptHostType, L"UpdateScript");
		s_Data->SetFieldValue = (ScriptEngineData::SetFieldValueFn)s_Data->Runtime.GetFunction(scriptHostType, L"SetFieldValue");

		return s_Data->Initialize && s_Data->LoadAssembly && s_Data->CreateScript
			&& s_Data->StartScript && s_Data->DestroyScript && s_Data->UpdateScript
			&& s_Data->SetFieldValue;
	}

	static void InitializeScriptPaths()
	{
		std::filesystem::path root = GetSourceRoot();
		std::filesystem::path editorRoot = GetEditorRoot();
		bool editorMode = Application::Get().IsEditor();

		std::filesystem::path sourceCoreAssemblyPath = GetManagedAssemblyPath(root / "Rod-ScriptCore" / "Rod.ScriptCore.csproj", "Rod.ScriptCore.runtimeconfig.json");
		std::filesystem::path packagedCoreAssemblyPath = SelectPackagedAssembly({
			editorRoot / "assets" / "Scripts" / "Core" / "Rod.ScriptCore.dll",
			editorRoot / "assets" / "Scripts" / "bin" / "Debug" / "net9.0" / "Rod.ScriptCore.dll"
		}, "Rod.ScriptCore.runtimeconfig.json");

		s_Data->CoreAssemblyPath = editorMode || !std::filesystem::exists(packagedCoreAssemblyPath)
			? sourceCoreAssemblyPath
			: packagedCoreAssemblyPath;
		s_Data->RuntimeConfigPath = s_Data->CoreAssemblyPath.parent_path() / "Rod.ScriptCore.runtimeconfig.json";
		s_Data->ScriptProjectPath = root / "Rod-Editor" / "assets" / "Scripts" / "RodGame.csproj";
		s_Data->ScriptAssemblyPath = GetManagedAssemblyPath(s_Data->ScriptProjectPath);
		if (!editorMode && !std::filesystem::exists(s_Data->ScriptAssemblyPath))
		{
			s_Data->ScriptAssemblyPath = SelectPackagedAssembly({
				editorRoot / "assets" / "Scripts" / "App" / "RodGame.dll",
				editorRoot / "assets" / "Scripts" / "bin" / "Debug" / "net9.0" / "RodGame.dll"
			});
		}
		s_Data->ScriptSourceDirectory = root / "Rod-Editor" / "assets" / "Scripts" / "Source";
		s_Data->ScriptCacheDirectory = editorRoot / "assets" / "Scripts" / "Cache";
	}

	static void ClearFailedInit()
	{
		if (!s_Data)
			return;

		s_Data->Runtime.Shutdown();
		s_Data.reset();
	}

	void ScriptEngine::Init()
	{
		if (s_Data)
			return;

		s_Data = std::make_unique<ScriptEngineData>();
		InitializeScriptPaths();

		if (!std::filesystem::exists(s_Data->CoreAssemblyPath))
		{
			RD_CORE_WARN("Script core assembly is missing. Build Rod.ScriptCore before using scripts: {}", s_Data->CoreAssemblyPath.string());
			ClearFailedInit();
			return;
		}

		if (!std::filesystem::exists(s_Data->RuntimeConfigPath))
		{
			RD_CORE_WARN("Script core runtime config is missing. Build Rod.ScriptCore before using scripts: {}", s_Data->RuntimeConfigPath.string());
			ClearFailedInit();
			return;
		}

		if (!s_Data->Runtime.Initialize(s_Data->RuntimeConfigPath, s_Data->CoreAssemblyPath))
		{
			ClearFailedInit();
			return;
		}

		if (!BindHostFunctions())
		{
			ClearFailedInit();
			return;
		}

		std::vector<NativeCall> nativeCalls = ScriptGlue::GetNativeCalls();
		if (s_Data->Initialize(nativeCalls.data(), (int32_t)nativeCalls.size()) != 0)
		{
			RD_CORE_ERROR("Managed script host initialization failed.");
			ClearFailedInit();
			return;
		}

		s_Data->Initialized = true;
		BuildScripts();
		ReloadAssembly();
		RefreshSourceWriteTime();
	}

	void ScriptEngine::Shutdown()
	{
		if (!s_Data)
			return;

		OnRuntimeStop();

		if (s_Data->Shutdown)
			s_Data->Shutdown();

		s_Data->Runtime.Shutdown();
		s_Data.reset();
	}

	void ScriptEngine::SetScriptProject(const std::filesystem::path& projectPath)
	{
		if (!s_Data)
			Init();

		s_Data->ScriptProjectPath = projectPath;
		s_Data->ScriptAssemblyPath = GetManagedAssemblyPath(projectPath);
		s_Data->ScriptSourceDirectory = projectPath.parent_path() / "Source";
	}

	bool ScriptEngine::BuildScripts()
	{
		if (!s_Data)
			Init();

		if (!std::filesystem::exists(s_Data->ScriptProjectPath))
		{
			if (std::filesystem::exists(s_Data->ScriptAssemblyPath))
			{
				RD_CORE_TRACE("Script project is not available. Using packaged script assembly: {}", s_Data->ScriptAssemblyPath.string());
				return true;
			}
		}

		ScriptBuildResult result = ScriptBuilder::BuildProject(s_Data->ScriptProjectPath);
		if (!result.Success)
		{
			RD_CORE_ERROR("Script build failed:\n{}", result.Output);
			return false;
		}

		RD_CORE_TRACE("Script build succeeded:\n{}", result.Output);
		s_Data->ScriptAssemblyPath = GetManagedAssemblyPath(s_Data->ScriptProjectPath);
		return true;
	}

	bool ScriptEngine::ReloadAssembly()
	{
		if (!s_Data || !s_Data->Initialized || !std::filesystem::exists(s_Data->ScriptAssemblyPath))
			return false;

		if (s_Data->UnloadAssembly)
			s_Data->UnloadAssembly();

		std::filesystem::path cachedAssembly = CreateAssemblyCachePath(s_Data->ScriptAssemblyPath);
		std::string metadataJson = CopyManagedString(s_Data->LoadAssembly(cachedAssembly.string().c_str()));
		if (metadataJson.empty())
			return false;

		return s_Data->AppAssembly.Load(cachedAssembly, metadataJson);
	}

	void ScriptEngine::OnUpdate()
	{
		if (!s_Data || !s_Data->Initialized)
			return;

		std::filesystem::file_time_type latestWriteTime = GetLatestSourceWriteTime();
		if (latestWriteTime == std::filesystem::file_time_type::min() || latestWriteTime <= s_Data->LastSourceWriteTime)
			return;

		RefreshSourceWriteTime();

		bool wasRunning = s_Data->RuntimeRunning;
		Scene* scene = s_Data->SceneContext;
		if (wasRunning)
			OnRuntimeStop();

		if (BuildScripts() && ReloadAssembly() && wasRunning)
			OnRuntimeStart(scene);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		if (!s_Data)
			Init();
		if (!s_Data || !s_Data->Initialized)
			return;

		s_Data->SceneContext = scene;
		s_Data->RuntimeRunning = true;
		ScriptGlue::SetSceneContext(scene);

		auto view = scene->m_Registry.view<IDComponent, ScriptComponent>();
		for (auto entityID : view)
			OnCreateEntity(Entity{ entityID, scene });
	}

	void ScriptEngine::OnRuntimeStop()
	{
		if (!s_Data || !s_Data->RuntimeRunning)
			return;

		if (s_Data->SceneContext)
		{
			auto view = s_Data->SceneContext->m_Registry.view<IDComponent, ScriptComponent>();
			for (auto entityID : view)
				OnDestroyEntity(Entity{ entityID, s_Data->SceneContext });
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
			OnUpdateEntity(Entity{ entityID, s_Data->SceneContext }, ts);
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		if (!s_Data || !s_Data->CreateScript || !entity.HasComponent<ScriptComponent>())
			return;

		auto& script = entity.GetComponent<ScriptComponent>();
		if (script.ClassName.empty() || !s_Data->AppAssembly.HasClass(script.ClassName))
			return;

		if (s_Data->CreateScript(entity.GetUUID(), script.ClassName.c_str()) != 0)
		{
			ApplyScriptFields(entity);
			s_Data->StartScript(entity.GetUUID());
		}
	}

	void ScriptEngine::OnDestroyEntity(Entity entity)
	{
		if (!s_Data || !s_Data->DestroyScript)
			return;
		s_Data->DestroyScript(entity.GetUUID());
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		if (!s_Data || !s_Data->UpdateScript)
			return;
		s_Data->UpdateScript(entity.GetUUID(), ts.GetSeconds());
	}

	void ScriptEngine::SetRuntimeFieldValue(Entity entity, const std::string& fieldName)
	{
		if (!s_Data || !s_Data->RuntimeRunning || !s_Data->SetFieldValue || !entity || !entity.HasComponent<ScriptComponent>())
			return;

		auto& script = entity.GetComponent<ScriptComponent>();
		auto fieldIt = script.Fields.find(fieldName);
		if (fieldIt == script.Fields.end())
			return;

		const auto& field = fieldIt->second;
		s_Data->SetFieldValue(entity.GetUUID(), fieldName.c_str(), (int32_t)field.Field.Type, field.Value.c_str());
	}

	bool ScriptEngine::IsInitialized()
	{
		return s_Data && s_Data->Initialized;
	}

	bool ScriptEngine::IsRuntimeRunning()
	{
		return s_Data && s_Data->RuntimeRunning;
	}

	const std::unordered_map<std::string, ScriptClass>& ScriptEngine::GetScriptClasses()
	{
		static std::unordered_map<std::string, ScriptClass> emptyClasses;
		return s_Data ? s_Data->AppAssembly.GetClasses() : emptyClasses;
	}

	ScriptClass* ScriptEngine::GetScriptClass(const std::string& className)
	{
		return s_Data ? s_Data->AppAssembly.GetClass(className) : nullptr;
	}

	void ScriptEngine::RefreshSourceWriteTime()
	{
		if (s_Data)
			s_Data->LastSourceWriteTime = GetLatestSourceWriteTime();
	}

	std::filesystem::file_time_type ScriptEngine::GetLatestSourceWriteTime()
	{
		if (!s_Data || !std::filesystem::exists(s_Data->ScriptSourceDirectory))
			return std::filesystem::file_time_type::min();

		std::filesystem::file_time_type latestWriteTime = std::filesystem::file_time_type::min();
		for (const auto& entry : std::filesystem::recursive_directory_iterator(s_Data->ScriptSourceDirectory))
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
		if (!s_Data || !s_Data->SetFieldValue || !entity.HasComponent<ScriptComponent>())
			return;

		auto& script = entity.GetComponent<ScriptComponent>();
		for (const auto& [name, field] : script.Fields)
		{
			if (!field.Value.empty())
				s_Data->SetFieldValue(entity.GetUUID(), name.c_str(), (int32_t)field.Field.Type, field.Value.c_str());
		}
	}
}
