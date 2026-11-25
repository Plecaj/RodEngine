#include "rdpch.h"

#include "OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <utility>

namespace Rod {
	namespace {
		auto ToOpenGLStage(ShaderStage stage) -> GLenum
		{
			switch (stage)
			{
				case ShaderStage::Vertex:
					return GL_VERTEX_SHADER;
				case ShaderStage::Fragment:
					return GL_FRAGMENT_SHADER;
				case ShaderStage::Geometry:
					return GL_GEOMETRY_SHADER;
				case ShaderStage::TessellationControl:
					return GL_TESS_CONTROL_SHADER;
				case ShaderStage::TessellationEvaluation:
					return GL_TESS_EVALUATION_SHADER;
				case ShaderStage::Compute:
					return GL_COMPUTE_SHADER;
			}
			throw std::invalid_argument("Unknown shader stage");
		}

		void DeleteAttachedShaders(GLuint program, std::span<const GLuint> shaders)
		{
			for (const GLuint shader : shaders)
			{
				glDetachShader(program, shader);
				glDeleteShader(shader);
			}
		}
	}

	OpenGLShader::OpenGLShader(std::string name, std::span<const CompiledShaderStage> stages) : m_Name(std::move(name))
	{
		const GLuint program = glCreateProgram();
		std::vector<GLuint> glShaderIDs;
		glShaderIDs.reserve(stages.size());

		for (const auto& [stage, words] : stages)
		{
			const GLuint shader = glCreateShader(ToOpenGLStage(stage));

			glShaderBinary(
			    1,
			    &shader,
			    GL_SHADER_BINARY_FORMAT_SPIR_V,
			    words.data(),
			    static_cast<GLsizei>(words.size() * sizeof(uint32_t)));
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
				DeleteAttachedShaders(program, glShaderIDs);
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

			DeleteAttachedShaders(program, glShaderIDs);
			glDeleteProgram(program);

			throw std::runtime_error(
			    "SPIR-V shader linking failed: " + std::string(infoLog.empty() ? "unknown error" : infoLog.data()));
		}

		DeleteAttachedShaders(program, glShaderIDs);
		m_RendererID = program;
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		glProgramUniform1i(m_RendererID, GetUniformLocation(name), value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, std::span<const int> values)
	{
		RD_CORE_ASSERT(std::in_range<GLsizei>(values.size()), "Uniform integer array exceeds the OpenGL count limit");
		glProgramUniform1iv(m_RendererID, GetUniformLocation(name), static_cast<GLsizei>(values.size()), values.data());
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		glProgramUniform1f(m_RendererID, GetUniformLocation(name), value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		glProgramUniform3f(m_RendererID, GetUniformLocation(name), value.x, value.y, value.z);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		glProgramUniform4f(m_RendererID, GetUniformLocation(name), value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		glProgramUniformMatrix4fv(m_RendererID, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	auto OpenGLShader::GetUniformLocation(const std::string& name) const -> int
	{
		const auto found = m_UniformLocationCache.find(name);
		if (found != m_UniformLocationCache.end())
			return found->second;

		const int location = glGetUniformLocation(m_RendererID, name.c_str());
		m_UniformLocationCache.emplace(name, location);
		return location;
	}

}
