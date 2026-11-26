#include "rdpch.h"

#include "RuntimeApp.h"

#include "Rod/Core/DesktopHost.h"
#include "Rod/Core/KeyCodes.h"
#include "Rod/Project/Project.h"
#include "Rod/Renderer/RenderDevice.h"
#include "Rod/Renderer/SceneRenderer.h"
#include "Rod/Scene/SceneSerializer.h"
#include "Rod/Scripting/ScriptEngine.h"

#include <exception>

namespace Rod {

	RuntimeApp::RuntimeApp(
	    const Project& project,
	    DesktopHost& host,
	    const RenderDevice& device,
	    SceneRenderer& renderer)
	    : m_Host(host), m_Device(device), m_Renderer(renderer)
	{
		try
		{
			ScriptEngine::Init(project, host.GetInput(), device, ScriptEngine::Mode::Runtime);
			if (!LoadScene(project))
			{
				host.RequestClose();
				return;
			}

			Resize(host.GetWindow().GetFramebufferSize());
			ScriptEngine::OnRuntimeStart(m_Scene);
			m_Running = true;
		}
		catch (...)
		{
			ScriptEngine::Shutdown();
			throw;
		}
	}

	RuntimeApp::~RuntimeApp()
	{
		if (m_Running)
			ScriptEngine::OnRuntimeStop();
		ScriptEngine::Shutdown();
	}

	auto RuntimeApp::LoadScene(const Project& project) -> bool
	{
		const auto path = project.GetStartupScenePath();
		if (!std::filesystem::exists(path))
		{
			Log::Core().error("Runtime startup scene was not found: {}", path.string());
			return false;
		}

		try
		{
			const SceneSerializer serializer(m_Scene, project.GetDirectory());
			serializer.DeserializeText(path.string(), m_Device);
			return true;
		}
		catch (const std::exception& error)
		{
			Log::Core().error("{}", error.what());
			return false;
		}
	}

	auto RuntimeApp::Update(const Frame& frame) -> bool
	{
		const bool escape = std::ranges::any_of(frame.Events, [](const Event& event) {
			const auto* key = std::get_if<KeyChanged>(&event);
			return key && key->Key == Key::Escape && key->Action == KeyAction::Press;
		});
		if (escape)
			m_Host.RequestClose();

		if (escape || !frame.Drawable)
			return false;

		if (frame.Framebuffer != m_Size)
			Resize(frame.Framebuffer);

		m_Device.Clear(s_ClearColor);
		ScriptEngine::OnRuntimeUpdate(frame.Delta);
		m_Renderer.RenderRuntime(m_Scene);
		return true;
	}

	void RuntimeApp::Resize(glm::uvec2 size)
	{
		if (size.x == 0 || size.y == 0)
			return;

		m_Size = size;
		m_Device.SetViewport(0, 0, size.x, size.y);
		m_Scene.ResizeViewport(size.x, size.y);
	}

}
