#pragma once

#include "ScriptClass.h"

#include <filesystem>

namespace Rod {

	class ScriptAssembly
	{
	public:
		bool Load(const std::filesystem::path& assemblyPath, const std::string& metadataJson);
		void Clear();

		bool HasClass(const std::string& className) const;
		ScriptClass* GetClass(const std::string& className);
		const std::unordered_map<std::string, ScriptClass>& GetClasses() const { return m_Classes; }
	private:
		std::unordered_map<std::string, ScriptClass> m_Classes;
		std::filesystem::path m_AssemblyPath;
	};
}
