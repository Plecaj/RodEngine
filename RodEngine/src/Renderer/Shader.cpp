#include "rdpch.h"
#include "Shader.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <Glad/glad.h>

namespace Rod {

	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch(Shader::GetAPI())
		{
			case RendererAPI::API::OpenGL: return new OpenGLShader(vertexSrc, fragmentSrc);
			case RendererAPI::API::None:  
				return nullptr;
				RD_CORE_ASSERT(false, "None API is not supported");
		}
		return nullptr;
	}

}
