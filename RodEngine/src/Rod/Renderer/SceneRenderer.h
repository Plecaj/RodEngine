#pragma once

#include "Rod/Renderer/EditorCamera.h"

#include <vector>

namespace Rod {

	class Scene;
	struct DirectionalLightComponent;

	class SceneRenderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void OnViewportResize(uint32_t width, uint32_t height);

		static void RenderRuntime(Scene& scene);
		static void RenderEditor(Scene& scene, EditorCamera& camera);

	private:
		struct RuntimeCameraData;
		static RuntimeCameraData FindRuntimeCamera(Scene& scene);
		static std::vector<DirectionalLightComponent> CollectLights(Scene& scene);
		static void SubmitMeshes(Scene& scene);
		static void SubmitSprites(Scene& scene);
	};

}
