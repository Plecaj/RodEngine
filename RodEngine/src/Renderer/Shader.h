#pragma once

#include <string>
#include <glm/glm.hpp>

#include "RenderCommand.h"

namespace Rod {

	class Shader
	{
	public:
		static Shader* Create(const std::string& filepath);
		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};

}