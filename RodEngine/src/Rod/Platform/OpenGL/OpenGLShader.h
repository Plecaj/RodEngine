#pragma once

#include "Rod/Renderer/Shader.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <filesystem>

#include <yaml-cpp/yaml.h>

// TODO: REMOVE
typedef unsigned int GLenum;

namespace Rod {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath, const ShaderOptions& options = {});
		virtual	~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		virtual void SetFloat(const std::string& name, const float value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const std::string& GetName() const override { return m_Name; };

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	private:
		int GetUniformLocation(const std::string& name) const;
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<shaderc_shader_kind, std::string> PreProcess(const std::string& source);
		void CreateYAMLCacheDatabase();
		void ValidateCachedFiles();

		void CompileOrOpenSpirv(const std::unordered_map<shaderc_shader_kind, std::string>& shaderSources, const ShaderOptions& options);
		bool OpenSpirv(const shaderc_shader_kind& kind, const std::string& source);
		std::vector<uint32_t> CompileSpirv(const shaderc_shader_kind& kind, const std::string& source, shaderc::Compiler& compiler);
		void CacheCompiledSpirv(const shaderc_shader_kind& kind, const std::string& source, std::vector<uint32_t>& data);

		void LoadSpirv();
	private:
		uint32_t m_RendererID;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
		std::string m_Name;

		std::unordered_map<shaderc_shader_kind, std::vector<uint32_t>> m_SPIRV;

		std::filesystem::path m_CacheDirectory = "assets/shaders/cached/OpenGL";
		std::filesystem::path m_CacheFile = m_CacheDirectory / "cache.yaml";
		YAML::Node m_CacheData;
	};

}