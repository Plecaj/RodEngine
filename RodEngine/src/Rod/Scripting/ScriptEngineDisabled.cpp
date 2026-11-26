#include "rdpch.h"

#include "Rod/Core/Log.h"
#include "Rod/Scene/Entity.h"
#include "ScriptEngine.h"

namespace Rod {

	namespace {
		void ReportUnavailable()
		{
			if (Log::IsInitialized())
				Log::Core().warn("C# scripting is unavailable in this build");
		}
	}

	void ScriptEngine::Init(
	    const Project& /*project*/,
	    const InputState& /*input*/,
	    const RenderDevice& /*device*/,
	    Mode /*mode*/)
	{}

	void ScriptEngine::Shutdown()
	{}

	void ScriptEngine::SetScriptProject(const std::filesystem::path& /*projectPath*/)
	{}

	auto ScriptEngine::BuildScripts() -> bool
	{
		ReportUnavailable();
		return false;
	}

	auto ScriptEngine::ReloadAssembly() -> bool
	{
		ReportUnavailable();
		return false;
	}

	void ScriptEngine::OnUpdate()
	{}

	void ScriptEngine::OnRuntimeStart(Scene& /*scene*/)
	{}

	void ScriptEngine::OnRuntimeStop()
	{}

	void ScriptEngine::OnRuntimeUpdate(Timestep /*timestep*/)
	{}

	void ScriptEngine::OnCreateEntity(Entity /*entity*/)
	{}

	void ScriptEngine::OnDestroyEntity(Entity /*entity*/)
	{}

	void ScriptEngine::OnUpdateEntity(Entity /*entity*/, Timestep /*timestep*/)
	{}

	void ScriptEngine::SetRuntimeFieldValue(Entity /*entity*/, const std::string& /*fieldName*/)
	{}

	auto ScriptEngine::IsInitialized() noexcept -> bool
	{
		return false;
	}

	auto ScriptEngine::IsRuntimeRunning() noexcept -> bool
	{
		return false;
	}

	auto ScriptEngine::GetScriptClasses() -> const std::unordered_map<std::string, ScriptClass>&
	{
		static const std::unordered_map<std::string, ScriptClass> emptyClasses;
		return emptyClasses;
	}

	auto ScriptEngine::GetScriptClass(const std::string& /*className*/) -> std::optional<ScriptClass&>
	{
		return std::nullopt;
	}

}
