#include "rdpch.h"

#include "Rod/Utils/PlatformUtils.h"

// NOLINTBEGIN(misc-include-cleaner)
#include <GLFW/glfw3.h>
#include <Windows.h>

#include <commdlg.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <filesystem>
#include <memory>
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

	std::string FileDialogs::OpenFile(const char* filter, const std::string& initialDirectory)
	{
		CHAR szFile[MAX_PATH] = {0};
		auto ofn = CreateOpenFileName(szFile, sizeof(szFile), filter, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
		ofn.lpstrInitialDir = initialDirectory.empty() ? nullptr : initialDirectory.c_str();

		if (GetOpenFileNameA(&ofn))
			return ofn.lpstrFile;

		return {};
	}

	std::string FileDialogs::SelectFolder(const std::string& initialDirectory)
	{
		struct ComApartment
		{
			HRESULT Result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			~ComApartment()
			{
				if (SUCCEEDED(Result))
					CoUninitialize();
			}
		};
		const ComApartment apartment;
		if (FAILED(apartment.Result) && apartment.Result != RPC_E_CHANGED_MODE)
			return {};

		Microsoft::WRL::ComPtr<IFileOpenDialog> dialog;
		if (FAILED(CoCreateInstance(
		        __uuidof(FileOpenDialog), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(dialog.GetAddressOf()))))
			return {};
		DWORD options = 0;
		if (FAILED(dialog->GetOptions(&options))
		    || FAILED(dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR)))
			return {};

		if (!initialDirectory.empty())
		{
			Microsoft::WRL::ComPtr<IShellItem> folder;
			const auto path = std::filesystem::path(initialDirectory).wstring();
			if (SUCCEEDED(SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(folder.GetAddressOf()))))
				dialog->SetFolder(folder.Get());
		}
		if (FAILED(dialog->Show(GetOwnerWindow())))
			return {};

		Microsoft::WRL::ComPtr<IShellItem> item;
		if (FAILED(dialog->GetResult(item.GetAddressOf())))
			return {};
		PWSTR path = nullptr;
		if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
			return {};
		const std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)> selected(path, &CoTaskMemFree);
		return std::filesystem::path(selected.get()).string();
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
