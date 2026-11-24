#include "rdpch.h"

#include "ComponentSerializers.h"
#include "Rod/Scene/Components.h"
#include "YamlVectors.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

namespace Rod::SceneSerialization {

	void DirectionalLightSerializer::Write(YAML::Emitter& out, const DirectionalLightComponent& light)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Direction" << YAML::Value << light.Direction;
		out << YAML::Key << "Color" << YAML::Value << light.Color;
		out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;
		out << YAML::EndMap;
	}

	void DirectionalLightSerializer::Read(const YAML::Node& node, DirectionalLightComponent& light)
	{
		light.Direction = node["Direction"].as<glm::vec3>();
		light.Color = node["Color"].as<glm::vec3>();
		light.Intensity = node["Intensity"].as<float>();
	}
}
