#include "rdpch.h"
#include "OpenGLShader.h"
#include "Rod/Renderer/ShaderCacheDatabase.h"

#include <fstream>
#include <array>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>;
#include <shaderc/shaderc.hpp>


namespace Rod {

	namespace Utils {

		static shaderc_shader_kind ShadercTypeFromString(const std::string& type) {
			if (type == "vertex")
				return shaderc_vertex_shader;
			if (type == "fragment" || type == "pixel")
				return shaderc_fragment_shader;
			if (type == "geometry")
				return shaderc_geometry_shader;
			if (type == "tess_control" || type == "hull")
				return shaderc_tess_control_shader;
			if (type == "tess_evaluation" || type == "domain")
				return shaderc_tess_evaluation_shader;
			if (type == "compute")
				return shaderc_compute_shader;

			RD_CORE_ASSERT(false, "Invalid shader type specified");
			return shaderc_glsl_infer_from_source;
		}

		static std::string ShaderExtensionFromType(shaderc_shader_kind type) {
			switch (type) {
				case shaderc_vertex_shader:          return "vert";
				case shaderc_fragment_shader:        return "frag";
				case shaderc_geometry_shader:        return "geom";
				case shaderc_tess_control_shader:    return "tesc";
				case shaderc_tess_evaluation_shader: return "tese";
				case shaderc_compute_shader:         return "comp";
				default:
					RD_CORE_ASSERT(false, "Invalid shaderc_shader_kind");
					return "unknown";
			}
		}

		static GLenum OpenGLShaderKind(shaderc_shader_kind kind)
		{
			switch (kind)
			{
				case shaderc_vertex_shader:          return GL_VERTEX_SHADER;
				case shaderc_fragment_shader:        return GL_FRAGMENT_SHADER;
				case shaderc_geometry_shader:        return GL_GEOMETRY_SHADER;
				case shaderc_tess_control_shader:    return GL_TESS_CONTROL_SHADER;
				case shaderc_tess_evaluation_shader: return GL_TESS_EVALUATION_SHADER;
				case shaderc_compute_shader:         return GL_COMPUTE_SHADER;
				default:
					RD_CORE_ASSERT(false, "Unsupported Shaderc type for OpenGL");
					return 0;
			}
		}

	}

	OpenGLShader::OpenGLShader(std::unordered_map<shaderc_shader_kind, std::string> shaders, std::string name, const ShaderOptions& options )
	{
		RD_PROFILE_FUNCTION();

		shaderc::Compiler compiler;
		shaderc::CompileOptions compileOptions;                 

		compileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		switch (options.OptimizationLevel)
		{
		case Shader::OptimalizationLevel::None: 			compileOptions.SetOptimizationLevel(shaderc_optimization_level_zero); break;
		case Shader::OptimalizationLevel::Performance: 		compileOptions.SetOptimizationLevel(shaderc_optimization_level_performance); break;
		case Shader::OptimalizationLevel::Size: 			compileOptions.SetOptimizationLevel(shaderc_optimization_level_size); break;
		}

		for (auto&& [key, value] : options.Macros)
			compileOptions.AddMacroDefinition(key, value);

		if (options.GenerateDebugInfo)
			compileOptions.SetGenerateDebugInfo();

		for (auto&& [kind, source] : shaders)
		{
			RD_CORE_ASSERT(m_SPIRV.find(kind) == m_SPIRV.end(), "2 Shaders of same kind defined");
			
			size_t hash = std::hash<std::string>{}(source);
			if(ShaderCacheDatabase::Get().IsHashInside(hash))
				if (OpenSpirv(kind, source, hash))
				{
					RD_CORE_TRACE("Loaded shader {}.{} from cache!", m_Name, Utils::ShaderExtensionFromType(kind));
					ShaderCacheDatabase::Get().AddValidHash(hash);
					continue;
				}

			std::vector<uint32_t> res = CompileSpirv(kind, source, compiler);
			if (!ShaderCacheDatabase::Get().CacheShader(hash, res))
				RD_CORE_WARN("Couldnt cache shader {}.{}", m_Name, Utils::ShaderExtensionFromType(kind));
		}
		LoadSpirv();
	}

	OpenGLShader::~OpenGLShader()
	{
		RD_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	bool OpenGLShader::OpenSpirv(const shaderc_shader_kind& kind, const std::string& source, size_t hash)
	{
		std::filesystem::path filename = m_CacheDirectory / std::to_string(hash);
		std::ifstream in(filename, std::ios::binary | std::ios::ate);
		if (!in) 
		{
			RD_CORE_ERROR("Failed to open cached shader file: {}", filename.string());
			return false; 
		}
		std::streamsize size = in.tellg();
		in.seekg(0, std::ios::beg);

		size_t count = static_cast<size_t>((size + 3) / 4);
		std::vector<uint32_t> buffer(count);

		if (size > 0)
			in.read(reinterpret_cast<char*>(buffer.data()), size);

		m_SPIRV[kind] = buffer;

		return true;
	}

	std::vector<uint32_t> OpenGLShader::CompileSpirv(const shaderc_shader_kind& kind, const std::string& source, shaderc::Compiler& compiler)
	{
		shaderc::SpvCompilationResult res = compiler.CompileGlslToSpv(source, kind, m_Name.c_str());
		std::string messages = res.GetErrorMessage();
		if (!messages.empty())
			RD_CORE_WARN("Shader compilation messages:\n{0}", messages);

		if (res.GetCompilationStatus() != shaderc_compilation_status_success)
			RD_CORE_ASSERT(false, "Shader compilation failed");

		std::vector<uint32_t> data(res.cbegin(), res.cend());
		m_SPIRV[kind] = data;

		return data;
	}

	void OpenGLShader::LoadSpirv()
	{
		RD_PROFILE_FUNCTION();

		GLuint program = glCreateProgram();
		std::vector<GLuint> glShaderIDs;

		for (auto&& [kind, spirvBinary] : m_SPIRV)
		{
			GLuint shader = glCreateShader(Utils::OpenGLShaderKind(kind));

			glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvBinary.data(), spirvBinary.size() * sizeof(uint32_t));
			glSpecializeShader(shader, "main", 0, nullptr, nullptr); 

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());

				glDeleteShader(shader);

				RD_CORE_ERROR("{0}", infoLog.data());
				RD_CORE_ASSERT(false, "SPIR-V shader compilation failure!");
				return;
			}

			glAttachShader(program, shader);
			glShaderIDs.push_back(shader);
		}

		m_RendererID = program;
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

			glDeleteProgram(program);
			for (auto id : glShaderIDs)
			{
				glDetachShader(program, id);
				glDeleteShader(id);
			}

			RD_CORE_ERROR("{0}", infoLog.data());
			RD_CORE_ASSERT(false, "SPIR-V shader linking failure!");
			return;
		}

		for (auto id : glShaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
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

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
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
		GLint location = GetUniformLocation(name);
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = GetUniformLocation(name);
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = GetUniformLocation(name);
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& values)
	{
		GLint location = GetUniformLocation(name);
		glUniform2f(location, values.x, values.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& values)
	{
		GLint location = GetUniformLocation(name);
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& values)
	{
		GLint location = GetUniformLocation(name);
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = GetUniformLocation(name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	int OpenGLShader::GetUniformLocation(const std::string& name) const
	{
		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
			return m_UniformLocationCache[name];

		int location = glGetUniformLocation(m_RendererID, name.c_str());
		m_UniformLocationCache[name] = location;
		return location;
	}

}