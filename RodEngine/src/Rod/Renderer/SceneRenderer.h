#pragma once

#include "Rod/Renderer/EditorCamera.h"

namespace Rod {

	class Scene;

	class SceneRenderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void OnViewportResize(uint32_t width, uint32_t height);

		static void RenderRuntime(Scene& scene);
		static void RenderEditor(Scene& scene, EditorCamera& camera);
	};

}
