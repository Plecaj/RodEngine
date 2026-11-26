#include "rdpch.h"

#include "ComponentSerializers.h"

#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <algorithm>
#include <utility>

namespace Rod::SceneSerialization {
	namespace {
		auto NormalizeAssetPath(std::string path) -> std::string
		{
			std::ranges::replace(path, '\\', '/');
			return path;
		}
	}

	AssetPaths::AssetPaths(std::filesystem::path projectDirectory) : m_ProjectDirectory(std::move(projectDirectory))
	{}

	auto AssetPaths::ForSave(const std::string& path) const -> std::string
	{
		const std::filesystem::path assetPath = NormalizeAssetPath(path);
		if (m_ProjectDirectory.empty())
			return assetPath.generic_string();
		return assetPath.lexically_normal().lexically_relative(m_ProjectDirectory.lexically_normal()).generic_string();
	}

	auto AssetPaths::ForLoad(const std::string& path) const -> std::string
	{
		return (m_ProjectDirectory / NormalizeAssetPath(path)).lexically_normal().string();
	}

	auto AssetPaths::ReadReference(const YAML::Node& node) -> std::optional<std::string>
	{
		auto path = NormalizeAssetPath(node.as<std::string>());
		if (path == "None")
			return std::nullopt;
		return path;
	}
}
