#pragma once

#include "Entity.h"	
#include "Rod/Core/Timestep.h"

namespace Rod {

	class ScriptableEntity 
	{
	public:
		virtual ~ScriptableEntity() = default;

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}
	protected:
		virtual void OnCreate() {};
		virtual void OnDestroy() {};
		virtual void OnUpdate(Timestep ts) {};
	private:
		Entity m_Entity;
		friend class Scene;
	};

}