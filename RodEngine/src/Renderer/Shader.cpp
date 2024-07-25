#include "rdpch.h"
#include "Shader.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <Glad/glad.h>

namespace Rod {

	Shader::API Shader::s_API = Shader::API::OpenGL;

	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch(s_API)
		{
			case Shader::API::OpenGL: return new OpenGLShader(vertexSrc, fragmentSrc);
			case Shader::API::None:  
				return nullptr;
				RD_CORE_ASSERT(false, "None API is not supported");
		}
		return nullptr;
	}

}
