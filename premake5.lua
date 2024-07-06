workspace "RodEngine"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "RodEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "RodEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "RodEngine/vendor/imgui"
IncludeDir["glm"] = "RodEngine/vendor/glm"

include "RodEngine/vendor/GLFW"
include "RodEngine/vendor/Glad"
include "RodEngine/vendor/imgui"

project "RodEngine"
	location "RodEngine"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "rdpch.h"
	pchsource "RodEngine/src/rdpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
		defines
		{
			"RD_PLATFORM_WINDOWS",
			"RD_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
		}

	filter "configurations:Debug"
		defines "RD_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "RD_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "RD_DIST"
		runtime "Release"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"RodEngine/vendor/spdlog/include",
		"RodEngine/src",
		"%{IncludeDir.glm}"
	}

	links
	{
		"RodEngine"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"RD_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "RD_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "RD_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "RD_DIST"
		runtime "Release"
		optimize "On"