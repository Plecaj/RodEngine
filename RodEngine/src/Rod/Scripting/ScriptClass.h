#pragma once

#include "ScriptField.h"

namespace Rod {

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(std::string fullName, std::string namespaceName, std::string className);

		const std::string& GetFullName() const { return m_FullName; }
		const std::string& GetNamespaceName() const { return m_NamespaceName; }
		const std::string& GetClassName() const { return m_ClassName; }

		const ScriptFieldMap& GetFields() const { return m_Fields; }
		ScriptFieldMap& GetFields() { return m_Fields; }
	private:
		std::string m_FullName;
		std::string m_NamespaceName;
		std::string m_ClassName;
		ScriptFieldMap m_Fields;
	};
}
