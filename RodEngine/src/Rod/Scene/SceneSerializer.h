#pragma once

#include "Scene.h"

namespace Rod {

	class SceneSerializer
	{
	public:
		explicit SceneSerializer(Scene& scene);

		void SerializeText(const std::string& filepath);

		bool DeserializeText(const std::string& filepath);
	private:
		Scene& m_Scene;
	};

}
