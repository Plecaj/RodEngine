#pragma once

#include <string>
#include <glm/glm.hpp>

#include <unordered_map>

#include "RenderCommand.h"

#include <shaderc/shaderc.hpp>

namespace Rod {

	class Shader
	{
	public:
		enum class OptimalizationLevel
		{
			None,
			Performance,
			Size
		};

		struct ShaderOptions
		{
			std::unordered_map<std::string, std::string> Macros; // #define KEY VALUE
			OptimalizationLevel OptimizationLevel = OptimalizationLevel::Performance;
			bool GenerateDebugInfo = false;
		};

		static Ref<Shader> Create(const std::string& filepath, const ShaderOptions& options = {});
		virtual ~Shader() = default;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, const float value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual const std::string& GetName() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};

	class ShaderLibrary {
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};

}