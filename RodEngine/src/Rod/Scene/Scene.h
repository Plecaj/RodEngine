#pragma once

#include "entt.hpp"

#include "Rod/Core/Timestep.h"


namespace Rod {

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "");

		void OnUpdate(Timestep& ts);
	private:
		entt::registry m_Registry;

		friend class Entity;
	};

}