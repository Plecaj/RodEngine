#pragma once

namespace Rod {

	class Window;

	class ImGuiSession
	{
	public:
		explicit ImGuiSession(const Window& window);
		~ImGuiSession();
		ImGuiSession(const ImGuiSession&) = delete;
		auto operator=(const ImGuiSession&) -> ImGuiSession& = delete;
		ImGuiSession(ImGuiSession&&) = delete;
		auto operator=(ImGuiSession&&) -> ImGuiSession& = delete;

		static void Begin();
		static void End(bool drawable);

	private:
		static void SetDarkThemeColors();
	};

}
