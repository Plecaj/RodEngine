#pragma once

#include "Rod/Core/Log.h"
#include "Rod/Utils/FunctionPointerCast.h"

#include <GLFW/glfw3.h>
#include <catch2/catch_test_macros.hpp>
#include <glad/glad.h>

#include <filesystem>
#include <memory>
#include <string_view>

namespace Rod::Tests {

	class OpenGLFixture
	{
	public:
		OpenGLFixture()
		{
			if (!Log::GetCoreLogger())
				Log::Init();
			REQUIRE(glfwInit() == GLFW_TRUE);
			glfwDefaultWindowHints();
			constexpr int majorVersion = 4;
			constexpr int minorVersion = 6;
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			m_Window.reset(glfwCreateWindow(1, 1, "Tests", nullptr, nullptr));
			if (!m_Window)
			{
				glfwTerminate();
				FAIL("OpenGL 4.6 required");
			}
			glfwMakeContextCurrent(m_Window.get());
			const int loaded =
			    gladLoadGLLoader([](const char* name) { return FunctionPointerCast<void*>(glfwGetProcAddress(name)); });
			REQUIRE(loaded != 0);
			REQUIRE(GLAD_GL_VERSION_4_6 != 0);
		}

		[[nodiscard]] static auto AssetPath(std::string_view filename) -> std::filesystem::path
		{
			return std::filesystem::path(ROD_TEST_ASSET_DIRECTORY) / filename;
		}

	private:
		static void DestroyWindow(GLFWwindow* window) noexcept
		{
			glfwDestroyWindow(window);
			glfwTerminate();
		}

		std::unique_ptr<GLFWwindow, void (*)(GLFWwindow*)> m_Window{nullptr, DestroyWindow};
	};
}
