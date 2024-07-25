#pragma once

#include <string>

namespace Rod {

	class Shader
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static API GetAPI() { return s_API; };
	private:
		static API s_API;
	};


}