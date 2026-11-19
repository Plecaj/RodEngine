#pragma once

#include "ScriptClass.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace Rod {

	class ScriptAssembly
	{
	public:
		ScriptAssembly(const std::filesystem::path& assemblyPath, const std::string& metadataJson);
		~ScriptAssembly() = default;
		ScriptAssembly(const ScriptAssembly&) = default;
		auto operator=(const ScriptAssembly&) -> ScriptAssembly& = default;
		ScriptAssembly(ScriptAssembly&&) noexcept = default;
		auto operator=(ScriptAssembly&&) noexcept -> ScriptAssembly& = default;

		[[nodiscard]] auto HasClass(const std::string& className) const -> bool;
		[[nodiscard]] auto GetClass(const std::string& className) -> std::optional<ScriptClass&>;
		[[nodiscard]] auto GetClasses() const noexcept -> const std::unordered_map<std::string, ScriptClass>&
		{
			return m_Classes;
		}

	private:
		std::unordered_map<std::string, ScriptClass> m_Classes;
		std::filesystem::path m_AssemblyPath;
	};
}
