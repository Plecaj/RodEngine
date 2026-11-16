#include "rdpch.h"

#include "Entity.h"

#include "Components.h"
#include "Rod/Core/UUID.h"
#include "Rod/Scene/Scene.h"

namespace Rod {

	UUID Entity::GetUUID() const
	{
		return GetComponent<IDComponent>().ID;
	}

	const std::string& Entity::GetName() const
	{
		return GetComponent<TagComponent>().Tag;
	}

}
