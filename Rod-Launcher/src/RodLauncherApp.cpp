#include "rdpch.h"

#include "LauncherApp.h"
#include "Rod/Core/AppMain.h"
#include "Rod/Core/DesktopHost.h"
#include "Rod/ImGui/ImGuiSession.h"
#include "Rod/Renderer/RenderDevice.h"

#include <stdexcept>

namespace Rod {

	namespace {
		constexpr auto g_WindowTitle = "RodEngine - Project Browser";
		constexpr uint32_t g_LauncherWidth = 700;
		constexpr uint32_t g_LauncherHeight = 440;

		auto LauncherWindowProps() -> WindowProps
		{
			WindowProps props{g_WindowTitle, g_LauncherWidth, g_LauncherHeight, "RodAssets/Textures/logo.png"};
			props.Resizable = false;
			props.Centered = true;
			return props;
		}
	}

}

auto main(int argc, char*[]) -> int
{
	return Rod::RunMain([argc] {
		if (argc != 1)
			throw std::invalid_argument("Launcher does not accept a project argument");

		Rod::DesktopHost host(Rod::LauncherWindowProps());
		const Rod::ImGuiSession ui(host.GetWindow());
		const Rod::RenderDevice device;
		Rod::LauncherApp launcher(host, device);
		while (const auto frame = host.NextFrame())
		{
			Rod::ImGuiSession::Begin();
			launcher.Draw();
			Rod::ImGuiSession::End(frame->Drawable);
			if (frame->Drawable)
				host.Present();
		}
	});
}
