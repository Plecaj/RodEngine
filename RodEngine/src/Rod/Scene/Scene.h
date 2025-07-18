#pragma once

#include "entt.hpp"

#include "Rod/Core/Timestep.h"


namespace Rod {

	class Scene
	{
	public:
		Scene();
		~Scene();

		// Temp data type, should use Rod::Entity
		entt::entity CreateEntity();

		// very temp
		entt::registry& Reg() { return m_Registry; }

		void OnUpdate(Timestep& ts);
	private:
		entt::registry m_Registry;
	};

}