#include "rdpch.h"

#include "Rod/Scripting/ScriptGlue.h"

#include "Rod/Scene/Entity.h"
#include "Rod/Scene/Scene.h"

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <stdexcept>
#include <string_view>

namespace {
	class NativeSceneContext
	{
	public:
		explicit NativeSceneContext(Rod::Scene& scene) { Rod::ScriptGlue::SetSceneContext(&scene); }
		~NativeSceneContext() { Rod::ScriptGlue::SetSceneContext(m_PreviousScene); }
		NativeSceneContext(const NativeSceneContext&) = delete;
		NativeSceneContext& operator=(const NativeSceneContext&) = delete;
		NativeSceneContext(NativeSceneContext&&) = delete;
		NativeSceneContext& operator=(NativeSceneContext&&) = delete;

		template <typename Function>
		[[nodiscard]] Function GetFunction(std::string_view name) const
		{
			const auto call =
			    std::find_if(m_Calls.begin(), m_Calls.end(), [&](const auto& entry) { return name == entry.Name; });
			if (call == m_Calls.end())
				throw std::runtime_error("Missing native call: " + std::string(name));
			return std::bit_cast<Function>(call->Function);
		}

	private:
		Rod::Scene* m_PreviousScene = Rod::ScriptGlue::GetSceneContext();
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
	constexpr int32_t cameraComponentType = 3;
	REQUIRE(!hasComponent(id, cameraComponentType));
	addComponent(id, cameraComponentType);
	REQUIRE(hasComponent(id, cameraComponentType));
	REQUIRE(getPrimary(id));
	setPrimary(id, false);
	REQUIRE(!getPrimary(id));
}
