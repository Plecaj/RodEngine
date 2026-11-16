#pragma once

#include <utility>

namespace Rod {

	class Input
	{
	public:
		[[nodiscard]] static bool IsKeyPressed(int keycode);

		[[nodiscard]] static bool IsMouseButtonPressed(int button);
		[[nodiscard]] static std::pair<float, float> GetMousePosition();
		[[nodiscard]] static float GetMouseX();
		[[nodiscard]] static float GetMouseY();
	};

}
