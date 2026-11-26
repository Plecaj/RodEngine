#pragma once

#include <glm/glm.hpp>

#include <span>
#include <string>

namespace Rod {

	class Shader
	{
	public:
		Shader() = default;
		virtual ~Shader() = default;
		Shader(const Shader&) = delete;
		auto operator=(const Shader&) -> Shader& = delete;
		Shader(Shader&&) = delete;
		auto operator=(Shader&&) -> Shader& = delete;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, std::span<const int> values) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		[[nodiscard]] virtual auto GetName() const noexcept -> const std::string& = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};

}
