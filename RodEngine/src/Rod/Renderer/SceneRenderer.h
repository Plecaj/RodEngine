#pragma once

#include "Rod/Renderer/EditorCamera.h"

#include <filesystem>
#include <vector>

namespace Rod {

	class Scene;
	struct DirectionalLightComponent;

	class SceneRenderer
	{
	public:
		explicit SceneRenderer(const std::filesystem::path& shaderDirectory);
		~SceneRenderer() noexcept;
		SceneRenderer(const SceneRenderer&) = delete;
		auto operator=(const SceneRenderer&) -> SceneRenderer& = delete;
		SceneRenderer(SceneRenderer&&) = delete;
		auto operator=(SceneRenderer&&) -> SceneRenderer& = delete;

		static void RenderRuntime(Scene& scene);
		static void RenderEditor(Scene& scene, EditorCamera& camera);

	private:
		struct RuntimeCameraData;
		[[nodiscard]] static auto FindRuntimeCamera(Scene& scene) -> RuntimeCameraData;
		static auto CollectLights(Scene& scene) -> std::vector<DirectionalLightComponent>;
		static void SubmitMeshes(Scene& scene);
		static void SubmitSprites(Scene& scene);
	};

}
