#pragma once

#include "Rod/Core/Core.h"

#include <yaml-cpp/yaml.h>

#include <cstdint>
#include <filesystem>
#include <span>
#include <unordered_set>

namespace Rod {

	class ShaderCacheDatabase
	{
	public:
		explicit ShaderCacheDatabase(std::filesystem::path cacheDirectory);

		[[nodiscard]] auto CacheShader(const std::string& key, std::span<const uint32_t> data) -> bool;
		void AddValidKey(const std::string& key) { m_ValidKeys.insert(key); }
		[[nodiscard]] auto Contains(const std::string& key) const -> bool;

		[[nodiscard]] static auto Get() -> ShaderCacheDatabase&
		{
			static ShaderCacheDatabase s_Instance;
			return s_Instance;
		};

		void Flush();
		[[nodiscard]] auto GetCacheDirectory() const noexcept -> const std::filesystem::path&
		{
			return m_CacheDirectory;
		}

	private:
		ShaderCacheDatabase();

		void CreateYAMLCacheDatabase();
		void ValidateCache();
		void SaveDatabase();

	private:
		std::unordered_set<std::string> m_ValidKeys;

		std::filesystem::path m_CacheDirectory;
		std::filesystem::path m_CacheDatabase = m_CacheDirectory / "cache.yaml";
		YAML::Node m_CacheData;
	};
}
