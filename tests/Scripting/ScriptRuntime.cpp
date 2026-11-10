#include "rdpch.h"

#include "Rod/Scripting/ScriptAssembly.h"
#include "Rod/Scripting/ScriptRuntime.h"

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#ifdef RD_PLATFORM_WINDOWS
	#include <objbase.h>
#endif

#ifdef ROD_TEST_CORE_ASSEMBLY_PATH
TEST_CASE("Failed managed assembly loads preserve the active assembly", "[scripting][loading]")
{
	const std::filesystem::path corePath = ROD_TEST_CORE_ASSEMBLY_PATH;
	const auto configPath = corePath.parent_path() / "Rod.ScriptCore.runtimeconfig.json";
	const std::string hostType = "Rod.Internal.ScriptHost, Rod.ScriptCore";
	{
		Rod::ScriptRuntime initial(configPath, corePath);
		REQUIRE(initial.GetFunction(hostType, "Shutdown") != nullptr);
	}
	Rod::ScriptRuntime runtime(configPath, corePath);
	using LoadFn = void*(RD_CDECL*)(const char*);
	using CreateFn = int32_t(RD_CDECL*)(uint64_t, const char*);
	using ShutdownFn = void(RD_CDECL*)();
	const auto load = std::bit_cast<LoadFn>(runtime.GetFunction(hostType, "LoadAssembly"));
	const auto create = std::bit_cast<CreateFn>(runtime.GetFunction(hostType, "CreateScript"));
	const auto shutdown = std::bit_cast<ShutdownFn>(runtime.GetFunction(hostType, "Shutdown"));
	REQUIRE(load != nullptr);
	REQUIRE(create != nullptr);
	REQUIRE(shutdown != nullptr);
	auto stop = [shutdown](void*) { shutdown(); };
	std::unique_ptr<void, decltype(stop)> hostGuard(&runtime, stop);
	auto loadAssembly = [load](const std::string& path) {
		auto release = [](void* text) {
	#ifdef RD_PLATFORM_WINDOWS
			CoTaskMemFree(text);
	#else
			std::free(text);
	#endif
		};
		std::unique_ptr<void, decltype(release)> text(load(path.c_str()), release);
		REQUIRE(text != nullptr);
		return Rod::ScriptAssembly(path, static_cast<const char*>(text.get()));
	};
	const auto assembly = loadAssembly(ROD_TEST_GAME_ASSEMBLY_PATH);
	REQUIRE(assembly.HasClass("Game.Rotator"));
	REQUIRE_THROWS_AS(loadAssembly(std::string(ROD_TEST_GAME_ASSEMBLY_PATH) + ".missing"), std::runtime_error);
	REQUIRE(create(1, "Game.Rotator") == 1);
}
#endif
