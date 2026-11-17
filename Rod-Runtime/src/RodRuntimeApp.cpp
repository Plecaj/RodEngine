#include "rdpch.h"

#include "Rod/Core/EntryPoint.h" // IWYU pragma: keep
#include "Rod/Project/ProjectExporter.h"
#include "Rod/Project/ProjectSerializer.h"
#include "Rod/Utils/PlatformUtils.h"
#include "RuntimeLayer.h"

#include <optional>

namespace Rod {

	class RodRuntime : public Application
	{
	public:
		explicit RodRuntime(Project project) : Application(WindowProps{"Rod Game"}, std::move(project))
		{
			PushLayer(CreateScope<RuntimeLayer>(*GetProject()));
		}
	};

	Scope<Application> CreateApplication(const std::optional<Project>& project)
	{
		const auto directory = Platform::GetExecutablePath().parent_path();
		Project runtimeProject = project ? *project : Project{directory / ProjectExporter::MANIFEST_FILENAME, {}};
		if (!project)
			ProjectSerializer::Deserialize(runtimeProject);
		std::filesystem::current_path(directory);
		return CreateScope<RodRuntime>(std::move(runtimeProject));
	}

}
