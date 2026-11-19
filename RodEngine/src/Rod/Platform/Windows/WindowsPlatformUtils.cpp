#include "rdpch.h"

#include "Rod/Core/Window.h"
#include "Rod/Utils/PlatformUtils.h"

// NOLINTBEGIN(misc-include-cleaner)
#include <GLFW/glfw3.h>
#include <Windows.h>

#include <commdlg.h>
#include <filesystem>
#include <memory>
#include <shobjidl.h>
#include <span>
#include <system_error>
#include <wrl/client.h>
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3native.h>

namespace Rod {

	auto Platform::GetExecutablePath() -> std::filesystem::path
	{
		constexpr auto growthFactor = 2U;
		std::wstring path(MAX_PATH, L'\0');
		for (;;)
		{
			const DWORD size = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
			if (!size)
				throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
			if (size < path.size())
			{
				path.resize(size);
				return path;
			}
			path.resize(path.size() * growthFactor);
		}
	}

	static auto GetOwnerWindow(const Window* owner) -> HWND
	{
		return owner ? glfwGetWin32Window(static_cast<GLFWwindow*>(owner->GetNativeWindow())) : nullptr;
	}

	static auto CreateOpenFileName(HWND owner, std::span<char, MAX_PATH> fileBuffer, const char* filter, DWORD flags)
	    -> OPENFILENAMEA
	{
		OPENFILENAMEA ofn = {};
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = owner;
		ofn.lpstrFile = fileBuffer.data();
		ofn.nMaxFile = static_cast<DWORD>(fileBuffer.size());
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = flags | OFN_NOCHANGEDIR;

		return ofn;
	}

	auto FileDialogs::OpenFile(const char* filter, const std::string& initialDirectory) const -> std::string
	{
		CHAR szFile[MAX_PATH] = {0};
		auto ofn = CreateOpenFileName(GetOwnerWindow(m_Owner), szFile, filter, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
		ofn.lpstrInitialDir = initialDirectory.empty() ? nullptr : initialDirectory.c_str();

		if (GetOpenFileNameA(&ofn))
			return ofn.lpstrFile;

		return {};
	}

	auto FileDialogs::SelectFolder(const std::string& initialDirectory) const -> std::string
	{
		struct ComApartment
		{
			ComApartment() noexcept = default;
			ComApartment(const ComApartment&) = delete;
			auto operator=(const ComApartment&) -> ComApartment& = delete;
			ComApartment(ComApartment&&) = delete;
			auto operator=(ComApartment&&) -> ComApartment& = delete;

			const HRESULT Result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
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
		        __uuidof(FileOpenDialog),
		        nullptr,
		        CLSCTX_INPROC_SERVER,
		        IID_PPV_ARGS(dialog.GetAddressOf()))))
			return {};
		DWORD options = 0;
		if (FAILED(dialog->GetOptions(&options)) ||
		    FAILED(dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR)))
			return {};

		if (!initialDirectory.empty())
		{
			Microsoft::WRL::ComPtr<IShellItem> folder;
			const auto path = std::filesystem::path(initialDirectory).wstring();
			if (SUCCEEDED(SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(folder.GetAddressOf()))))
				dialog->SetFolder(folder.Get());
		}
		if (FAILED(dialog->Show(GetOwnerWindow(m_Owner))))
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

	auto FileDialogs::SaveFile(const char* filter) const -> std::string
	{
		CHAR szFile[MAX_PATH] = {0};
		auto ofn = CreateOpenFileName(GetOwnerWindow(m_Owner), szFile, filter, OFN_PATHMUSTEXIST);

		if (GetSaveFileNameA(&ofn))
			return ofn.lpstrFile;

		return {};
	}

}
// NOLINTEND(misc-include-cleaner)
