#include "rdpch.h"

#include "ShaderCacheDatabase.h"

#include "Renderer.h"

#include <algorithm>
#include <limits>

namespace Rod {

	ShaderCacheDatabase::ShaderCacheDatabase() : ShaderCacheDatabase(Renderer::GetShaderDirectory() / "Cached")
	{}

	ShaderCacheDatabase::ShaderCacheDatabase(std::filesystem::path cacheDirectory)
	    : m_CacheDirectory(std::move(cacheDirectory))
	{
		if (!std::filesystem::exists(m_CacheDirectory))
		{
			const bool created = std::filesystem::create_directories(m_CacheDirectory);
			if (!created)
				RD_CORE_ERROR("Directory for shader cache could not be created");
		}

		if (!std::filesystem::exists(m_CacheDatabase))
			CreateYAMLCacheDatabase();

		m_CacheData = YAML::LoadFile(m_CacheDatabase.string());

		if (!m_CacheData["Hashes"])
		{
			CreateYAMLCacheDatabase();
			m_CacheData = YAML::LoadFile(m_CacheDatabase.string());
		}
	}

	void ShaderCacheDatabase::Flush()
	{
		ValidateCache();
		SaveDatabase();
	}

	auto ShaderCacheDatabase::CacheShader(const std::string& key, std::span<const uint32_t> data) -> bool
	{
		const auto maxWords = static_cast<size_t>(std::numeric_limits<std::streamsize>::max()) / sizeof(uint32_t);
		if (data.size() > maxWords)
			return false;

		m_ValidKeys.insert(key);

		const std::filesystem::path filename = m_CacheDirectory / key;
		std::ofstream fout(filename, std::ios::binary | std::ios::out | std::ios::trunc);
		if (!fout)
			return false;

		const auto byteCount = static_cast<std::streamsize>(data.size_bytes());
		// Binary streams consume the object representation of the SPIR-V words as bytes.
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		fout.write(reinterpret_cast<const char*>(data.data()), byteCount);
		if (!fout)
			return false;
		if (!Contains(key))
			m_CacheData["Hashes"].push_back(key);
		return true;
	}

	auto ShaderCacheDatabase::Contains(const std::string& key) const -> bool
	{
		const auto hashes = m_CacheData["Hashes"];
		return std::any_of(hashes.begin(), hashes.end(), [&key](const auto& hashEntry) {
			return hashEntry.template as<std::string>() == key;
		});
	}

	void ShaderCacheDatabase::CreateYAMLCacheDatabase()
	{
		YAML::Node root;
		root["Hashes"] = YAML::Node(YAML::NodeType::Sequence);

		std::ofstream fout(m_CacheDatabase, std::ios::out | std::ios::trunc);
		if (!fout.is_open())
		{
			RD_CORE_ERROR("Failed to open file: {}", m_CacheDatabase.string());
			return;
		}
		fout << root;
	}

	void ShaderCacheDatabase::ValidateCache()
	{
		std::unordered_set<std::string> existingFiles;

		for (const auto& directoryEntry : std::filesystem::directory_iterator(m_CacheDirectory))
		{
			if (!directoryEntry.is_directory())
				existingFiles.insert(directoryEntry.path().filename().string());
		}

		std::unordered_set<std::string> allowedFiles;
		allowedFiles.insert(m_CacheDatabase.filename().string());

		if (m_CacheData["Hashes"])
		{
			YAML::Node newHashes(YAML::NodeType::Sequence);

			for (const auto& hashNode : m_CacheData["Hashes"])
			{
				auto key = hashNode.as<std::string>();
				if (existingFiles.contains(key) && m_ValidKeys.contains(key))
				{
					allowedFiles.insert(key);
					newHashes.push_back(key);
				}
				else
				{
					RD_CORE_WARN("Removing shader cache key {} because file is missing or is outdated", key);
				}
			}

			m_CacheData["Hashes"] = newHashes;
		}

		for (const auto& directoryEntry : std::filesystem::directory_iterator(m_CacheDirectory))
		{
			const std::string name = directoryEntry.path().filename().string();
			if (!allowedFiles.contains(name))
			{
				if (directoryEntry.is_directory())
					std::filesystem::remove_all(directoryEntry.path());
				else
					std::filesystem::remove(directoryEntry.path());

				RD_CORE_WARN(
				    "Removing {} because there is no corresponding YAML entry",
				    directoryEntry.path().string());
			}
		}
	}

	void ShaderCacheDatabase::SaveDatabase()
	{
		std::ofstream fout(m_CacheDatabase, std::ios::out | std::ios::trunc);
		if (!fout.is_open())
		{
			RD_CORE_ERROR("Failed to open file: {}", m_CacheDatabase.string());
			return;
		}
		fout << m_CacheData;
	}

}
