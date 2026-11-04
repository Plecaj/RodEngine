#pragma once

#include "ScriptClass.h"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Rod {

	class ScriptAssembly
	{
	public:
		bool Load(const std::filesystem::path& assemblyPath, const std::string& metadataJson);
		void Clear();

		[[nodiscard]] bool HasClass(const std::string& className) const;
		[[nodiscard]] ScriptClass* GetClass(const std::string& className);
		[[nodiscard]] const std::unordered_map<std::string, ScriptClass>& GetClasses() const noexcept
		{
			return m_Classes;
		}

	private:
		static ScriptFieldType ScriptFieldTypeFromString(const std::string& type);

		std::unordered_map<std::string, ScriptClass> m_Classes;
		std::filesystem::path m_AssemblyPath;
	};
}
