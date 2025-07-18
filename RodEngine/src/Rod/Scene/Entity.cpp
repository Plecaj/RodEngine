#include "rdpch.h"
#include "Entity.h"

namespace Rod {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

}