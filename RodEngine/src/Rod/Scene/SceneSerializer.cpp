#include "rdpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

}

namespace Rod {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeTagComponent(YAML::Emitter& out, const TagComponent& tag)
	{
		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Tag" << YAML::Value << tag.Tag;
		out << YAML::EndMap;
	}

	static void SerializeTransformComponent(YAML::Emitter& out, const TransformComponent& transform)
	{
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
		out << YAML::EndMap;
	}

	static void SerializeCameraComponent(YAML::Emitter& out, const CameraComponent& cameraComponent)
	{
		const auto& camera = cameraComponent.Camera;

		out << YAML::Key << "CameraComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Camera" << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
		out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
		out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
		out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
		out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
		out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
		out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
		out << YAML::EndMap;

		out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
		out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
		out << YAML::EndMap;
	}

	static std::string NormalizeAssetPath(std::string path)
	{
		std::replace(path.begin(), path.end(), '\\', '/');
		return path;
	}

	static void SerializeSpriteRendererComponent(YAML::Emitter& out, const SpriteRendererComponent& sprite)
	{
		out << YAML::Key << "SpriteRendererComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Color" << YAML::Value << sprite.Color;
		out << YAML::Key << "TilingFactor" << YAML::Value << sprite.TilingFactor;
		out << YAML::Key << "Texture" << YAML::Value << (sprite.Texture ? NormalizeAssetPath(sprite.Texture->GetPath()) : "None");
		out << YAML::EndMap;
	}

	static void SerializeMeshComponent(YAML::Emitter& out, const MeshComponent& meshComponent)
	{
		out << YAML::Key << "MeshComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Path" << YAML::Value << (meshComponent.Mesh ? NormalizeAssetPath(meshComponent.Mesh->GetPath()) : "None");

		if (meshComponent.Mesh && meshComponent.Mesh->GetMaterial())
		{
			auto material = meshComponent.Mesh->GetMaterial();
			out << YAML::Key << "Albedo" << YAML::Value << material->GetAlbedo();
			out << YAML::Key << "Emissive" << YAML::Value << material->GetEmissive();
			out << YAML::Key << "Roughness" << YAML::Value << material->GetRoughness();
			out << YAML::Key << "Metallic" << YAML::Value << material->GetMetallic();
		}

		out << YAML::EndMap;
	}

	static void SerializeDirectionalLightComponent(YAML::Emitter& out, const DirectionalLightComponent& light)
	{
		out << YAML::Key << "DirectionalLightComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Direction" << YAML::Value << light.Direction;
		out << YAML::Key << "Color" << YAML::Value << light.Color;
		out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;
		out << YAML::EndMap;
	}

	static void SerializeScriptComponent(YAML::Emitter& out, const ScriptComponent& script)
	{
		out << YAML::Key << "ScriptComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "ClassName" << YAML::Value << script.ClassName;
		out << YAML::Key << "Fields" << YAML::Value << YAML::BeginSeq;
		for (const auto& [name, field] : script.Fields)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << name;
			out << YAML::Key << "Type" << YAML::Value << (int)field.Field.Type;
			out << YAML::Key << "TypeName" << YAML::Value << field.Field.TypeName;
			out << YAML::Key << "Value" << YAML::Value << field.Value;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << (uint64_t)entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
			SerializeTagComponent(out, entity.GetComponent<TagComponent>());
		if (entity.HasComponent<TransformComponent>())
			SerializeTransformComponent(out, entity.GetComponent<TransformComponent>());
		if (entity.HasComponent<CameraComponent>())
			SerializeCameraComponent(out, entity.GetComponent<CameraComponent>());
		if (entity.HasComponent<SpriteRendererComponent>())
			SerializeSpriteRendererComponent(out, entity.GetComponent<SpriteRendererComponent>());
		if (entity.HasComponent<MeshComponent>())
			SerializeMeshComponent(out, entity.GetComponent<MeshComponent>());
		if (entity.HasComponent<DirectionalLightComponent>())
			SerializeDirectionalLightComponent(out, entity.GetComponent<DirectionalLightComponent>());
		if (entity.HasComponent<ScriptComponent>())
			SerializeScriptComponent(out, entity.GetComponent<ScriptComponent>());

		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeText(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		auto view = m_Scene->m_Registry.view<IDComponent>();
		for (auto entityID : view)
		{
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				continue;

			SerializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::filesystem::path path = filepath;
		if (path.has_parent_path())
			std::filesystem::create_directories(path.parent_path());

		std::ofstream fout(filepath);
		if (!fout.is_open())
		{
			RD_CORE_ERROR("Failed to open file: {}", filepath);
			return;
		}
		fout << out.c_str();
	}

	static void DeserializeTransformComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto transformComponent = entityNode["TransformComponent"];
		if (!transformComponent)
			return;

		auto& transform = entity.GetComponent<TransformComponent>();
		transform.Translation = transformComponent["Translation"].as<glm::vec3>();
		transform.Rotation = transformComponent["Rotation"].as<glm::vec3>();
		transform.Scale = transformComponent["Scale"].as<glm::vec3>();
	}

	static void DeserializeCameraComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto cameraComponent = entityNode["CameraComponent"];
		if (!cameraComponent)
			return;

		auto& camera = entity.AddComponent<CameraComponent>();
		auto cameraProps = cameraComponent["Camera"];

		camera.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
		camera.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
		camera.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
		camera.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
		camera.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
		camera.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
		camera.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

		camera.Primary = cameraComponent["Primary"].as<bool>();
		camera.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
	}

	static void DeserializeSpriteRendererComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto spriteRendererComponent = entityNode["SpriteRendererComponent"];
		if (!spriteRendererComponent)
			return;

		auto& sprite = entity.AddComponent<SpriteRendererComponent>();
		sprite.Color = spriteRendererComponent["Color"].as<glm::vec4>();
		sprite.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();

		std::string texturePath = NormalizeAssetPath(spriteRendererComponent["Texture"].as<std::string>());
		if (texturePath != "None")
			sprite.Texture = Texture2D::Create(texturePath);
	}

	static void DeserializeMeshComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto meshComponent = entityNode["MeshComponent"];
		if (!meshComponent)
			return;

		auto& mesh = entity.AddComponent<MeshComponent>();
		std::string meshPath = NormalizeAssetPath(meshComponent["Path"].as<std::string>());
		if (meshPath != "None")
			mesh.Mesh = Mesh::Create(meshPath);

		if (!mesh.Mesh || !mesh.Mesh->GetMaterial())
			return;

		auto material = mesh.Mesh->GetMaterial();
		if (meshComponent["Albedo"])
			material->SetAlbedo(meshComponent["Albedo"].as<glm::vec4>());
		if (meshComponent["Emissive"])
			material->SetEmissive(meshComponent["Emissive"].as<glm::vec3>());
		if (meshComponent["Roughness"])
			material->SetRoughness(meshComponent["Roughness"].as<float>());
		if (meshComponent["Metallic"])
			material->SetMetallic(meshComponent["Metallic"].as<float>());
	}

	static void DeserializeDirectionalLightComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto directionalLightComponent = entityNode["DirectionalLightComponent"];
		if (!directionalLightComponent)
			return;

		auto& light = entity.AddComponent<DirectionalLightComponent>();
		light.Direction = directionalLightComponent["Direction"].as<glm::vec3>();
		light.Color = directionalLightComponent["Color"].as<glm::vec3>();
		light.Intensity = directionalLightComponent["Intensity"].as<float>();
	}

	static void DeserializeScriptComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto scriptComponent = entityNode["ScriptComponent"];
		if (!scriptComponent)
			return;

		auto& script = entity.AddComponent<ScriptComponent>();
		script.ClassName = scriptComponent["ClassName"] ? scriptComponent["ClassName"].as<std::string>() : "";

		auto fields = scriptComponent["Fields"];
		if (!fields)
			return;

		for (auto fieldNode : fields)
		{
			ScriptField field;
			field.Name = fieldNode["Name"].as<std::string>();
			field.Type = (ScriptFieldType)fieldNode["Type"].as<int>();
			field.TypeName = fieldNode["TypeName"] ? fieldNode["TypeName"].as<std::string>() : "";

			ScriptFieldInstance instance;
			instance.Field = field;
			instance.Value = fieldNode["Value"] ? fieldNode["Value"].as<std::string>() : "";
			script.Fields[field.Name] = instance;
		}
	}

	static Entity DeserializeEntity(Scene& scene, const YAML::Node& entityNode)
	{
		uint64_t uuid = entityNode["Entity"].as<uint64_t>();

		std::string name;
		auto tagComponent = entityNode["TagComponent"];
		if (tagComponent)
			name = tagComponent["Tag"].as<std::string>();

		RD_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

		Entity entity = scene.CreateEntityWithUUID(uuid, name);
		DeserializeTransformComponent(entityNode, entity);
		DeserializeCameraComponent(entityNode, entity);
		DeserializeSpriteRendererComponent(entityNode, entity);
		DeserializeMeshComponent(entityNode, entity);
		DeserializeDirectionalLightComponent(entityNode, entity);
		DeserializeScriptComponent(entityNode, entity);
		return entity;
	}

	bool SceneSerializer::DeserializeText(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (const YAML::Exception& exception)
		{
			RD_CORE_ERROR("Failed to deserialize scene '{}': {}", filepath, exception.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		RD_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (!entities)
			return true;

		for (auto entityNode : entities)
		{
			if (entityNode["Entity"])
				DeserializeEntity(*m_Scene, entityNode);
		}

		return true;
	}
}
