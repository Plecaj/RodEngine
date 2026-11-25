#pragma once

#include "Shader.h"
#include "ShaderCache.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>

namespace Rod {

	enum class ShaderOptimization : uint8_t
	{
		None,
		Performance,
		Size
	};

	struct ShaderCompileOptions
	{
		std::map<std::string, std::string> Macros;
		ShaderOptimization Optimization = ShaderOptimization::Performance;
		bool GenerateDebugInfo = false;
	};

	class ShaderLoader
	{
	public:
		ShaderLoader(std::filesystem::path sourceDirectory, std::filesystem::path cacheDirectory);

		[[nodiscard]] auto Load(const std::filesystem::path& path, const ShaderCompileOptions& options = {}) const
		    -> Scope<Shader>;

	private:
		std::filesystem::path m_SourceDirectory;
		ShaderCache m_Cache;
	};

}
