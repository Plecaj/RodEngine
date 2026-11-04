#pragma once

#include "RendererAPI.h"

#include <glm/glm.hpp>
#include <shaderc/shaderc.hpp>

#include <cstdint>
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

		[[nodiscard]] static Scope<Shader>
		Create(const std::string& filepath, const ShaderOptions& options = ShaderOptions());
		Shader() = default;
		virtual ~Shader() = default;
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&&) = delete;
		Shader& operator=(Shader&&) = delete;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, const int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		[[nodiscard]] virtual const std::string& GetName() const noexcept = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

	private:
		static std::string Trim(const std::string& value);
		static shaderc_shader_kind ShadercTypeFromString(const std::string& type);
		static std::string ReadFile(const std::string& filepath);
		static std::unordered_map<shaderc_shader_kind, std::string> PreProcess(const std::string& source);
	};

	inline Shader::ShaderOptions::ShaderOptions() = default;

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);

		[[nodiscard]] Ref<Shader> Get(const std::string& name);

		[[nodiscard]] bool Exists(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};

}
