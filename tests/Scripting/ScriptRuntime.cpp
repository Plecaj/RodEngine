#include "rdpch.h"

#include "Rod/Scripting/ScriptRuntime.h"

#include "Rod/Scripting/ScriptAssembly.h"
#include "Rod/Scripting/ScriptInterop.h"
#include "Rod/Utils/FunctionPointerCast.h"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>

namespace {
	using LoadFn = void*(RD_CDECL*)(const char*);

	template <typename Function>
	auto RequireFunction(const Rod::ScriptRuntime& runtime, const std::string& hostType, const std::string& methodName)
	    -> Function
	{
		auto* const address = runtime.GetFunction(hostType, methodName);
		REQUIRE(address != nullptr);
		return Rod::FunctionPointerCast<Function>(address);
	}

	auto LoadAssembly(LoadFn load, const std::string& path) -> Rod::ScriptAssembly
	{
		const std::unique_ptr<void, decltype(&Rod::ScriptInterop::Free)> text(
		    load(path.c_str()),
		    Rod::ScriptInterop::Free);
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
	const auto stop = [shutdown](void*) {
		shutdown();
	};
	const std::unique_ptr<void, decltype(stop)> hostGuard(&runtime, stop);
	const auto assembly = LoadAssembly(load, ROD_TEST_GAME_ASSEMBLY_PATH);
	REQUIRE(assembly.HasClass("Game.Rotator"));
	REQUIRE_THROWS_AS(LoadAssembly(load, std::string(ROD_TEST_GAME_ASSEMBLY_PATH) + ".missing"), std::runtime_error);
	REQUIRE(create(1, "Game.Rotator") == 1);
}
