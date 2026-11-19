#include "rdpch.h"

#include "Rod/Core/Input.h"
#include "Rod/Core/Window.h"

#include <GLFW/glfw3.h>

namespace Rod {

	auto Input::IsKeyPressed(int keycode) const -> bool
	{
		auto* window = static_cast<GLFWwindow*>(m_Window.GetNativeWindow());
		const auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	auto Input::IsMouseButtonPressed(int button) const -> bool
	{
		auto* window = static_cast<GLFWwindow*>(m_Window.GetNativeWindow());
		const auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	auto Input::GetMousePosition() const -> std::pair<float, float>
	{
		auto* window = static_cast<GLFWwindow*>(m_Window.GetNativeWindow());
		double xPos{}, yPos{};
		glfwGetCursorPos(window, &xPos, &yPos);

		return {static_cast<float>(xPos), static_cast<float>(yPos)};
	}

}
