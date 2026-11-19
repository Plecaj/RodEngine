#include "rdpch.h"

#include "Entity.h"

#include "Components.h"
#include "Rod/Core/UUID.h"
#include "Rod/Scene/Scene.h"

namespace Rod {

	auto Entity::GetUUID() const -> UUID
	{
		return GetComponent<IDComponent>().ID;
	}

	auto Entity::GetName() const -> const std::string&
	{
		return GetComponent<TagComponent>().Tag;
	}

}
