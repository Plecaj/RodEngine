#include "rdpch.h"

#include "ProjectSerializer.h"

#include "Project.h"
#include "Rod/Utils/FileUtils.h"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <stdexcept>

namespace Rod {

	void ProjectSerializer::Serialize(const Project& project)
	{
		const auto& manifestPath = project.GetManifestPath();
		const auto& properties = project.GetProperties();
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << properties.Name;
		out << YAML::Key << "AssetsPath" << YAML::Value << properties.AssetsPath.generic_string();
		out << YAML::Key << "StartupScenePath" << YAML::Value << properties.StartupScenePath.generic_string();
		out << YAML::Key << "ScriptsProjectPath" << YAML::Value << properties.ScriptsProjectPath.generic_string();
		out << YAML::EndMap;
		if (!out.good())
			throw std::runtime_error("Project save failed '" + manifestPath.string() + "': " + out.GetLastError());
		WriteTextFile(manifestPath, out.c_str());
	}

	void ProjectSerializer::Deserialize(Project& project)
	{
		const auto& manifestPath = project.GetManifestPath();
		try
		{
			if (manifestPath.empty())
				throw std::runtime_error("Path empty");

			std::ifstream file(manifestPath);
			if (!file.is_open())
				throw std::runtime_error("Open failed; check path/permissions");
			const auto data = YAML::Load(file);
			if (file.bad())
				throw std::runtime_error("Read failed");
			if (!data.IsMap())
				throw std::runtime_error("YAML mapping required");

			for (const auto* key : {"Name", "AssetsPath", "StartupScenePath", "ScriptsProjectPath"})
			{
				const auto field = data[key];
				if (!field.IsDefined())
					throw std::runtime_error("Field '" + std::string(key) + "': missing");
				if (!field.IsScalar())
					throw std::runtime_error("Field '" + std::string(key) + "': text required");
			}

			ProjectProperties properties{
			    .Name = data["Name"].as<std::string>(),
			    .AssetsPath = data["AssetsPath"].as<std::string>(),
			    .StartupScenePath = data["StartupScenePath"].as<std::string>(),
			    .ScriptsProjectPath = data["ScriptsProjectPath"].as<std::string>()};
			project.SetProperties(std::move(properties));
		}
		catch (const std::exception& exception)
		{
			throw std::runtime_error(
			    "Project load failed '" + manifestPath.string() + "': " + exception.what() + " | Properties unchanged.");
		}
	}

}
