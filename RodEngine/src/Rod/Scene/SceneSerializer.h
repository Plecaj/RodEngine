#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Rod {
	class RenderDevice;
	class Scene;

	class SceneSerializer
	{
	public:
		explicit SceneSerializer(Scene& scene, std::filesystem::path projectDirectory = {}) noexcept;

		void SerializeText(const std::string& filepath) const;
		void DeserializeText(const std::string& filepath, const RenderDevice& device) const;
		[[nodiscard]] static auto ReadAssetReferences(const std::filesystem::path& filepath)
		    -> std::vector<std::filesystem::path>;

	private:
		Scene& m_Scene;
		std::filesystem::path m_ProjectDirectory;
	};
}
