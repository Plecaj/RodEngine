#include "rdpch.h"

#include "ScriptAssembly.h"

#include "Rod/Scripting/ScriptClass.h"
#include "Rod/Scripting/ScriptField.h"

#include <json.hpp>
#include <stdexcept>

namespace Rod {

	ScriptFieldType ScriptAssembly::ScriptFieldTypeFromString(const std::string& type)
	{
		static const std::unordered_map<std::string, ScriptFieldType> FIELD_TYPES = {
		    {"Single", ScriptFieldType::Float},    {"float", ScriptFieldType::Float},
		    {"Double", ScriptFieldType::Double},   {"double", ScriptFieldType::Double},
		    {"Boolean", ScriptFieldType::Bool},    {"bool", ScriptFieldType::Bool},
		    {"Char", ScriptFieldType::Char},       {"char", ScriptFieldType::Char},
		    {"Byte", ScriptFieldType::Byte},       {"Int16", ScriptFieldType::Short},
		    {"Int32", ScriptFieldType::Int},       {"int", ScriptFieldType::Int},
		    {"Int64", ScriptFieldType::Long},      {"long", ScriptFieldType::Long},
		    {"SByte", ScriptFieldType::UByte},     {"UInt16", ScriptFieldType::UShort},
		    {"UInt32", ScriptFieldType::UInt},     {"UInt64", ScriptFieldType::ULong},
		    {"Vector2", ScriptFieldType::Vector2}, {"Vector3", ScriptFieldType::Vector3},
		    {"Vector4", ScriptFieldType::Vector4}, {"Entity", ScriptFieldType::Entity},
		    {"String", ScriptFieldType::String},   {"string", ScriptFieldType::String}};

		auto it = FIELD_TYPES.find(type);
		return it != FIELD_TYPES.end() ? it->second : ScriptFieldType::None;
	}

	ScriptAssembly::ScriptAssembly(const std::filesystem::path& assemblyPath, const std::string& metadataJson)
	    : m_AssemblyPath(assemblyPath)
	{
		try
		{
			const auto metadata = nlohmann::json::parse(metadataJson);
			if (metadata.contains("error"))
				throw std::runtime_error(
				    "Assembly load failed '" + assemblyPath.string() + "': " + metadata.at("error").get<std::string>());
			if (!metadata.is_object() || !metadata.contains("classes") || !metadata.at("classes").is_array())
				throw std::runtime_error("Assembly metadata invalid '" + assemblyPath.string() + "': classes required");
			for (const auto& classJson : metadata.at("classes"))
			{
				const auto fullName = classJson.at("fullName").get<std::string>();
				if (fullName.empty() || !classJson.at("fields").is_array())
					throw std::runtime_error(
					    "Assembly metadata invalid '" + assemblyPath.string() + "': class/fields required");
				const std::string namespaceName = classJson.value("namespace", "");
				const std::string className = classJson.value("name", "");

				ScriptClass scriptClass(fullName, namespaceName, className);

				for (const auto& fieldJson : classJson.at("fields"))
				{
					ScriptField field;
					field.Name = fieldJson.at("name").get<std::string>();
					field.TypeName = fieldJson.value("type", "");
					field.Type = ScriptFieldTypeFromString(field.TypeName);

					if (field.Type != ScriptFieldType::None)
						scriptClass.GetFields()[field.Name] = {field, fieldJson.value("defaultValue", "")};
				}

				m_Classes[fullName] = std::move(scriptClass);
			}
		}
		catch (const nlohmann::json::exception& exception)
		{
			throw std::runtime_error("Assembly metadata invalid '" + assemblyPath.string() + "': " + exception.what());
		}
	}

	bool ScriptAssembly::HasClass(const std::string& className) const
	{
		return m_Classes.contains(className);
	}

	ScriptClass* ScriptAssembly::GetClass(const std::string& className)
	{
		auto it = m_Classes.find(className);
		if (it == m_Classes.end())
			return nullptr;
		return &it->second;
	}
}
