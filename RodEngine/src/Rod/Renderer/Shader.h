#pragma once

#include "RendererAPI.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>

namespace Rod {

	class Shader
	{
	public:
		enum class OptimizationLevel : uint8_t
		{
			None,
			Performance,
			Size
		};

		struct ShaderOptions
		{
			std::unordered_map<std::string, std::string> Macros;
			Shader::OptimizationLevel OptimizationLevel = Shader::OptimizationLevel::Performance;
			bool GenerateDebugInfo = false;

			ShaderOptions();
		};

		[[nodiscard]] static auto Create(const std::string& filepath, const ShaderOptions& options = ShaderOptions())
		    -> Scope<Shader>;
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

	inline Shader::ShaderOptions::ShaderOptions() = default;

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		auto Load(const std::string& filepath) -> Ref<Shader>;
		auto Load(const std::string& name, const std::string& filepath) -> Ref<Shader>;

		[[nodiscard]] auto Get(const std::string& name) -> Ref<Shader>;

		[[nodiscard]] auto Exists(const std::string& name) const -> bool;

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};

}
