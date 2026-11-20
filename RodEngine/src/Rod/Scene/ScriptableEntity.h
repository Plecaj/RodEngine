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
		auto operator=(const ScriptableEntity&) -> ScriptableEntity& = delete;
		ScriptableEntity(ScriptableEntity&&) = delete;
		auto operator=(ScriptableEntity&&) -> ScriptableEntity& = delete;

		template <typename T>
		[[nodiscard]] auto GetComponent() -> T&
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
