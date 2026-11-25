#pragma once

#include "Rod/Renderer/Shader.h"
#include "Rod/Renderer/ShaderStage.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>

namespace Rod {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(std::string name, std::span<const CompiledShaderStage> stages);
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

	private:
		[[nodiscard]] auto GetUniformLocation(const std::string& name) const -> int;

		uint32_t m_RendererID = 0;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
		std::string m_Name;
	};

}
