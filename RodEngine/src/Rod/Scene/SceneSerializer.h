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
	struct TagComponent;
	struct TransformComponent;
	struct CameraComponent;
	struct SpriteRendererComponent;
	struct MeshComponent;
	struct DirectionalLightComponent;
	struct ScriptComponent;

	class SceneSerializer
	{
	public:
		explicit SceneSerializer(Scene& scene, std::filesystem::path projectDirectory = {});

		void SerializeText(const std::string& filepath);
		void DeserializeText(const std::string& filepath);

	private:
		static void SerializeTagComponent(YAML::Emitter& out, const TagComponent& tag);
		static void SerializeTransformComponent(YAML::Emitter& out, const TransformComponent& transform);
		static void SerializeCameraComponent(YAML::Emitter& out, const CameraComponent& cameraComponent);
		static std::string NormalizeAssetPath(std::string path);
		std::string SerializeAssetPath(const std::string& path) const;
		std::string ResolveAssetPath(const std::string& path) const;
		void SerializeSpriteRendererComponent(YAML::Emitter& out, const SpriteRendererComponent& sprite) const;
		void SerializeMeshComponent(YAML::Emitter& out, const MeshComponent& meshComponent) const;
		static void SerializeDirectionalLightComponent(YAML::Emitter& out, const DirectionalLightComponent& light);
		static void SerializeScriptComponent(YAML::Emitter& out, const ScriptComponent& script);
		void SerializeEntity(YAML::Emitter& out, Entity entity) const;
		static void DeserializeTransformComponent(const YAML::Node& entityNode, Entity entity);
		static void DeserializeCameraComponent(const YAML::Node& entityNode, Entity entity);
		void DeserializeSpriteRendererComponent(const YAML::Node& entityNode, Entity entity) const;
		void DeserializeMeshComponent(const YAML::Node& entityNode, Entity entity) const;
		static void DeserializeDirectionalLightComponent(const YAML::Node& entityNode, Entity entity);
		static void DeserializeScriptComponent(const YAML::Node& entityNode, Entity entity);
		void DeserializeEntity(Scene& scene, const YAML::Node& entityNode, std::vector<Entity>& createdEntities) const;

		Scene& m_Scene;
		std::filesystem::path m_ProjectDirectory;
	};

}
