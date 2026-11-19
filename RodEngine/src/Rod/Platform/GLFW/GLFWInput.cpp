#include "rdpch.h"

#include "Rod/Core/Input.h"
#include "Rod/Core/Window.h"

#include <GLFW/glfw3.h>

namespace Rod {

	bool Input::IsKeyPressed(int keycode) const
	{
		auto* window = static_cast<GLFWwindow*>(m_Window.GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int button) const
	{
		auto* window = static_cast<GLFWwindow*>(m_Window.GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePosition() const
	{
		auto* window = static_cast<GLFWwindow*>(m_Window.GetNativeWindow());
		double xPos{}, yPos{};
		glfwGetCursorPos(window, &xPos, &yPos);

		return {static_cast<float>(xPos), static_cast<float>(yPos)};
	}

}
