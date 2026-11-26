#pragma once

#include "Renderer2D.h"
#include "Renderer3D.h"

#include <glm/mat4x4.hpp>

namespace Rod {

	class Scene;

	class SceneRenderer
	{
	public:
		SceneRenderer(const RenderDevice& device, const ShaderLoader& shaders);

		void Render(const Scene& scene, const glm::mat4& viewProjection);
		void RenderRuntime(Scene& scene);

	private:
		Renderer3D m_Meshes;
		Renderer2D m_Sprites;
	};

}
