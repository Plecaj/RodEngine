#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace YAML {
	class Emitter;
	class Node;
}

namespace Rod {
	class Scene;
	class Entity;
	struct SpriteRendererComponent;
	struct MeshComponent;

	class SceneSerializer
	{
	public:
		explicit SceneSerializer(Scene& scene, std::filesystem::path projectDirectory = {}) noexcept;

		void SerializeText(const std::string& filepath) const;
		void DeserializeText(const std::string& filepath) const;

	private:
		[[nodiscard]] auto SerializeAssetPath(const std::string& path) const -> std::string;
		[[nodiscard]] auto ResolveAssetPath(const std::string& path) const -> std::string;
		void SerializeSpriteRendererComponent(YAML::Emitter& out, const SpriteRendererComponent& sprite) const;
		void SerializeMeshComponent(YAML::Emitter& out, const MeshComponent& meshComponent) const;
		void SerializeEntity(YAML::Emitter& out, Entity entity) const;
		void DeserializeSpriteRendererComponent(const YAML::Node& entityNode, Entity entity) const;
		void DeserializeMeshComponent(const YAML::Node& entityNode, Entity entity) const;
		void DeserializeEntity(Scene& scene, const YAML::Node& entityNode, std::vector<Entity>& createdEntities) const;

		Scene& m_Scene;
		std::filesystem::path m_ProjectDirectory;
	};

}
