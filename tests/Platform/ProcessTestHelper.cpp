#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

int main(int argc, char** argv)
{
	if (argc < 3)
		return 1;
	const std::filesystem::path output = argv[1];
	const std::filesystem::path release = argv[2];
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (!std::filesystem::exists(release))
	{
		if (std::chrono::steady_clock::now() >= deadline)
			return 1;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	const auto temporary = output.string() + ".tmp";
	std::ofstream file(temporary, std::ios::binary);
	for (int index = 3; index < argc; ++index)
		file << argv[index] << '\0';
	file.close();
	if (!file)
		return 1;
	std::filesystem::rename(temporary, output);
	return 0;
}
