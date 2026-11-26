#include "rdpch.h"

#include "ComponentSerializers.h"
#include "Rod/Scene/Components.h"
#include "Rod/Scripting/ScriptField.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <stdexcept>

namespace Rod::SceneSerialization {

	void ScriptSerializer::Write(YAML::Emitter& out, const ScriptComponent& script)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "ClassName" << YAML::Value << script.ClassName;
		out << YAML::Key << "Fields" << YAML::Value << YAML::BeginSeq;
		for (const auto& [name, field] : script.Fields)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << name;
			out << YAML::Key << "Type" << YAML::Value << static_cast<int>(field.Field.Type);
			out << YAML::Key << "TypeName" << YAML::Value << field.Field.TypeName;
			out << YAML::Key << "Value" << YAML::Value << field.Value;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	void ScriptSerializer::Read(const YAML::Node& node, ScriptComponent& script)
	{
		script.ClassName = node["ClassName"].as<std::string>();

		const auto fields = node["Fields"];
		if (!fields.IsSequence())
			throw std::runtime_error("Field 'Fields': sequence required");

		for (auto fieldNode : fields)
		{
			ScriptField field;
			field.Name = fieldNode["Name"].as<std::string>();
			field.Type = static_cast<ScriptFieldType>(fieldNode["Type"].as<int>());
			field.TypeName = fieldNode["TypeName"].as<std::string>();

			ScriptFieldInstance instance;
			instance.Field = field;
			instance.Value = fieldNode["Value"].as<std::string>();
			script.Fields[field.Name] = instance;
		}
	}
}
