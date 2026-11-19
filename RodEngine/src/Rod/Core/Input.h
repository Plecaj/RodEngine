#pragma once

#include <utility>

namespace Rod {

	class Window;

	class Input
	{
	public:
		explicit Input(const Window& window) noexcept : m_Window(window) {}

		[[nodiscard]] auto IsKeyPressed(int keycode) const -> bool;
		[[nodiscard]] auto IsMouseButtonPressed(int button) const -> bool;
		[[nodiscard]] auto GetMousePosition() const -> std::pair<float, float>;

	private:
		const Window& m_Window;
	};

}
