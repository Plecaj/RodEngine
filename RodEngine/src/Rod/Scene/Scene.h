#pragma once

#include "entt.hpp"

namespace Rod {

	class Scene
	{
	public:
		Scene();
		~Scene();
	private:
		entt::registry m_Registry;
	};

}