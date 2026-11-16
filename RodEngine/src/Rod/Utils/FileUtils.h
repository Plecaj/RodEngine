#pragma once

#include <filesystem>
#include <string_view>

namespace Rod {

	void WriteTextFile(const std::filesystem::path& path, std::string_view text);

}
