#pragma once

#include "Rod/Core/Core.h"

#include <filesystem>
#include <unordered_set>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace Rod{

	class ShaderCacheDatabase
	{
	public:
		bool CacheShader(const std::string& key, const std::vector<uint32_t>& data);
		void AddValidKey(const std::string& key) { m_ValidKeys.insert(key); }
		bool Contains(const std::string& key) const;

		static ShaderCacheDatabase& Get()
		{
			static ShaderCacheDatabase instance;
			return instance;
		};

		void Flush();
	private:
		ShaderCacheDatabase();

		void CreateYAMLCacheDatabase();
		void ValidateCache();
		void SaveDatabase();
	private:
		std::unordered_set<std::string> m_ValidKeys;

		std::filesystem::path m_CacheDirectory = "assets/shaders/cached";
		std::filesystem::path m_CacheDatabase = m_CacheDirectory / "cache.yaml";
		YAML::Node m_CacheData;
	};
}
