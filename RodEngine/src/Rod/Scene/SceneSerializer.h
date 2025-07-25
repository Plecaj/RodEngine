#pragma once

#include "Scene.h"

namespace Rod {

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void SerializeText(const std::string& filepath);

		bool DeserializeText(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};

}