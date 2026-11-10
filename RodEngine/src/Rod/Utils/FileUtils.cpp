#include "rdpch.h"

#include "FileUtils.h"

#include "Rod/Core/UUID.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace Rod {

	void WriteTextFile(const std::filesystem::path& path, std::string_view text)
	{
		if (path.empty() || path.filename().empty())
			throw std::runtime_error("Filename required | File unchanged.");

		const auto temporaryPath = path.parent_path() / (".rod-save-" + std::to_string(static_cast<uint64_t>(UUID{})));
		try
		{
			if (path.has_parent_path())
				std::filesystem::create_directories(path.parent_path());
			std::ofstream file;
			file.exceptions(std::ios::failbit | std::ios::badbit);
			file.open(temporaryPath);
			file << text;
			file.close();
			std::filesystem::rename(temporaryPath, path);
		}
		catch (const std::exception& exception)
		{
			std::error_code error;
			std::filesystem::remove(temporaryPath, error);
			std::string message = "File write failed '" + path.string() + "': " + exception.what() + " | File unchanged.";
			if (error)
				message += " | Cleanup failed '" + temporaryPath.string() + "': " + error.message();
			throw std::runtime_error(message);
		}
		catch (...)
		{
			std::error_code error;
			std::filesystem::remove(temporaryPath, error);
			throw;
		}
	}

}
