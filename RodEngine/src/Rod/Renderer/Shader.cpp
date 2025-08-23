#include "rdpch.h"
#include "Shader.h"

#include "Rod/Platform/OpenGL/OpenGLShader.h"

#include <Glad/glad.h>

namespace Rod {

	namespace Utils {

		static shaderc_shader_kind ShadercTypeFromString(const std::string& type) {
			if (type == "vertex")
				return shaderc_vertex_shader;
			if (type == "fragment" || type == "pixel")
				return shaderc_fragment_shader;
			if (type == "geometry")
				return shaderc_geometry_shader;
			if (type == "tess_control" || type == "hull")
				return shaderc_tess_control_shader;
			if (type == "tess_evaluation" || type == "domain")
				return shaderc_tess_evaluation_shader;
			if (type == "compute")
				return shaderc_compute_shader;

			RD_CORE_ASSERT(false, "Invalid shader type specified");
			return shaderc_glsl_infer_from_source;
		}
	}

    Ref<Shader> Shader::Create(const std::string& filepath, const ShaderOptions& options)
    {
       std::string source = Shader::ReadFile(filepath); 
	   std::unordered_map<shaderc_shader_kind, std::string> processed = Shader::PreProcess(source);

	   std::filesystem::path file(filepath);
	   std::string name = file.stem().string();

       switch (RendererAPI::GetAPI())
       {
       case RendererAPI::API::OpenGL: return CreateRef<OpenGLShader>(processed, name, options);
       case RendererAPI::API::None:
           RD_CORE_ASSERT(false, "None API is not supported");
           return nullptr;
       }
       return nullptr;
    }

	std::string Shader::ReadFile(const std::string& filepath)
	{
		RD_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else {
			RD_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<shaderc_shader_kind, std::string> Shader::PreProcess(const std::string& source)
	{
		RD_PROFILE_FUNCTION();

		std::unordered_map<shaderc_shader_kind, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			RD_CORE_ASSERT(eol != std::string::npos, "Syntax Error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			size_t shaderEnd = (pos == std::string::npos) ? source.size() : pos;
			shaderSources[Utils::ShadercTypeFromString(type)] =
				source.substr(nextLinePos, shaderEnd - nextLinePos);
		}

		return shaderSources;
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
