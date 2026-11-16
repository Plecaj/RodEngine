#pragma once

#include "Rod/Core/Core.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <unordered_set>
#include <vector>

namespace Rod {

	class ShaderCacheDatabase
	{
	public:
		bool CacheShader(const std::string& key, const std::vector<uint32_t>& data);
		void AddValidKey(const std::string& key) { m_ValidKeys.insert(key); }
		[[nodiscard]] bool Contains(const std::string& key) const;

		[[nodiscard]] static ShaderCacheDatabase& Get()
		{
			static ShaderCacheDatabase s_Instance;
			return s_Instance;
		};

		void Flush();
		[[nodiscard]] const std::filesystem::path& GetCacheDirectory() const { return m_CacheDirectory; }

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
