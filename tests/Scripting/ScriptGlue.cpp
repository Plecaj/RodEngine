#include "rdpch.h"

#include "Rod/Scripting/ScriptGlue.h"

#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"
#include "Rod/Utils/FunctionPointerCast.h"

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <stdexcept>
#include <string_view>

namespace {
	class NativeSceneContext
	{
	public:
		explicit NativeSceneContext(Rod::Scene& scene) { Rod::ScriptGlue::SetSceneContext(scene); }
		~NativeSceneContext() { Rod::ScriptGlue::SetSceneContext(m_PreviousScene); }
		NativeSceneContext(const NativeSceneContext&) = delete;
		auto operator=(const NativeSceneContext&) -> NativeSceneContext& = delete;
		NativeSceneContext(NativeSceneContext&&) = delete;
		auto operator=(NativeSceneContext&&) -> NativeSceneContext& = delete;

		template <typename Function>
		[[nodiscard]] auto GetFunction(std::string_view name) const -> Function
		{
			const auto call =
			    std::find_if(m_Calls.begin(), m_Calls.end(), [&](const auto& entry) { return name == entry.Name; });
			if (call == m_Calls.end())
				throw std::runtime_error("Missing native call: " + std::string(name));
			return Rod::FunctionPointerCast<Function>(call->Function);
		}

	private:
		std::optional<Rod::Scene&> m_PreviousScene = Rod::ScriptGlue::GetSceneContext();
		const std::vector<Rod::NativeCall> m_Calls = Rod::ScriptGlue::GetNativeCalls();
	};
}

TEST_CASE("Native script calls create and rename scene entities", "[scripting]")
{
	using CreateEntityFn = uint64_t(RD_CDECL*)(const char*);
	using SetNameFn = void(RD_CDECL*)(uint64_t, const char*);
	Rod::Scene scene;
	const NativeSceneContext context(scene);
	const auto create = context.GetFunction<CreateEntityFn>("Entity.Create");
	const auto setName = context.GetFunction<SetNameFn>("Entity.SetName");
	const auto id = create("Scripted");
	REQUIRE(scene.FindEntityByUUID(Rod::UUID{id}));
	setName(id, "Renamed");
	REQUIRE(scene.FindEntityByName("Renamed").GetUUID() == Rod::UUID{id});
}

TEST_CASE("Native script calls add cameras and update primary flags", "[scripting]")
{
	using CreateEntityFn = uint64_t(RD_CDECL*)(const char*);
	using AddComponentFn = void(RD_CDECL*)(uint64_t, int32_t);
	using HasComponentFn = bool(RD_CDECL*)(uint64_t, int32_t);
	using GetPrimaryFn = bool(RD_CDECL*)(uint64_t);
	using SetPrimaryFn = void(RD_CDECL*)(uint64_t, bool);
	Rod::Scene scene;
	const NativeSceneContext context(scene);
	const auto create = context.GetFunction<CreateEntityFn>("Entity.Create");
	const auto addComponent = context.GetFunction<AddComponentFn>("Entity.AddComponent");
	const auto hasComponent = context.GetFunction<HasComponentFn>("Entity.HasComponent");
	const auto getPrimary = context.GetFunction<GetPrimaryFn>("Camera.GetPrimary");
	const auto setPrimary = context.GetFunction<SetPrimaryFn>("Camera.SetPrimary");
	const auto id = create("Scripted");
	REQUIRE(!hasComponent(id, 3));
	addComponent(id, 3);
	REQUIRE(hasComponent(id, 3));
	REQUIRE(getPrimary(id));
	setPrimary(id, false);
	REQUIRE(!getPrimary(id));
}
