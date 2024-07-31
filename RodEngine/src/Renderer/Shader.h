#pragma once

#include <string>
#include <glm/glm.hpp>

#include "RenderCommand.h"

namespace Rod {

	class Shader
	{
	public:
		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};

}