#pragma once

#include "Rod/Renderer/Shader.h"

#include <glm/glm.hpp>
#include <shaderc/shaderc.hpp>

#include <filesystem>
#include <span>
#include <unordered_map>

namespace Rod {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(
		    std::unordered_map<shaderc_shader_kind, std::string> shaders,
		    std::string name,
		    const ShaderOptions& options = ShaderOptions());
		~OpenGLShader() override;
		OpenGLShader(const OpenGLShader&) = delete;
		auto operator=(const OpenGLShader&) -> OpenGLShader& = delete;
		OpenGLShader(OpenGLShader&&) = delete;
		auto operator=(OpenGLShader&&) -> OpenGLShader& = delete;

		void Bind() const override;
		void Unbind() const override;

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, std::span<const int> values) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;

		[[nodiscard]] auto GetName() const noexcept -> const std::string& override { return m_Name; };

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, std::span<const int> values);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		[[nodiscard]] auto GetUniformLocation(const std::string& name) const -> int;

		[[nodiscard]] auto OpenSpirv(const shaderc_shader_kind& kind, const std::string& cacheKey) -> bool;
		[[nodiscard]] auto CompileSpirv(
		    const shaderc_shader_kind& kind,
		    const std::string& source,
		    shaderc::Compiler& compiler,
		    const shaderc::CompileOptions& options) -> std::vector<uint32_t>;
		void LoadSpirv();

	private:
		uint32_t m_RendererID = 0;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
		std::string m_Name;

		std::unordered_map<shaderc_shader_kind, std::vector<uint32_t>> m_SPIRV;
	};

}
