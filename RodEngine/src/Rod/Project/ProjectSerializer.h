#pragma once

namespace Rod {

	class Project;

	class ProjectSerializer
	{
	public:
		static void Serialize(const Project& project);
		static void Deserialize(Project& project);
	};

}
