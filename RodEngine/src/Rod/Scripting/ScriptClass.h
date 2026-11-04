#pragma once

#include "ScriptField.h"

#include <string>

namespace Rod {

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(std::string fullName, std::string namespaceName, std::string className);

		[[nodiscard]] const std::string& GetFullName() const noexcept { return m_FullName; }
		[[nodiscard]] const std::string& GetNamespaceName() const noexcept { return m_NamespaceName; }
		[[nodiscard]] const std::string& GetClassName() const noexcept { return m_ClassName; }

		[[nodiscard]] const ScriptFieldMap& GetFields() const noexcept { return m_Fields; }
		[[nodiscard]] ScriptFieldMap& GetFields() noexcept { return m_Fields; }

	private:
		std::string m_FullName;
		std::string m_NamespaceName;
		std::string m_ClassName;
		ScriptFieldMap m_Fields;
	};
}
