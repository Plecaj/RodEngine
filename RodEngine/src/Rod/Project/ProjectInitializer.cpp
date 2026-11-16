#include "rdpch.h"

#include "ProjectInitializer.h"

#include "Rod/Scene/Components.h"
#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Scene/SceneSerializer.h"
#include "Rod/Utils/FileUtils.h"

#include <stdexcept>
#include <string_view>

namespace Rod {

	namespace {
		void RequireProjectPath(const std::filesystem::path& path, const std::filesystem::path& root)
		{
			const auto relative = std::filesystem::weakly_canonical(path).lexically_relative(root);
			if (relative.empty() || relative.has_root_path() || std::ranges::find(relative, "..") != relative.end())
				throw std::runtime_error("Choose a path inside the project folder.");
		}

		void ValidateFile(const std::filesystem::path& path, const char* extension, bool isDefault)
		{
			if (path.extension() != extension)
				throw std::runtime_error(std::string("Choose a ") + extension + " file.");
			if (!std::filesystem::is_regular_file(path) && (!isDefault || std::filesystem::exists(path)))
				throw std::runtime_error("A selected file no longer exists or is not a regular file. Select it again.");
		}

		std::string EscapeXml(std::string_view value)
		{
			std::string result;
			for (const char character : value)
			{
				switch (character)
				{
					case '&':
						result += "&amp;";
						break;
					case '<':
						result += "&lt;";
						break;
					case '>':
						result += "&gt;";
						break;
					case '\"':
						result += "&quot;";
						break;
					case '\'':
						result += "&apos;";
						break;
					default:
						result += character;
						break;
				}
			}
			return result;
		}
	}

	void ProjectInitializer::CreateDefaultFiles(const Project& project)
	{
		const auto root = std::filesystem::canonical(project.GetDirectory());
		const auto assets = project.GetAssetsPath();
		const auto scene = project.GetStartupScenePath();
		const auto scripts = project.GetScriptProjectPath();
		for (const auto& path : {assets, scene, scripts})
		{
			if (path.empty())
				throw std::runtime_error("Project paths must not be empty.");
			RequireProjectPath(path, root);
		}
		if (std::filesystem::exists(assets) && !std::filesystem::is_directory(assets))
			throw std::runtime_error("Choose an asset folder.");
		ValidateFile(scene, ".rod", scene == assets / "Scenes/Main.rod");
		ValidateFile(scripts, ".csproj", scripts == assets / "Scripts/Game.csproj");

		std::filesystem::create_directories(assets);
		if (!std::filesystem::exists(scene))
		{
			Scene starter;
			auto camera = starter.CreateEntity("Camera");
			camera.AddComponent<CameraComponent>();
			camera.GetComponent<TransformComponent>().Translation.z = 8.0f;
			SceneSerializer serializer(starter, root);
			serializer.SerializeText(scene.string());
		}
		if (!std::filesystem::exists(scripts))
		{
			const auto core = std::filesystem::absolute("RodAssets/Scripts/Core/Rod.ScriptCore.dll");
			const std::string contents =
			    "<Project Sdk=\"Microsoft.NET.Sdk\">\n"
			    "  <PropertyGroup>\n"
			    "    <TargetFramework>net9.0</TargetFramework>\n"
			    "    <Nullable>enable</Nullable>\n"
			    "    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>\n"
			    "    <ImplicitUsings>enable</ImplicitUsings>\n"
			    "  </PropertyGroup>\n"
			    "  <ItemGroup>\n"
			    "    <Reference Include=\"Rod.ScriptCore\">\n"
			    "      <HintPath>" + EscapeXml(core.generic_string()) + "</HintPath>\n"
			    "    </Reference>\n"
			    "  </ItemGroup>\n"
			    "</Project>\n";
			WriteTextFile(scripts, contents);
			std::filesystem::create_directories(scripts.parent_path() / "Source");
		}
	}

}
