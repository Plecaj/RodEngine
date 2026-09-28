#include "rdpch.h"
#include "ScriptClass.h"

namespace Rod {

	ScriptClass::ScriptClass(std::string fullName, std::string namespaceName, std::string className)
		: m_FullName(std::move(fullName)), m_NamespaceName(std::move(namespaceName)), m_ClassName(std::move(className))
	{
	}
}
