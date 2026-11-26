#include "rdpch.h"

#include "ShaderLoader.h"

#include "Rod/Platform/OpenGL/OpenGLShader.h"
#include "ShaderStage.h"

#include <shaderc/shaderc.hpp>

#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace Rod {
	namespace {
		constexpr auto g_Target = shaderc_target_env_vulkan;
		constexpr auto g_TargetVersion = shaderc_env_version_vulkan_1_2;

		struct StageSource
		{
			ShaderStage Stage;
			std::string_view Text;
		};

		auto ParseStage(std::string_view name) -> ShaderStage
		{
			if (name == "vertex")
				return ShaderStage::Vertex;
			if (name == "fragment" || name == "pixel")
				return ShaderStage::Fragment;
			if (name == "geometry")
				return ShaderStage::Geometry;
			if (name == "tess_control" || name == "hull")
				return ShaderStage::TessellationControl;
			if (name == "tess_evaluation" || name == "domain")
				return ShaderStage::TessellationEvaluation;
			if (name == "compute")
				return ShaderStage::Compute;
			throw std::runtime_error("Unknown shader stage '" + std::string(name) + "'");
		}

		auto SplitStages(std::string_view source) -> std::vector<StageSource>
		{
			constexpr std::string_view whitespace = " \t\r\n";
			std::vector<StageSource> stages;
			std::optional<ShaderStage> stage;
			size_t stageStart = 0;
			const auto finishStage = [&](size_t end) {
				if (!stage)
					return;
				const auto text = source.substr(stageStart, end - stageStart);
				if (text.find_first_not_of(whitespace) == std::string_view::npos)
					throw std::runtime_error("Shader stage has no source");
				if (std::ranges::contains(stages, *stage, &StageSource::Stage))
					throw std::runtime_error("Duplicate shader stage");
				stages.push_back({.Stage = *stage, .Text = text});
			};

			for (size_t start = 0; start < source.size();)
			{
				const size_t newline = source.find('\n', start);
				const size_t end = newline == std::string_view::npos ? source.size() : newline;
				auto line = source.substr(start, end - start);
				const size_t first = line.find_first_not_of(whitespace);
				if (first != std::string_view::npos)
				{
					line.remove_prefix(first);
					const auto tokenEnd = line.find_first_of(whitespace);
					if (line.substr(0, tokenEnd) == "#type")
					{
						finishStage(start);
						const auto nameStart = line.find_first_not_of(whitespace, tokenEnd);
						if (nameStart == std::string_view::npos)
							throw std::runtime_error("Shader stage name required after #type");
						stage = ParseStage(line.substr(nameStart, line.find_last_not_of(whitespace) - nameStart + 1));
						stageStart = newline == std::string_view::npos ? end : end + 1;
					}
				}
				start = end + 1;
			}

			finishStage(source.size());
			if (stages.empty())
				throw std::runtime_error("Shader file contains no stages");
			return stages;
		}

		auto ToShadercStage(ShaderStage stage) -> shaderc_shader_kind
		{
			switch (stage)
			{
				case ShaderStage::Vertex:
					return shaderc_vertex_shader;
				case ShaderStage::Fragment:
					return shaderc_fragment_shader;
				case ShaderStage::Geometry:
					return shaderc_geometry_shader;
				case ShaderStage::TessellationControl:
					return shaderc_tess_control_shader;
				case ShaderStage::TessellationEvaluation:
					return shaderc_tess_evaluation_shader;
				case ShaderStage::Compute:
					return shaderc_compute_shader;
			}
			throw std::invalid_argument("Unknown shader stage");
		}

		auto CreateOptions(const ShaderCompileOptions& settings) -> shaderc::CompileOptions
		{
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(g_Target, g_TargetVersion);
			switch (settings.Optimization)
			{
				case ShaderOptimization::None:
					options.SetOptimizationLevel(shaderc_optimization_level_zero);
					break;
				case ShaderOptimization::Performance:
					options.SetOptimizationLevel(shaderc_optimization_level_performance);
					break;
				case ShaderOptimization::Size:
					options.SetOptimizationLevel(shaderc_optimization_level_size);
					break;
			}
			for (const auto& [name, value] : settings.Macros)
				options.AddMacroDefinition(name, value);
			if (settings.GenerateDebugInfo)
				options.SetGenerateDebugInfo();
			return options;
		}

		class StageCompiler
		{
		public:
			explicit StageCompiler(const ShaderCompileOptions& options) : m_Options(CreateOptions(options)) {}

			[[nodiscard]] auto Compile(const StageSource& stage, const std::string& name) const -> std::vector<uint32_t>
			{
				const auto result = m_Compiler.CompileGlslToSpv(
				    stage.Text.data(),
				    stage.Text.size(),
				    ToShadercStage(stage.Stage),
				    name.c_str(),
				    m_Options);
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
					throw std::runtime_error(result.GetErrorMessage());
				if (result.GetNumWarnings() != 0)
					Log::Core().warn("{}", result.GetErrorMessage());
				return {result.cbegin(), result.cend()};
			}

		private:
			shaderc::Compiler m_Compiler;
			shaderc::CompileOptions m_Options;
		};

		auto ReadSource(const std::filesystem::path& path) -> std::string
		{
			std::ifstream input(path, std::ios::binary);
			if (!input)
				throw std::runtime_error("Could not open source");
			std::string source{std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
			if (input.bad())
				throw std::runtime_error("Could not read source");
			return source;
		}

		auto CacheKey(std::string_view name, const StageSource& stage, const ShaderCompileOptions& options) -> uint64_t
		{
			// FNV-1a over length-prefixed fields, independent of std::hash and host endianness.
			constexpr uint64_t prime = 1099511628211ULL;
			constexpr uint64_t offsetBasis = 14695981039346656037ULL;
			constexpr uint64_t byteMask = std::numeric_limits<uint8_t>::max();
			constexpr auto bitsPerByte = static_cast<unsigned>(std::numeric_limits<uint8_t>::digits);
			uint64_t key = offsetBasis;
			const auto number = [&](uint64_t value) {
				for (size_t i = 0; i < sizeof(value); ++i)
				{
					key = (key ^ (value & byteMask)) * prime;
					value >>= bitsPerByte;
				}
			};
			const auto text = [&](std::string_view value) {
				number(value.size());
				for (const char byte : value)
					key = (key ^ static_cast<unsigned char>(byte)) * prime;
			};

			// Bump when shaderc or its bundled compiler tooling changes.
			text("rod-shader-v1");
			number(g_Target);
			number(g_TargetVersion);
			text(name);
			number(static_cast<uint8_t>(stage.Stage));
			text(stage.Text);
			number(static_cast<uint8_t>(options.Optimization));
			number(static_cast<uint64_t>(options.GenerateDebugInfo));
			for (const auto& [macro, value] : options.Macros)
			{
				text(macro);
				text(value);
			}
			return key;
		}
	}

	ShaderLoader::ShaderLoader(std::filesystem::path sourceDirectory, std::filesystem::path cacheDirectory)
	    : m_SourceDirectory(std::move(sourceDirectory)), m_Cache(std::move(cacheDirectory))
	{}

	auto ShaderLoader::Load(const std::filesystem::path& path, const ShaderCompileOptions& options) const
	    -> Scope<Shader>
	{
		const auto fullPath = m_SourceDirectory / path;
		try
		{
			const auto source = ReadSource(fullPath);
			const std::string name = fullPath.stem().string();
			std::optional<StageCompiler> compiler;
			std::vector<CompiledShaderStage> stages;
			for (const auto& stage : SplitStages(source))
			{
				const auto key = CacheKey(name, stage, options);
				if (auto cached = m_Cache.Load(key))
				{
					stages.push_back({.Stage = stage.Stage, .Words = std::move(*cached)});
					continue;
				}

				if (!compiler)
					compiler.emplace(options);
				auto words = compiler->Compile(stage, name);
				if (!m_Cache.Store(key, words))
					Log::Core().warn("Could not cache shader '{}'", name);
				stages.push_back({.Stage = stage.Stage, .Words = std::move(words)});
			}

			return CreateScope<OpenGLShader>(name, stages);
		}
		catch (const std::runtime_error& error)
		{
			throw std::runtime_error("Shader load failed '" + fullPath.string() + "': " + error.what());
		}
	}

}
