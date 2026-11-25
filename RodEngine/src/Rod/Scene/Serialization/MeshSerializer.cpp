#include "rdpch.h"

#include "ComponentSerializers.h"
#include "Rod/Renderer/Mesh.h"
#include "Rod/Scene/Components.h"
#include "YamlVectors.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

namespace Rod::SceneSerialization {

	void MeshSerializer::Write(YAML::Emitter& out, const MeshComponent& meshComponent) const
	{
		out << YAML::BeginMap;
		out << YAML::Key << s_AssetKey << YAML::Value
		    << (meshComponent.Mesh ? Paths.ForSave(meshComponent.Mesh->GetPath()) : "None");

		if (meshComponent.Mesh)
		{
			const auto& material = meshComponent.Material;
			out << YAML::Key << "Albedo" << YAML::Value << material.GetAlbedo();
			out << YAML::Key << "Emissive" << YAML::Value << material.GetEmissive();
			out << YAML::Key << "Roughness" << YAML::Value << material.GetRoughness();
			out << YAML::Key << "Metallic" << YAML::Value << material.GetMetallic();
		}

		out << YAML::EndMap;
	}

	void MeshSerializer::Read(const YAML::Node& node, MeshComponent& mesh, const RenderDevice& device) const
	{
		const auto path = ReadAssetPath(node);
		mesh.SetMesh(path ? CreateRef<Mesh>(device, Paths.ForLoad(*path)) : nullptr);

		if (!mesh.Mesh)
			return;

		auto& material = mesh.Material;
		material.SetAlbedo(node["Albedo"].as<glm::vec4>());
		material.SetEmissive(node["Emissive"].as<glm::vec3>());
		material.SetRoughness(node["Roughness"].as<float>());
		material.SetMetallic(node["Metallic"].as<float>());
	}

	auto MeshSerializer::ReadAssetPath(const YAML::Node& node) -> std::optional<std::string>
	{
		return AssetPaths::ReadReference(node[s_AssetKey]);
	}
}
