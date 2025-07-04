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
IncludeDir["stb_image"] = "RodEngine/vendor/stb_image"

include "RodEngine/vendor/GLFW"
include "RodEngine/vendor/Glad"
include "RodEngine/vendor/imgui"

project "RodEngine"
	location "RodEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "rdpch.h"
	pchsource "RodEngine/src/rdpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/stb_image.h",
		"%{prj.name}/vendor/stb_image/stb_image.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"RD_PLATFORM_WINDOWS",
			"RD_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "RD_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RD_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "RD_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}"
	}

	links
	{
		"RodEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"RD_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "RD_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RD_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "RD_DIST"
		runtime "Release"
		optimize "on"