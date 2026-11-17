#pragma once

#include "Rod/Core/Core.h"
#include "ScriptClass.h"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Rod {

	class ScriptAssembly
	{
	public:
		ScriptAssembly(const std::filesystem::path& assemblyPath, const std::string& metadataJson);
		~ScriptAssembly() = default;
		ScriptAssembly(const ScriptAssembly&) = default;
		ScriptAssembly& operator=(const ScriptAssembly&) = default;
		ScriptAssembly(ScriptAssembly&&) noexcept = default;
		ScriptAssembly& operator=(ScriptAssembly&&) noexcept = default;

		[[nodiscard]] bool HasClass(const std::string& className) const;
		[[nodiscard]] ObserverPtr<ScriptClass> GetClass(const std::string& className);
		[[nodiscard]] const std::unordered_map<std::string, ScriptClass>& GetClasses() const noexcept
		{
			return m_Classes;
		}

	private:
		std::unordered_map<std::string, ScriptClass> m_Classes;
		std::filesystem::path m_AssemblyPath;
	};
}
