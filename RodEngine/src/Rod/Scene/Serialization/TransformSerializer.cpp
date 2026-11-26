#include "rdpch.h"

#include "ComponentSerializers.h"
#include "Rod/Scene/Components.h"
#include "YamlVectors.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

namespace Rod::SceneSerialization {

	void TransformSerializer::Write(YAML::Emitter& out, const TransformComponent& transform)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
		out << YAML::EndMap;
	}

	void TransformSerializer::Read(const YAML::Node& node, TransformComponent& transform)
	{
		transform.Translation = node["Translation"].as<glm::vec3>();
		transform.Rotation = node["Rotation"].as<glm::vec3>();
		transform.Scale = node["Scale"].as<glm::vec3>();
	}
}
