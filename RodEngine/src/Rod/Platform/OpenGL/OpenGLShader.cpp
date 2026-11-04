#include "rdpch.h"

#include "OpenGLShader.h"

#include "Rod/Renderer/Shader.h"
#include "Rod/Renderer/ShaderCacheDatabase.h"

#include <glad/glad.h>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>

#include <fstream>
#include <ios>
#include <stdexcept>

namespace Rod {

	namespace Utils {

		static std::string ShaderExtensionFromType(shaderc_shader_kind type)
		{
			if (type == shaderc_vertex_shader)
				return "vert";
			if (type == shaderc_fragment_shader)
				return "frag";
			if (type == shaderc_geometry_shader)
				return "geom";
			if (type == shaderc_tess_control_shader)
				return "tesc";
			if (type == shaderc_tess_evaluation_shader)
				return "tese";
			if (type == shaderc_compute_shader)
				return "comp";

			RD_CORE_ASSERT(false, "Invalid shaderc_shader_kind");
			return "unknown";
		}

		static std::string OptimizationName(Shader::OptimizationLevel level)
		{
			switch (level)
			{
				case Shader::OptimizationLevel::None:
					return "none";
				case Shader::OptimizationLevel::Performance:
					return "performance";
				case Shader::OptimizationLevel::Size:
					return "size";
			}

			RD_CORE_ASSERT(false, "Invalid shader optimization level");
			return "unknown";
		}

		static std::string HashToString(size_t hash)
		{
			std::stringstream stream;
			stream << std::hex << hash;
			return stream.str();
		}

		static std::string CreateShaderCacheKey(
		    const std::string& shaderName,
		    shaderc_shader_kind kind,
		    const std::string& source,
		    const Shader::ShaderOptions& options)
		{
			std::string hashInput;
			hashInput += shaderName;
			hashInput += '|';
			hashInput += Utils::ShaderExtensionFromType(kind);
			hashInput += '|';
			hashInput += source;
			hashInput += '|';
			hashInput += OptimizationName(options.OptimizationLevel);
			hashInput += '|';
			hashInput += options.GenerateDebugInfo ? "debug" : "nodebug";

			std::vector<std::pair<std::string, std::string>> macros(options.Macros.begin(), options.Macros.end());
			std::sort(macros.begin(), macros.end());
			for (const auto& [key, value] : macros)
			{
				hashInput += '|';
				hashInput += key;
				hashInput += '=';
				hashInput += value;
			}

			const size_t hash = std::hash<std::string>{}(hashInput);
			return shaderName + "." + Utils::ShaderExtensionFromType(kind) + "." + HashToString(hash);
		}

		static GLenum OpenGLShaderKind(shaderc_shader_kind kind)
		{
			if (kind == shaderc_vertex_shader)
				return GL_VERTEX_SHADER;
			if (kind == shaderc_fragment_shader)
				return GL_FRAGMENT_SHADER;
			if (kind == shaderc_geometry_shader)
				return GL_GEOMETRY_SHADER;
			if (kind == shaderc_tess_control_shader)
				return GL_TESS_CONTROL_SHADER;
			if (kind == shaderc_tess_evaluation_shader)
				return GL_TESS_EVALUATION_SHADER;
			if (kind == shaderc_compute_shader)
				return GL_COMPUTE_SHADER;

			RD_CORE_ASSERT(false, "Unsupported Shaderc type for OpenGL");
			return 0;
		}

		static void DeleteAttachedShaders(GLuint program, const std::vector<GLuint>& shaders)
		{
			for (const GLuint shader : shaders)
			{
				glDetachShader(program, shader);
				glDeleteShader(shader);
			}
		}

	}

	OpenGLShader::OpenGLShader(
	    std::unordered_map<shaderc_shader_kind, std::string> shaders,
	    std::string name,
	    const ShaderOptions& options)
	    : m_Name(std::move(name))
	{
		RD_PROFILE_FUNCTION();

		shaderc::Compiler compiler;
		shaderc::CompileOptions compileOptions;

		compileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		switch (options.OptimizationLevel)
		{
			case Shader::OptimizationLevel::None:
				compileOptions.SetOptimizationLevel(shaderc_optimization_level_zero);
				break;
			case Shader::OptimizationLevel::Performance:
				compileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
				break;
			case Shader::OptimizationLevel::Size:
				compileOptions.SetOptimizationLevel(shaderc_optimization_level_size);
				break;
		}

		for (auto&& [key, value] : options.Macros)
			compileOptions.AddMacroDefinition(key, value);

		if (options.GenerateDebugInfo)
			compileOptions.SetGenerateDebugInfo();

		for (auto&& [kind, source] : shaders)
		{
			RD_CORE_ASSERT(!m_SPIRV.contains(kind), "2 Shaders of same kind defined");

			const std::string cacheKey = Utils::CreateShaderCacheKey(m_Name, kind, source, options);
			if (ShaderCacheDatabase::Get().Contains(cacheKey))
			{
				if (OpenSpirv(kind, cacheKey))
				{
					RD_CORE_TRACE("Loaded shader {}.{} from cache!", m_Name, Utils::ShaderExtensionFromType(kind));
					ShaderCacheDatabase::Get().AddValidKey(cacheKey);
					continue;
				}
			}

			const std::vector<uint32_t> res = CompileSpirv(kind, source, compiler, compileOptions);
			if (!ShaderCacheDatabase::Get().CacheShader(cacheKey, res))
				RD_CORE_WARN("Could not cache shader {}.{}", m_Name, Utils::ShaderExtensionFromType(kind));
		}
		LoadSpirv();
	}

	OpenGLShader::~OpenGLShader()
	{
		RD_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	bool OpenGLShader::OpenSpirv(const shaderc_shader_kind& kind, const std::string& cacheKey)
	{
		const std::filesystem::path filename = m_CacheDirectory / cacheKey;
		std::ifstream in(filename, std::ios::binary | std::ios::ate);
		if (!in)
		{
			RD_CORE_ERROR("Failed to open cached shader file: {}", filename.string());
			return false;
		}
		const std::streamsize size = in.tellg();
		if (size <= 0 || size % static_cast<std::streamsize>(sizeof(uint32_t)) != 0)
		{
			RD_CORE_WARN("Cached shader file is invalid: {}", filename.string());
			return false;
		}

		in.seekg(0, std::ios::beg);

		const size_t count = static_cast<size_t>(size) / sizeof(uint32_t);
		std::vector<uint32_t> buffer(count);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		in.read(reinterpret_cast<char*>(buffer.data()), size);
		if (!in)
		{
			RD_CORE_WARN("Could not read cached shader file: {}", filename.string());
			return false;
		}

		m_SPIRV[kind] = buffer;

		return true;
	}

	std::vector<uint32_t> OpenGLShader::CompileSpirv(
	    const shaderc_shader_kind& kind,
	    const std::string& source,
	    shaderc::Compiler& compiler,
	    const shaderc::CompileOptions& options)
	{
		const shaderc::SpvCompilationResult res = compiler.CompileGlslToSpv(source, kind, m_Name.c_str(), options);
		std::string messages = res.GetErrorMessage();
		if (!messages.empty())
			RD_CORE_WARN("Shader compilation messages:\n{0}", messages);

		if (res.GetCompilationStatus() != shaderc_compilation_status_success)
			throw std::runtime_error("Shader compilation failed: " + messages);

		std::vector<uint32_t> data(res.cbegin(), res.cend());
		m_SPIRV[kind] = data;

		return data;
	}

	void OpenGLShader::LoadSpirv()
	{
		RD_PROFILE_FUNCTION();

		const GLuint program = glCreateProgram();
		std::vector<GLuint> glShaderIDs;
		glShaderIDs.reserve(m_SPIRV.size());

		for (auto&& [kind, spirvBinary] : m_SPIRV)
		{
			const GLuint shader = glCreateShader(Utils::OpenGLShaderKind(kind));

			glShaderBinary(
			    1,
			    &shader,
			    GL_SHADER_BINARY_FORMAT_SPIR_V,
			    spirvBinary.data(),
			    static_cast<GLsizei>(spirvBinary.size() * sizeof(uint32_t)));
			glSpecializeShader(shader, "main", 0, nullptr, nullptr);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (!isCompiled)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(static_cast<size_t>(maxLength));
				glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());

				glDeleteShader(shader);
				Utils::DeleteAttachedShaders(program, glShaderIDs);
				glDeleteProgram(program);

				throw std::runtime_error(
				    "SPIR-V shader compilation failed: " +
				    std::string(infoLog.empty() ? "unknown error" : infoLog.data()));
			}

			glAttachShader(program, shader);
			glShaderIDs.push_back(shader);
		}

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (!isLinked)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(static_cast<size_t>(maxLength));
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

			Utils::DeleteAttachedShaders(program, glShaderIDs);
			glDeleteProgram(program);

			throw std::runtime_error(
			    "SPIR-V shader linking failed: " + std::string(infoLog.empty() ? "unknown error" : infoLog.data()));
		}

		Utils::DeleteAttachedShaders(program, glShaderIDs);
		m_RendererID = program;
	}

	void OpenGLShader::Bind() const
	{
		RD_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		RD_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		RD_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, const int* values, uint32_t count)
	{
		RD_PROFILE_FUNCTION();

		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, const float value)
	{
		RD_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		RD_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		RD_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		RD_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		const GLint location = GetUniformLocation(name);
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, const int* values, uint32_t count)
	{
		const GLint location = GetUniformLocation(name);
		glUniform1iv(location, static_cast<GLsizei>(count), values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		const GLint location = GetUniformLocation(name);
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& values)
	{
		const GLint location = GetUniformLocation(name);
		glUniform2f(location, values.x, values.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& values)
	{
		const GLint location = GetUniformLocation(name);
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& values)
	{
		const GLint location = GetUniformLocation(name);
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		const GLint location = GetUniformLocation(name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		const GLint location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	int OpenGLShader::GetUniformLocation(const std::string& name) const
	{
		if (m_UniformLocationCache.contains(name))
			return m_UniformLocationCache[name];

		const int location = glGetUniformLocation(m_RendererID, name.c_str());
		m_UniformLocationCache[name] = location;
		return location;
	}

}
