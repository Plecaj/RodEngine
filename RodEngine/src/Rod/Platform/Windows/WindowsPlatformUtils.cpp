#include "rdpch.h"
#include "Rod/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Rod/Core/Application.h"

namespace Rod {

	float Platform::GetTime()
	{
		return (float)glfwGetTime();
	}

	static HWND GetOwnerWindow()
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		return glfwGetWin32Window(window);
	}

	static OPENFILENAMEA CreateOpenFileName(char* fileBuffer, DWORD bufferSize, const char* filter, DWORD flags)
	{
		OPENFILENAMEA ofn = {};
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = GetOwnerWindow();
		ofn.lpstrFile = fileBuffer;
		ofn.nMaxFile = bufferSize;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = flags | OFN_NOCHANGEDIR;

		return ofn;
	}

	std::string FileDialogs::OpenFile(const char* filter)
	{
		CHAR szFile[260] = { 0 };
		auto ofn = CreateOpenFileName(szFile, sizeof(szFile), filter, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return {};
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		CHAR szFile[260] = { 0 };
		auto ofn = CreateOpenFileName(szFile, sizeof(szFile), filter, OFN_PATHMUSTEXIST);

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return {};
	}

}
