#include "rdpch.h"

#include "Rod/Scripting/ScriptAssembly.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

TEST_CASE("Script metadata loads classes and fields", "[scripting][loading]")
{
	Rod::ScriptAssembly assembly("Game.dll", R"({"classes":[{
"fullName":"Game.Player","namespace":"Game","name":"Player",
"fields":[{"name":"Speed","type":"Single","defaultValue":"3.5"}]}]})");
	const auto* scriptClass = assembly.GetClass("Game.Player");
	REQUIRE(scriptClass != nullptr);
	const auto& speed = scriptClass->GetFields().at("Speed");
	REQUIRE(speed.Field.Type == Rod::ScriptFieldType::Float);
	REQUIRE(speed.Value == "3.5");
}

TEST_CASE("Invalid script metadata is rejected", "[scripting][loading]")
{
	REQUIRE_THROWS_AS(Rod::ScriptAssembly("Game.dll", "{"), std::runtime_error);
	REQUIRE_THROWS_AS(Rod::ScriptAssembly("Game.dll", R"({"error":"Assembly missing"})"), std::runtime_error);
}
