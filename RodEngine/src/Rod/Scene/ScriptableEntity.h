#pragma once

#include "Entity.h"
#include "Rod/Core/Timestep.h"

namespace Rod {

	class ScriptableEntity
	{
	public:
		ScriptableEntity() = default;
		virtual ~ScriptableEntity() = default;
		ScriptableEntity(const ScriptableEntity&) = delete;
		ScriptableEntity& operator=(const ScriptableEntity&) = delete;
		ScriptableEntity(ScriptableEntity&&) = delete;
		ScriptableEntity& operator=(ScriptableEntity&&) = delete;

		template <typename T>
		[[nodiscard]] T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

	protected:
		virtual void OnCreate() {};
		virtual void OnDestroy() {};
		virtual void OnUpdate(Timestep){};

	private:
		Entity m_Entity;
		friend class Scene;
	};

}
