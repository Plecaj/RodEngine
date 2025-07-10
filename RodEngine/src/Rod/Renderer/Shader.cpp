#include "rdpch.h"
#include "Shader.h"

#include "Rod/Platform/OpenGL/OpenGLShader.h"

#include <Glad/glad.h>

namespace Rod {
	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLShader>(filepath);
		case RendererAPI::API::None:
			return nullptr;
			RD_CORE_ASSERT(false, "None API is not supported");
		}
		return nullptr;
	}
	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch(RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
			case RendererAPI::API::None:  
				return nullptr;
				RD_CORE_ASSERT(false, "None API is not supported");
		}
		return nullptr;
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		RD_CORE_ASSERT(!Exists(name), "Shader already exists");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		RD_CORE_ASSERT(Exists(name), "Shader not found");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name)
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

}
