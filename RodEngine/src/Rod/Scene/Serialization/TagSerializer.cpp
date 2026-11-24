#include "rdpch.h"

#include "ComponentSerializers.h"
#include "Rod/Scene/Components.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

namespace Rod::SceneSerialization {

	void TagSerializer::Write(YAML::Emitter& out, const TagComponent& tag)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Tag" << YAML::Value << tag.Tag;
		out << YAML::EndMap;
	}

	void TagSerializer::Read(const YAML::Node& node, TagComponent& tag)
	{
		tag.Tag = node["Tag"].as<std::string>();
	}
}
