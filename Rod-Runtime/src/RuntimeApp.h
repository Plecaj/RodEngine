#pragma once

#include "Rod/Scene/Scene.h"

#include <glm/vec4.hpp>

namespace Rod {

	class DesktopHost;
	class Project;
	struct Frame;

	class RuntimeApp
	{
	public:
		RuntimeApp(const Project& project, DesktopHost& host);
		~RuntimeApp();
		RuntimeApp(const RuntimeApp&) = delete;
		auto operator=(const RuntimeApp&) -> RuntimeApp& = delete;
		RuntimeApp(RuntimeApp&&) = delete;
		auto operator=(RuntimeApp&&) -> RuntimeApp& = delete;

		auto Update(const Frame& frame) -> bool;

	private:
		[[nodiscard]] auto LoadScene(const Project& project) -> bool;
		void Resize(glm::uvec2 size);

		static constexpr glm::vec4 s_ClearColor{0.15f, 0.15f, 0.15f, 1.0f};
		DesktopHost& m_Host;
		Scene m_Scene;
		glm::uvec2 m_Size{};
		bool m_Running = false;
	};

}
