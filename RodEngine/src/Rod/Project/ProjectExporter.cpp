#include "rdpch.h"

#include "ProjectExporter.h"

#include "Project.h"
#include "ProjectSerializer.h"
#include "Rod/Core/UUID.h"
#include "Rod/Scene/SceneSerializer.h"
#include "Rod/Scripting/ScriptBuilder.h"

#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h> // IWYU pragma: keep

#include <span>
#include <stdexcept>
#include <system_error>

namespace Rod {

	namespace {
		bool IsWithin(const std::filesystem::path& path, const std::filesystem::path& directory)
		{
			const auto relative = path.lexically_relative(directory);
			return !relative.empty() && *relative.begin() != "..";
		}

		std::filesystem::path ProjectRelative(const std::filesystem::path& path, const std::filesystem::path& root)
		{
			const auto resolved = std::filesystem::weakly_canonical(path);
			if (!IsWithin(resolved, root) || resolved == root)
				throw std::runtime_error("Export paths must stay inside the project: " + path.string());
			return resolved.lexically_relative(root);
		}

		void RequireFile(const std::filesystem::path& path)
		{
			if (!std::filesystem::is_regular_file(path))
				throw std::runtime_error("Export file is missing: " + path.string());
		}

		void CopyTree(
		    const std::filesystem::path& source,
		    const std::filesystem::path& destination,
		    std::span<const std::filesystem::path> excludedDirectories = {})
		{
			std::filesystem::create_directories(destination);
			for (auto iterator = std::filesystem::recursive_directory_iterator(source);
			     iterator != std::filesystem::recursive_directory_iterator();
			     ++iterator)
			{
				const auto& entry = *iterator;
				if (entry.is_symlink())
					throw std::runtime_error(
					    "Export requires files instead of symbolic links: " + entry.path().string());
				const auto target = destination / entry.path().lexically_relative(source);
				if (entry.is_directory())
				{
					if (std::ranges::find(excludedDirectories, entry.path()) != excludedDirectories.end())
					{
						iterator.disable_recursion_pending();
						continue;
					}
					std::filesystem::create_directories(target);
				}
				else if (entry.path().extension() != ".csproj" && entry.path().extension() != ".cs")
					std::filesystem::copy_file(entry.path(), target);
			}
		}

		void CopyRuntime(const std::filesystem::path& executable, const std::filesystem::path& destination)
		{
			std::filesystem::copy_file(executable, destination / ("RodGame" + executable.extension().string()));
			for (const auto& entry : std::filesystem::directory_iterator(executable.parent_path()))
			{
				const auto name = entry.path().filename().string();
				if (entry.is_regular_file() &&
				    (entry.path().extension() == ".dll" || name.ends_with(".so") || name.contains(".so.")))
					std::filesystem::copy_file(entry.path(), destination / entry.path().filename());
			}
		}

		void ValidateSceneAssets(const std::filesystem::path& scenePath, const std::filesystem::path& root)
		{
			const auto scene = YAML::LoadFile(scenePath.string());
			const auto entities = scene["Entities"];
			if (!scene["Scene"].IsScalar() || !entities.IsSequence())
				throw std::runtime_error("Export scene is invalid: " + scenePath.string());
			for (const auto& entity : entities)
			{
				for (const auto& [component, key] :
				     {std::pair{"SpriteRendererComponent", "Texture"}, std::pair{"MeshComponent", "Path"}})
				{
					const auto data = entity[component];
					if (!data)
						continue;
					const auto value = data[key].as<std::string>();
					if (value != "None")
						RequireFile(root / ProjectRelative(root / value, root));
				}
			}
		}

		void ValidatePackage(const Project& package)
		{
			const auto root = package.GetDirectory();
			const auto core = root / "RodAssets/Scripts/Core";
			const auto shaders = package.GetShadersPath();
			for (const auto& file :
			     {core / "Rod.ScriptCore.dll",
			      core / "Rod.ScriptCore.runtimeconfig.json",
			      shaders / "Texture.glsl",
			      shaders / "3D.glsl",
			      shaders / "Shadow.glsl",
			      ScriptBuilder::GetAssemblyPath(package.GetScriptProjectPath())})
				RequireFile(file);
#ifdef RD_PLATFORM_WINDOWS
			RequireFile(root / "nethost.dll");
#endif
			for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
			{
				if (entry.path().extension() == ".rod")
					ValidateSceneAssets(entry.path(), root);
			}
		}

		void PublishPackage(const std::filesystem::path& staging, const std::filesystem::path& destination)
		{
			auto backup = staging;
			backup += ".previous";
			const bool replace = std::filesystem::exists(destination);
			if (replace)
				std::filesystem::rename(destination, backup);
			try
			{
				std::filesystem::rename(staging, destination);
			}
			catch (...)
			{
				if (replace)
					std::filesystem::rename(backup, destination);
				throw;
			}
			std::error_code error;
			std::filesystem::remove_all(backup, error);
		}
	}

	void ProjectExporter::Export(
	    const Project& project,
	    const std::filesystem::path& runtimeExecutable,
	    const std::filesystem::path& outputDirectory,
	    Scene* startupScene)
	{
		if (outputDirectory.empty())
			throw std::invalid_argument("Export directory is required");
		const auto root = std::filesystem::canonical(project.GetDirectory());
		const auto runtime = std::filesystem::weakly_canonical(runtimeExecutable);
		const auto output = std::filesystem::weakly_canonical(outputDirectory);
		const auto shaders = std::filesystem::weakly_canonical(project.GetShadersPath());
		const auto assets = ProjectRelative(project.GetAssetsPath(), root);
		const auto scripts = ProjectRelative(project.GetScriptProjectPath(), root);
		const auto assemblyDirectory = ScriptBuilder::GetAssemblyPath(scripts).parent_path();
		const auto core = runtime.parent_path() / "RodAssets/Scripts/Core";
		for (const auto& source : {root / assets, shaders, core, root / assemblyDirectory})
		{
			if (IsWithin(source, output) || IsWithin(output, source))
				throw std::runtime_error("Export destination overlaps a source directory: " + output.string());
		}
		if (IsWithin(root, output) || IsWithin(runtime, output))
			throw std::runtime_error("Export destination contains the project directory: " + output.string());
		if (std::filesystem::exists(output) && !std::filesystem::is_regular_file(output / MANIFEST_FILENAME))
			throw std::runtime_error("Export destination already exists and is not a game package: " + output.string());

		auto staging = output;
		staging += ".tmp-" + std::to_string(static_cast<uint64_t>(UUID{}));
		auto properties = project.GetProperties();
		properties.AssetsPath = assets;
		properties.ScriptsProjectPath = scripts;
		properties.StartupScenePath = assets / "Scenes/Startup.rod";
		properties.ShadersPath = "RodAssets/Shaders";
		const Project packaged{staging / MANIFEST_FILENAME, std::move(properties)};

		try
		{
			std::filesystem::create_directories(staging);
			CopyRuntime(runtime, staging);
			const auto scriptDirectory = (root / scripts).parent_path();
			const std::array excluded{
			    scriptDirectory / "Source",
			    scriptDirectory / "bin",
			    scriptDirectory / "obj",
			    scriptDirectory / "Cache",
			    root / assets / "Scripts/Cache"};
			CopyTree(root / assets, staging / assets, excluded);
			CopyTree(core, staging / "RodAssets/Scripts/Core");
			CopyTree(shaders, staging / "RodAssets/Shaders", std::array{shaders / "Cached"});
			CopyTree(root / assemblyDirectory, staging / assemblyDirectory);

			std::filesystem::create_directories(packaged.GetStartupScenePath().parent_path());
			if (startupScene)
				SceneSerializer(*startupScene, root).SerializeText(packaged.GetStartupScenePath().string());
			else
				std::filesystem::copy_file(
				    project.GetStartupScenePath(),
				    packaged.GetStartupScenePath(),
				    std::filesystem::copy_options::overwrite_existing);
			ProjectSerializer::Serialize(packaged);
			ValidatePackage(packaged);
			PublishPackage(staging, output);
		}
		catch (...)
		{
			std::error_code error;
			std::filesystem::remove_all(staging, error);
			throw;
		}
	}

}
