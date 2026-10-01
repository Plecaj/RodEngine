#include "rdpch.h"
#include "ScriptAssembly.h"

#include <json.hpp>
#include <unordered_map>

namespace Rod {

	static ScriptFieldType ScriptFieldTypeFromString(const std::string& type)
	{
		static const std::unordered_map<std::string, ScriptFieldType> fieldTypes = {
			{ "Single", ScriptFieldType::Float },
			{ "float", ScriptFieldType::Float },
			{ "Double", ScriptFieldType::Double },
			{ "double", ScriptFieldType::Double },
			{ "Boolean", ScriptFieldType::Bool },
			{ "bool", ScriptFieldType::Bool },
			{ "Char", ScriptFieldType::Char },
			{ "char", ScriptFieldType::Char },
			{ "Byte", ScriptFieldType::Byte },
			{ "Int16", ScriptFieldType::Short },
			{ "Int32", ScriptFieldType::Int },
			{ "int", ScriptFieldType::Int },
			{ "Int64", ScriptFieldType::Long },
			{ "long", ScriptFieldType::Long },
			{ "SByte", ScriptFieldType::UByte },
			{ "UInt16", ScriptFieldType::UShort },
			{ "UInt32", ScriptFieldType::UInt },
			{ "UInt64", ScriptFieldType::ULong },
			{ "Vector2", ScriptFieldType::Vector2 },
			{ "Vector3", ScriptFieldType::Vector3 },
			{ "Vector4", ScriptFieldType::Vector4 },
			{ "Entity", ScriptFieldType::Entity },
			{ "String", ScriptFieldType::String },
			{ "string", ScriptFieldType::String }
		};

		auto it = fieldTypes.find(type);
		return it != fieldTypes.end() ? it->second : ScriptFieldType::None;
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
