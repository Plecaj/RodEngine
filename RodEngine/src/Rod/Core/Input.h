#pragma once

#include <utility>

namespace Rod {

	class Window;

	class Input
	{
	public:
		explicit Input(const Window& window) noexcept : m_Window(window) {}

		[[nodiscard]] bool IsKeyPressed(int keycode) const;
		[[nodiscard]] bool IsMouseButtonPressed(int button) const;
		[[nodiscard]] std::pair<float, float> GetMousePosition() const;

	private:
		const Window& m_Window;
	};

}
