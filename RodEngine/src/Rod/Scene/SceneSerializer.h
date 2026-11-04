#pragma once

#include <string>

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
		explicit SceneSerializer(Scene& scene);

		void SerializeText(const std::string& filepath);

		bool DeserializeText(const std::string& filepath);

	private:
		static void SerializeTagComponent(YAML::Emitter& out, const TagComponent& tag);
		static void SerializeTransformComponent(YAML::Emitter& out, const TransformComponent& transform);
		static void SerializeCameraComponent(YAML::Emitter& out, const CameraComponent& cameraComponent);
		static std::string NormalizeAssetPath(std::string path);
		static void SerializeSpriteRendererComponent(YAML::Emitter& out, const SpriteRendererComponent& sprite);
		static void SerializeMeshComponent(YAML::Emitter& out, const MeshComponent& meshComponent);
		static void SerializeDirectionalLightComponent(YAML::Emitter& out, const DirectionalLightComponent& light);
		static void SerializeScriptComponent(YAML::Emitter& out, const ScriptComponent& script);
		static void SerializeEntity(YAML::Emitter& out, Entity entity);
		static void DeserializeTransformComponent(const YAML::Node& entityNode, Entity entity);
		static void DeserializeCameraComponent(const YAML::Node& entityNode, Entity entity);
		static void DeserializeSpriteRendererComponent(const YAML::Node& entityNode, Entity entity);
		static void DeserializeMeshComponent(const YAML::Node& entityNode, Entity entity);
		static void DeserializeDirectionalLightComponent(const YAML::Node& entityNode, Entity entity);
		static void DeserializeScriptComponent(const YAML::Node& entityNode, Entity entity);
		static Entity DeserializeEntity(Scene& scene, const YAML::Node& entityNode);

		Scene& m_Scene;
	};

}
