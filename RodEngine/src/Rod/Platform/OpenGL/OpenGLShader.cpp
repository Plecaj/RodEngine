#include "rdpch.h"
#include "OpenGLShader.h"

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

			RD_CORE_ASSERT(false, "Invalid shader type specified: {0}", type);
			return shaderc_glsl_infer_from_source;
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


	OpenGLShader::OpenGLShader(const std::string& filepath, const ShaderOptions& options)
	{
		RD_PROFILE_FUNCTION();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);

		// Extracting name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);

		CompileSpirv(shaderSources, options);
		LoadSpirv();
	}

	OpenGLShader::~OpenGLShader()
	{
		RD_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}


	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		RD_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else {
			RD_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<shaderc_shader_kind, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		RD_PROFILE_FUNCTION();

		std::unordered_map<shaderc_shader_kind, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			RD_CORE_ASSERT(eol != std::string::npos, "Syntax Error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			size_t shaderEnd = (pos == std::string::npos) ? source.size() : pos;
			shaderSources[Utils::ShadercTypeFromString(type)] =
				source.substr(nextLinePos, shaderEnd - nextLinePos);
		}

		return shaderSources;
	}

	void OpenGLShader::CompileSpirv(const std::unordered_map<shaderc_shader_kind, std::string>& shaderSources, const ShaderOptions& options)
	{
		RD_PROFILE_FUNCTION();

		shaderc::Compiler compiler;
		shaderc::CompileOptions compileOptions;

		compileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		switch (options.OptimizationLevel)
		{
			case Shader::OptimalizationLevel::None: 			compileOptions.SetOptimizationLevel(shaderc_optimization_level_zero);
			case Shader::OptimalizationLevel::Performance: 		compileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
			case Shader::OptimalizationLevel::Size: 			compileOptions.SetOptimizationLevel(shaderc_optimization_level_size);
		}

		for (auto&& [key, value] : options.Macros)
			compileOptions.AddMacroDefinition(key, value);

		if (options.GenerateDebugInfo)
			compileOptions.SetGenerateDebugInfo();


		for (auto&& [kind, source] : shaderSources)
		{
			RD_CORE_ASSERT(m_SPIRV.find(kind) == m_SPIRV.end(), "2 Shaders of same kind defined");

			shaderc::SpvCompilationResult res = compiler.CompileGlslToSpv(source, kind, m_Name.c_str());
			std::string messages = res.GetErrorMessage();
			if (!messages.empty())
				RD_CORE_WARN("Shader compilation messages:\n{0}", messages);

			if (res.GetCompilationStatus() != shaderc_compilation_status_success)
				RD_CORE_ASSERT(false, "Shader compilation failed for {0}", m_Name);

			m_SPIRV[kind] = { res.cbegin(), res.cend() };
		}
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