#include "rdpch.h"

#include "Entity.h"

#include "Components.h"
#include "Rod/Core/UUID.h"
#include "Rod/Scene/Scene.h"

namespace Rod {

	Entity::Entity(entt::entity handle, Scene* scene) noexcept : m_EntityHandle(handle), m_Scene(scene)
	{}

	UUID Entity::GetUUID() const
	{
		return GetComponent<IDComponent>().ID;
	}

	const std::string& Entity::GetName() const
	{
		return GetComponent<TagComponent>().Tag;
	}

}
