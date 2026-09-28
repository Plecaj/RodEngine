#include "rdpch.h"
#include "ScriptAssembly.h"

#include <json.hpp>

namespace Rod {

	static ScriptFieldType ScriptFieldTypeFromString(const std::string& type)
	{
		if (type == "Single" || type == "float") return ScriptFieldType::Float;
		if (type == "Double" || type == "double") return ScriptFieldType::Double;
		if (type == "Boolean" || type == "bool") return ScriptFieldType::Bool;
		if (type == "Char" || type == "char") return ScriptFieldType::Char;
		if (type == "Byte") return ScriptFieldType::Byte;
		if (type == "Int16") return ScriptFieldType::Short;
		if (type == "Int32" || type == "int") return ScriptFieldType::Int;
		if (type == "Int64" || type == "long") return ScriptFieldType::Long;
		if (type == "SByte") return ScriptFieldType::UByte;
		if (type == "UInt16") return ScriptFieldType::UShort;
		if (type == "UInt32") return ScriptFieldType::UInt;
		if (type == "UInt64") return ScriptFieldType::ULong;
		if (type == "Vector2") return ScriptFieldType::Vector2;
		if (type == "Vector3") return ScriptFieldType::Vector3;
		if (type == "Vector4") return ScriptFieldType::Vector4;
		if (type == "Entity") return ScriptFieldType::Entity;
		if (type == "String" || type == "string") return ScriptFieldType::String;
		return ScriptFieldType::None;
	}

	bool ScriptAssembly::Load(const std::filesystem::path& assemblyPath, const std::string& metadataJson)
	{
		Clear();
		m_AssemblyPath = assemblyPath;

		try
		{
			nlohmann::json metadata = nlohmann::json::parse(metadataJson);
			for (const auto& classJson : metadata["classes"])
			{
				std::string fullName = classJson.value("fullName", "");
				std::string namespaceName = classJson.value("namespace", "");
				std::string className = classJson.value("name", "");

				ScriptClass scriptClass(fullName, namespaceName, className);

				for (const auto& fieldJson : classJson["fields"])
				{
					ScriptField field;
					field.Name = fieldJson.value("name", "");
					field.TypeName = fieldJson.value("type", "");
					field.Type = ScriptFieldTypeFromString(field.TypeName);

					if (field.Type != ScriptFieldType::None)
						scriptClass.GetFields()[field.Name] = { field, fieldJson.value("defaultValue", "") };
				}

				if (!fullName.empty())
					m_Classes[fullName] = scriptClass;
			}
		}
		catch (const std::exception& e)
		{
			RD_CORE_ERROR("Failed to parse script metadata: {}", e.what());
			Clear();
			return false;
		}

		return true;
	}

	void ScriptAssembly::Clear()
	{
		m_Classes.clear();
		m_AssemblyPath.clear();
	}

	bool ScriptAssembly::HasClass(const std::string& className) const
	{
		return m_Classes.find(className) != m_Classes.end();
	}

	ScriptClass* ScriptAssembly::GetClass(const std::string& className)
	{
		auto it = m_Classes.find(className);
		if (it == m_Classes.end())
			return nullptr;
		return &it->second;
	}
}
