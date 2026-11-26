#include "rdpch.h"

#include "ComponentSerializers.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Scene/Components.h"
#include "YamlVectors.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

namespace Rod::SceneSerialization {

	void SpriteRendererSerializer::Write(YAML::Emitter& out, const SpriteRendererComponent& sprite) const
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Color" << YAML::Value << sprite.Color;
		out << YAML::Key << "TilingFactor" << YAML::Value << sprite.TilingFactor;
		out << YAML::Key << s_AssetKey << YAML::Value
		    << (sprite.Texture ? Paths.ForSave(sprite.Texture->GetPath()) : "None");
		out << YAML::EndMap;
	}

	void SpriteRendererSerializer::Read(
	    const YAML::Node& node,
	    SpriteRendererComponent& sprite,
	    const RenderDevice& device) const
	{
		sprite.Color = node["Color"].as<glm::vec4>();
		sprite.TilingFactor = node["TilingFactor"].as<float>();

		const auto path = ReadAssetPath(node);
		sprite.Texture = path ? device.LoadTexture2D(Paths.ForLoad(*path)) : nullptr;
	}

	auto SpriteRendererSerializer::ReadAssetPath(const YAML::Node& node) -> std::optional<std::string>
	{
		return AssetPaths::ReadReference(node[s_AssetKey]);
	}
}
