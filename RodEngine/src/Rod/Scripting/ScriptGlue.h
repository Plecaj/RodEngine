#pragma once

#include "Rod/Scene/Scene.h"

namespace Rod {

	struct NativeCall
	{
		const char* Name;
		void* Function;
	};

	class ScriptGlue
	{
	public:
		static void SetSceneContext(Scene* scene);
		static Scene* GetSceneContext() { return s_SceneContext; }
		static std::vector<NativeCall> GetNativeCalls();
	private:
		static Scene* s_SceneContext;
	};
}
