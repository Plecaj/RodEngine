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
		static void Init(const std::filesystem::path& shaderDirectory);
		static void Shutdown();
		static void OnViewportResize(uint32_t width, uint32_t height);

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
