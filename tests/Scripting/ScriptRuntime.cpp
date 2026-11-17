#include "rdpch.h"

#include "Rod/Scripting/ScriptRuntime.h"

#include "Rod/Scripting/ScriptAssembly.h"

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <memory>
#include <stdexcept>

#ifdef RD_PLATFORM_WINDOWS
	#include <objbase.h> // IWYU pragma: keep
#else
	#include <cstdlib> // IWYU pragma: keep
#endif

#ifdef ROD_TEST_CORE_ASSEMBLY_PATH
namespace {
	using LoadFn = void*(RD_CDECL*)(const char*);

	template <typename Function>
	Function
	RequireFunction(const Rod::ScriptRuntime& runtime, const std::string& hostType, const std::string& methodName)
	{
		const auto function = std::bit_cast<Function>(runtime.GetFunction(hostType, methodName));
		REQUIRE(function != nullptr);
		return function;
	}

	Rod::ScriptAssembly LoadAssembly(LoadFn load, const std::string& path)
	{
		auto release = [](void* text) {
	#ifdef RD_PLATFORM_WINDOWS
			CoTaskMemFree(text);
	#else
			std::free(text);
	#endif
		};
		const std::unique_ptr<void, decltype(release)> text(load(path.c_str()), release);
		REQUIRE(text != nullptr);
		return {path, static_cast<const char*>(text.get())};
	}
}

TEST_CASE("Failed managed assembly loads preserve the active assembly", "[scripting][loading]")
{
	const std::filesystem::path corePath = ROD_TEST_CORE_ASSEMBLY_PATH;
	const auto configPath = corePath.parent_path() / "Rod.ScriptCore.runtimeconfig.json";
	const std::string hostType = "Rod.Internal.ScriptHost, Rod.ScriptCore";
	{
		const Rod::ScriptRuntime initial(configPath, corePath);
		REQUIRE(initial.GetFunction(hostType, "Shutdown") != nullptr);
	}
	Rod::ScriptRuntime runtime(configPath, corePath);
	using CreateFn = int32_t(RD_CDECL*)(uint64_t, const char*);
	using ShutdownFn = void(RD_CDECL*)();
	const auto load = RequireFunction<LoadFn>(runtime, hostType, "LoadAssembly");
	const auto create = RequireFunction<CreateFn>(runtime, hostType, "CreateScript");
	const auto shutdown = RequireFunction<ShutdownFn>(runtime, hostType, "Shutdown");
	auto stop = [shutdown](void*) {
		shutdown();
	};
	const std::unique_ptr<void, decltype(stop)> hostGuard(&runtime, stop);
	const auto assembly = LoadAssembly(load, ROD_TEST_GAME_ASSEMBLY_PATH);
	REQUIRE(assembly.HasClass("Game.Rotator"));
	REQUIRE_THROWS_AS(LoadAssembly(load, std::string(ROD_TEST_GAME_ASSEMBLY_PATH) + ".missing"), std::runtime_error);
	REQUIRE(create(1, "Game.Rotator") == 1);
}
#endif
