#include "rdpch.h"

#include "Shader.h"

#include "Rod/Platform/OpenGL/OpenGLShader.h"

#include <glad/glad.h>

namespace Rod {

	std::string Shader::Trim(const std::string& value)
	{
		const size_t begin = value.find_first_not_of(" \t\r\n");
		if (begin == std::string::npos)
			return {};

		const size_t end = value.find_last_not_of(" \t\r\n");
		return value.substr(begin, end - begin + 1);
	}

	shaderc_shader_kind Shader::ShadercTypeFromString(const std::string& type)
	{
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

	Scope<Shader> Shader::Create(const std::string& filepath, const ShaderOptions& options)
	{
		const std::string source = Shader::ReadFile(filepath);
		const std::unordered_map<shaderc_shader_kind, std::string> processed = Shader::PreProcess(source);
		RD_CORE_ASSERT(!processed.empty(), "Shader file contains no shader stages");

		const std::filesystem::path file(filepath);
		const std::string name = file.stem().string();
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::OpenGL:
				return CreateScope<OpenGLShader>(processed, name, options);
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
		if (in)
		{
			in.seekg(0, std::ios::end);
			const auto size = static_cast<std::streamsize>(in.tellg());
			if (size <= 0)
				return {};

			result.resize(static_cast<size_t>(size));
			in.seekg(0, std::ios::beg);
			in.read(result.data(), size);
			in.close();
		}
		else
		{
			RD_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		RD_CORE_ASSERT(!result.empty(), "Shader source is empty");
		return result;
	}

	std::unordered_map<shaderc_shader_kind, std::string> Shader::PreProcess(const std::string& source)
	{
		RD_PROFILE_FUNCTION();

		std::unordered_map<shaderc_shader_kind, std::string> shaderSources;

		const char* typeToken = "#type";
		const size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			const size_t eol = source.find_first_of("\r\n", pos);
			RD_CORE_ASSERT(eol != std::string::npos, "Syntax Error");
			const size_t begin = pos + typeTokenLength + 1;
			const std::string type = Trim(source.substr(begin, eol - begin));

			const size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			RD_CORE_ASSERT(nextLinePos != std::string::npos, "Shader stage has no source");
			pos = source.find(typeToken, nextLinePos);

			const size_t shaderEnd = (pos == std::string::npos) ? source.size() : pos;
			shaderSources[ShadercTypeFromString(type)] = source.substr(nextLinePos, shaderEnd - nextLinePos);
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
		Ref<Shader> shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		Ref<Shader> shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		RD_CORE_ASSERT(Exists(name), "Shader not found");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.contains(name);
	}

}
