#include "rdpch.h"

#include "SceneSerializer.h"

#include "Components.h"
#include "Entity.h"
#include "Rod/Utils/FileUtils.h"
#include "Scene.h"
#include "Serialization/ComponentSerializers.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <fstream>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace Rod {
	namespace {
		using SceneSerialization::AssetPaths;

		auto ReadSceneEntities(const std::filesystem::path& filepath) -> YAML::Node
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
			if (!entities.IsSequence())
				throw std::runtime_error("Field 'Entities': sequence required");
			return entities;
		}

		template <typename Visit>
		void VisitComponentSerializers(const AssetPaths& paths, Visit visit)
		{
			visit(SceneSerialization::TransformSerializer{});
			visit(SceneSerialization::CameraSerializer{});
			visit(SceneSerialization::SpriteRendererSerializer{paths});
			visit(SceneSerialization::MeshSerializer{paths});
			visit(SceneSerialization::DirectionalLightSerializer{});
			visit(SceneSerialization::ScriptSerializer{});
		}

		void WriteEntity(YAML::Emitter& out, const Entity entity, const AssetPaths& paths)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(entity.GetUUID());

			using TagSerializer = SceneSerialization::TagSerializer;
			out << YAML::Key << TagSerializer::s_Key << YAML::Value;
			TagSerializer::Write(out, entity.GetComponent<TagComponent>());

			VisitComponentSerializers(paths, [&](const auto& serializer) {
				using Serializer = std::remove_cvref_t<decltype(serializer)>;
				if (const auto* component = entity.TryGetComponent<typename Serializer::Component>())
				{
					out << YAML::Key << Serializer::s_Key << YAML::Value;
					serializer.Write(out, *component);
				}
			});
			out << YAML::EndMap;
		}

		auto
		ReadEntity(Scene& scene, const YAML::Node& node, const AssetPaths& paths, std::vector<Entity>& createdEntities)
		    -> Entity
		{
			using TagSerializer = SceneSerialization::TagSerializer;
			TagComponent tag;
			if (const auto tagNode = node[TagSerializer::s_Key])
				TagSerializer::Read(tagNode, tag);

			const auto id = node["Entity"].as<uint64_t>();
			auto entity = scene.CreateEntityWithUUID(UUID{id}, tag.Tag);
			createdEntities.push_back(entity);

			VisitComponentSerializers(paths, [&](const auto& serializer) {
				using Serializer = std::remove_cvref_t<decltype(serializer)>;
				using Component = Serializer::Component;
				if (const auto componentNode = node[Serializer::s_Key])
				{
					if constexpr (OptionalComponent<Component>)
					{
						auto& component = entity.AddComponent<Component>();
						serializer.Read(componentNode, component);
					}
					else
					{
						serializer.Read(componentNode, entity.GetComponent<Component>());
					}
				}
			});
			return entity;
		}

		template <typename Serializer>
		void AppendAssetReference(const YAML::Node& entity, std::vector<std::filesystem::path>& paths)
		{
			if (const auto component = entity[Serializer::s_Key])
			{
				if (const auto path = Serializer::ReadAssetPath(component))
					paths.emplace_back(*path);
			}
		}

		auto RollbackLoad(Scene& scene, const std::vector<Entity>& createdEntities) -> std::string
		{
			std::string rollbackErrors;
			for (const auto& createdEntity : std::views::reverse(createdEntities))
			{
				if (!createdEntity)
					continue;
				try
				{
					scene.DestroyEntity(createdEntity);
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

	SceneSerializer::SceneSerializer(Scene& scene, std::filesystem::path projectDirectory) noexcept
	    : m_Scene(scene), m_ProjectDirectory(std::move(projectDirectory))
	{}

	void SceneSerializer::SerializeText(const std::string& filepath) const
	{
		const SceneSerialization::AssetPaths paths(m_ProjectDirectory);
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		const auto view = m_Scene.View<IDComponent>();
		for (const auto entityID : view)
		{
			WriteEntity(out, m_Scene.FindEntityByHandle(entityID), paths);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		if (!out.good())
			throw std::runtime_error("Scene save failed '" + filepath + "': " + out.GetLastError());
		WriteTextFile(filepath, out.c_str());
	}

	void SceneSerializer::DeserializeText(const std::string& filepath) const
	{
		const SceneSerialization::AssetPaths paths(m_ProjectDirectory);
		std::vector<Entity> createdEntities;
		try
		{
			const auto entities = ReadSceneEntities(filepath);
			createdEntities.reserve(entities.size());
			bool hasPrimaryCamera = static_cast<bool>(m_Scene.FindPrimaryCamera());
			for (const auto& entityNode : entities)
			{
				if (!entityNode.IsMap() || !entityNode["Entity"])
					throw std::runtime_error("Field 'Entity': ID required");
				const auto entity = ReadEntity(m_Scene, entityNode, paths, createdEntities);
				const auto* camera = entity.TryGetComponent<CameraComponent>();
				if (camera && camera->Primary)
				{
					if (hasPrimaryCamera)
						m_Scene.SetCameraPrimary(entity, false);
					hasPrimaryCamera = true;
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

	auto SceneSerializer::ReadAssetReferences(const std::filesystem::path& filepath)
	    -> std::vector<std::filesystem::path>
	{
		try
		{
			std::vector<std::filesystem::path> paths;
			for (const auto& entity : ReadSceneEntities(filepath))
			{
				AppendAssetReference<SceneSerialization::SpriteRendererSerializer>(entity, paths);
				AppendAssetReference<SceneSerialization::MeshSerializer>(entity, paths);
			}
			return paths;
		}
		catch (const std::exception& exception)
		{
			throw std::runtime_error("Scene asset references failed '" + filepath.string() + "': " + exception.what());
		}
	}
}
