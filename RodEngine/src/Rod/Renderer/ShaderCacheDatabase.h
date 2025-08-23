#pragma once

#include "Rod/Core/Core.h"

#include <unordered_set>
#include <vector>
#include <functional>
#include <filesystem>

#include <yaml-cpp/yaml.h>
#include <shaderc/shaderc.h>

namespace Rod{

	class ShaderCacheDatabase
	{
	public:
		bool CacheShader(size_t hash, std::vector<uint32_t> data);
		void AddValidHash(size_t hash) { m_ValidHashes.insert(hash); }
		bool IsHashInside(size_t hash);

		static ShaderCacheDatabase& Get()
		{
			static ShaderCacheDatabase instance;
			return instance;
		};

		~ShaderCacheDatabase();
	private:
		ShaderCacheDatabase();

		void CreateYAMLCacheDatabase();
		void ValidateCache();
		void SaveDatabase();
	private:
		std::unordered_set<size_t> m_ValidHashes;

		std::filesystem::path m_CacheDirectory = "assets/shaders/cached";
		std::filesystem::path m_CacheDatabase = m_CacheDirectory / "cache.yaml";
		YAML::Node m_CacheData;
	};
}