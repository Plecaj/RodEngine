#include "rdpch.h"

#include "Rod/Core/Core.h"
#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep
#include "RuntimeLayer.h"

#include <optional>

namespace Rod {

	class RodRuntime : public Application
	{
	public:
		explicit RodRuntime(Project project) : Application(WindowProps{"Rod Game"}, std::move(project))
		{
			PushLayer(new RuntimeLayer(*GetProject()));
		}
	};

	Application* CreateApplication(const std::optional<Project>& project)
	{
		RD_CORE_ASSERT(project.has_value(), "Project must be provided for runtime");
		return new RodRuntime(project.value());
	}

}
