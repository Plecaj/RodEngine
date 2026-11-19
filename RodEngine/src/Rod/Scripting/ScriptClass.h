#pragma once

#include "ScriptField.h"

#include <string>

namespace Rod {

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(std::string fullName, std::string namespaceName, std::string className);

		[[nodiscard]] auto GetFullName() const noexcept -> const std::string& { return m_FullName; }
		[[nodiscard]] auto GetNamespaceName() const noexcept -> const std::string& { return m_NamespaceName; }
		[[nodiscard]] auto GetClassName() const noexcept -> const std::string& { return m_ClassName; }

		[[nodiscard]] auto GetFields() const noexcept -> const ScriptFieldMap& { return m_Fields; }
		[[nodiscard]] auto GetFields() noexcept -> ScriptFieldMap& { return m_Fields; }

	private:
		std::string m_FullName;
		std::string m_NamespaceName;
		std::string m_ClassName;
		ScriptFieldMap m_Fields;
	};
}
