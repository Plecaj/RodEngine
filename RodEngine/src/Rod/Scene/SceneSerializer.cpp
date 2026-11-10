#include "rdpch.h"

#include "SceneSerializer.h"

#include "Components.h"
#include "Entity.h"
#include "Rod/Renderer/Mesh.h"
#include "Rod/Renderer/Texture.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scripting/ScriptField.h"
#include "Rod/Utils/FileUtils.h"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <stdexcept>
#include <vector>

namespace YAML {

	template <>
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
			if (!node.IsSequence() || node.size() != static_cast<size_t>(rhs.length()))
				return false;

			for (decltype(rhs.length()) component = 0; component < rhs.length(); ++component)
				rhs[component] = node[static_cast<size_t>(component)].as<float>();
			return true;
		}
	};

	template <>
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
			if (!node.IsSequence() || node.size() != static_cast<size_t>(rhs.length()))
				return false;

			for (decltype(rhs.length()) component = 0; component < rhs.length(); ++component)
				rhs[component] = node[static_cast<size_t>(component)].as<float>();
			return true;
		}
	};

}

namespace Rod {

	namespace {
		std::string RollbackLoad(Scene& scene, const std::vector<Entity>& createdEntities)
		{
			std::string rollbackErrors;
			for (auto it = createdEntities.rbegin(); it != createdEntities.rend(); ++it)
			{
				if (!*it)
					continue;
				try
				{
					scene.DestroyEntity(*it);
				}
				catch (const std::exception& exception)
				{
					rollbackErrors += " " + std::string(exception.what());
				}
				catch (...)
				{
					rollbackErrors += " Unknown error.";
				}
			}
			return rollbackErrors.empty() ? " | Scene unchanged." : " | Rollback incomplete:" + rollbackErrors;
		}
	}

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

	SceneSerializer::SceneSerializer(Scene& scene) : m_Scene(scene)
	{}

	void SceneSerializer::SerializeTagComponent(YAML::Emitter& out, const TagComponent& tag)
	{
		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Tag" << YAML::Value << tag.Tag;
		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeTransformComponent(YAML::Emitter& out, const TransformComponent& transform)
	{
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeCameraComponent(YAML::Emitter& out, const CameraComponent& cameraComponent)
	{
		const auto& camera = cameraComponent.Camera;

		out << YAML::Key << "CameraComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Camera" << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(camera.GetProjectionType());
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

	std::string SceneSerializer::NormalizeAssetPath(std::string path)
	{
		std::replace(path.begin(), path.end(), '\\', '/');
		return path;
	}

	void SceneSerializer::SerializeSpriteRendererComponent(YAML::Emitter& out, const SpriteRendererComponent& sprite)
	{
		out << YAML::Key << "SpriteRendererComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Color" << YAML::Value << sprite.Color;
		out << YAML::Key << "TilingFactor" << YAML::Value << sprite.TilingFactor;
		out << YAML::Key << "Texture" << YAML::Value
		    << (sprite.Texture ? NormalizeAssetPath(sprite.Texture->GetPath()) : "None");
		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeMeshComponent(YAML::Emitter& out, const MeshComponent& meshComponent)
	{
		out << YAML::Key << "MeshComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Path" << YAML::Value
		    << (meshComponent.Mesh ? NormalizeAssetPath(meshComponent.Mesh->GetPath()) : "None");

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

	void SceneSerializer::SerializeDirectionalLightComponent(YAML::Emitter& out, const DirectionalLightComponent& light)
	{
		out << YAML::Key << "DirectionalLightComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Direction" << YAML::Value << light.Direction;
		out << YAML::Key << "Color" << YAML::Value << light.Color;
		out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;
		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeScriptComponent(YAML::Emitter& out, const ScriptComponent& script)
	{
		out << YAML::Key << "ScriptComponent";
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

	void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(entity.GetUUID());

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
		auto view = m_Scene.m_Registry.view<IDComponent>();
		for (auto entityID : view)
		{
			const Entity entity = {entityID, &m_Scene};
			if (entity)
				SerializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		if (!out.good())
			throw std::runtime_error("Scene save failed '" + filepath + "': " + out.GetLastError());
		WriteTextFile(filepath, out.c_str());
	}

	void SceneSerializer::DeserializeTransformComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto transformComponent = entityNode["TransformComponent"];
		if (!transformComponent)
			return;

		auto& transform = entity.GetComponent<TransformComponent>();
		transform.Translation = transformComponent["Translation"].as<glm::vec3>();
		transform.Rotation = transformComponent["Rotation"].as<glm::vec3>();
		transform.Scale = transformComponent["Scale"].as<glm::vec3>();
	}

	void SceneSerializer::DeserializeCameraComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto cameraComponent = entityNode["CameraComponent"];
		if (!cameraComponent)
			return;

		auto& camera = entity.AddComponent<CameraComponent>();
		auto cameraProps = cameraComponent["Camera"];

		camera.Camera.SetProjectionType(
		    static_cast<SceneCamera::ProjectionType>(cameraProps["ProjectionType"].as<int>()));
		camera.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
		camera.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
		camera.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
		camera.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
		camera.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
		camera.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

		camera.Primary = cameraComponent["Primary"].as<bool>();
		camera.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
	}

	void SceneSerializer::DeserializeSpriteRendererComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto spriteRendererComponent = entityNode["SpriteRendererComponent"];
		if (!spriteRendererComponent)
			return;

		auto& sprite = entity.AddComponent<SpriteRendererComponent>();
		sprite.Color = spriteRendererComponent["Color"].as<glm::vec4>();
		sprite.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();

		const std::string texturePath = NormalizeAssetPath(spriteRendererComponent["Texture"].as<std::string>());
		if (texturePath != "None")
			sprite.Texture = Texture2D::Create(texturePath);
	}

	void SceneSerializer::DeserializeMeshComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto meshComponent = entityNode["MeshComponent"];
		if (!meshComponent)
			return;

		auto& mesh = entity.AddComponent<MeshComponent>();
		const std::string meshPath = NormalizeAssetPath(meshComponent["Path"].as<std::string>());
		if (meshPath != "None")
			mesh.SetMesh(CreateRef<Mesh>(meshPath));

		if (!mesh.Mesh)
			return;

		auto& material = mesh.Material;
		if (meshComponent["Albedo"])
			material.SetAlbedo(meshComponent["Albedo"].as<glm::vec4>());
		if (meshComponent["Emissive"])
			material.SetEmissive(meshComponent["Emissive"].as<glm::vec3>());
		if (meshComponent["Roughness"])
			material.SetRoughness(meshComponent["Roughness"].as<float>());
		if (meshComponent["Metallic"])
			material.SetMetallic(meshComponent["Metallic"].as<float>());
	}

	void SceneSerializer::DeserializeDirectionalLightComponent(const YAML::Node& entityNode, Entity entity)
	{
		auto directionalLightComponent = entityNode["DirectionalLightComponent"];
		if (!directionalLightComponent)
			return;

		auto& light = entity.AddComponent<DirectionalLightComponent>();
		light.Direction = directionalLightComponent["Direction"].as<glm::vec3>();
		light.Color = directionalLightComponent["Color"].as<glm::vec3>();
		light.Intensity = directionalLightComponent["Intensity"].as<float>();
	}

	void SceneSerializer::DeserializeScriptComponent(const YAML::Node& entityNode, Entity entity)
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
			field.Type = static_cast<ScriptFieldType>(fieldNode["Type"].as<int>());
			field.TypeName = fieldNode["TypeName"] ? fieldNode["TypeName"].as<std::string>() : "";

			ScriptFieldInstance instance;
			instance.Field = field;
			instance.Value = fieldNode["Value"] ? fieldNode["Value"].as<std::string>() : "";
			script.Fields[field.Name] = instance;
		}
	}

	void SceneSerializer::DeserializeEntity(
	    Scene& scene,
	    const YAML::Node& entityNode,
	    std::vector<Entity>& createdEntities)
	{
		auto uuid = entityNode["Entity"].as<uint64_t>();
		if (scene.FindEntityByUUID(UUID{uuid}))
			throw std::runtime_error("Duplicate entity UUID: " + std::to_string(uuid));

		std::string name;
		auto tagComponent = entityNode["TagComponent"];
		if (tagComponent)
			name = tagComponent["Tag"].as<std::string>();

		Entity entity = scene.CreateEntityWithUUID(UUID{uuid}, name);
		createdEntities.push_back(entity);
		DeserializeTransformComponent(entityNode, entity);
		DeserializeCameraComponent(entityNode, entity);
		DeserializeSpriteRendererComponent(entityNode, entity);
		DeserializeMeshComponent(entityNode, entity);
		DeserializeDirectionalLightComponent(entityNode, entity);
		DeserializeScriptComponent(entityNode, entity);
	}

	void SceneSerializer::DeserializeText(const std::string& filepath)
	{
		std::vector<Entity> createdEntities;
		try
		{
			if (filepath.empty())
				throw std::runtime_error("Path empty");
			std::ifstream file(filepath);
			if (!file.is_open())
				throw std::runtime_error("Open failed; check path/permissions");
			const auto data = YAML::Load(file);
			if (file.bad())
				throw std::runtime_error("Read failed");
			if (!data.IsMap())
				throw std::runtime_error("YAML mapping required");
			const auto sceneName = data["Scene"];
			if (!sceneName.IsDefined() || !sceneName.IsScalar())
				throw std::runtime_error("Field 'Scene': text required");

			const auto entities = data["Entities"];
			if (entities.IsDefined())
			{
				if (!entities.IsSequence())
					throw std::runtime_error("Field 'Entities': sequence required");
				createdEntities.reserve(entities.size());
				for (const auto& entityNode : entities)
				{
					if (!entityNode.IsMap() || !entityNode["Entity"])
						throw std::runtime_error("Field 'Entity': ID required");
					DeserializeEntity(m_Scene, entityNode, createdEntities);
				}
			}
		}
		catch (const std::exception& exception)
		{
			const auto rollback = RollbackLoad(m_Scene, createdEntities);
			throw std::runtime_error("Scene load failed '" + filepath + "': " + exception.what() + rollback);
		}
		catch (...)
		{
			RollbackLoad(m_Scene, createdEntities);
			throw;
		}
	}
}
