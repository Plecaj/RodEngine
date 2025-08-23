#include "rdpch.h"
#include "ShaderCacheDatabase.h"

namespace Rod {

	ShaderCacheDatabase::ShaderCacheDatabase()
	{
		if (!std::filesystem::exists(m_CacheDirectory))
		{
			bool created = std::filesystem::create_directories(m_CacheDirectory);
			if(!created)
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

	ShaderCacheDatabase::~ShaderCacheDatabase()
	{
		ValidateCache();
		SaveDatabase();
	}

	bool ShaderCacheDatabase::CacheShader(size_t hash, std::vector<uint32_t> data)
	{
		m_ValidHashes.insert(hash);

		std::filesystem::path filename = m_CacheDirectory / std::to_string(hash);
		std::ofstream fout(filename, std::ios::binary | std::ios::out | std::ios::trunc);
		if (!fout)
			return false;

		fout.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint32_t));
		m_CacheData["Hashes"].push_back(std::to_string(hash));
		return true;
	}

	bool ShaderCacheDatabase::IsHashInside(size_t hash)
	{
		for (auto hashEntry : m_CacheData["Hashes"])
		{
			if (hashEntry.as<size_t>() == hash) return true;
		}
		return false;
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

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CacheDirectory))
		{
			if (!directoryEntry.is_directory())
				existingFiles.insert(directoryEntry.path().filename().string());
		}

		std::unordered_set<std::string> allowedFiles;
		allowedFiles.insert(m_CacheDatabase.filename().string());

		// Delete yaml entries that shouldnt be in database anymore
		// Coresponding file is missing or Shader is changed and its outdated
		if (m_CacheData["Hashes"])
		{
			YAML::Node newHashes(YAML::NodeType::Sequence);

			for (const auto& hashNode : m_CacheData["Hashes"])
			{
				std::string hashStr = hashNode.as<std::string>();
				size_t hash = std::stoull(hashStr);
				if (existingFiles.find(hashStr) != existingFiles.end() && m_ValidHashes.find(hash) != m_ValidHashes.end())
				{
					allowedFiles.insert(hashStr);
					newHashes.push_back(hashStr);
				}
				else
				{
					RD_CORE_WARN("Removing hash {} from YAML cache because file is missing or is outdated", hashStr);
				}
			}

			m_CacheData["Hashes"] = newHashes;
		}

		// Delete files not listed in yaml
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CacheDirectory))
		{
			std::string name = directoryEntry.path().filename().string();
			if (allowedFiles.find(name) == allowedFiles.end())
			{
				if (directoryEntry.is_directory())
					std::filesystem::remove_all(directoryEntry.path());
				else
					std::filesystem::remove(directoryEntry.path());

				RD_CORE_WARN("Removing {} because there is no coresponding YAML entry", directoryEntry.path().string());
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