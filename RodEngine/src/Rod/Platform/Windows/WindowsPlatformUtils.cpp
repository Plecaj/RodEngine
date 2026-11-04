#include "rdpch.h"

#include "Rod/Utils/PlatformUtils.h"

// NOLINTBEGIN(misc-include-cleaner)
#include <GLFW/glfw3.h>
#include <Windows.h>

#include <commdlg.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include "Rod/Core/Application.h"

#include <GLFW/glfw3native.h>

namespace Rod {

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
		CHAR szFile[MAX_PATH] = {0};
		auto ofn = CreateOpenFileName(szFile, sizeof(szFile), filter, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

		if (GetOpenFileNameA(&ofn))
			return ofn.lpstrFile;

		return {};
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		CHAR szFile[MAX_PATH] = {0};
		auto ofn = CreateOpenFileName(szFile, sizeof(szFile), filter, OFN_PATHMUSTEXIST);

		if (GetSaveFileNameA(&ofn))
			return ofn.lpstrFile;

		return {};
	}

}
// NOLINTEND(misc-include-cleaner)
